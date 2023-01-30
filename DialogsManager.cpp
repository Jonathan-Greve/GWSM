#include "pch.h"
#include "DialogsManager.h"

GW::UI::UIInteractionCallback NPCDialogUICallback_Func = nullptr;
GW::UI::UIInteractionCallback NPCDialogUICallback_Ret = nullptr;

std::vector<GW::UI::DialogButtonInfo*> dialog_buttons;
std::vector<EncString*> dialog_button_messages;

void DialogsManager::OnDialogButtonAdded(GW::UI::DialogButtonInfo* wparam)
{
    const auto button_info = new GW::UI::DialogButtonInfo();
    memcpy(button_info, wparam, sizeof(*button_info));

    const auto button_message = new EncString(button_info->message);
    button_info->message = const_cast<wchar_t*>(button_message->encoded().data());

    dialog_button_messages.push_back(button_message);
    dialog_buttons.push_back(button_info);

    // Start decoding (async, non-blocking)
    button_message->string();
}
// Parse any buttons held within the dialog body
void DialogsManager::OnDialogBodyDecoded(void*, wchar_t* decoded)
{
    const std::wregex button_regex(L"<a=([0-9]+)>([^<]+)<");
    std::wsmatch m;
    std::wstring subject(decoded);
    std::wstring msg;
    GW::UI::DialogButtonInfo embedded_button{};
    embedded_button.dialog_id = 0;
    embedded_button.skill_id = 0xFFFFFFF;
    while (std::regex_search(subject, m, button_regex))
    {
        if (! ParseUInt(m[1].str().c_str(), &embedded_button.dialog_id))
        {
            ChatWriter::WriteIngameDebugChat(
              std::format(L"Failed to parse dialog id for {}, {}", m[1].str(), m[2].str().c_str()));
            return;
        }
        // Technically theres no encoded string for this button, wrap it to avoid issues later.
        msg = L"\x108\x107";
        msg += m[2].str();
        msg += L"\x1";

        embedded_button.message = msg.data();
        OnDialogButtonAdded(&embedded_button);
        subject = m.suffix().str();
    }
}
// Wipe dialog ready for new one
void DialogsManager::ResetDialog()
{
    for (const auto d : dialog_buttons)
    {
        delete d;
    }
    dialog_buttons.clear();
    for (const auto d : dialog_button_messages)
    {
        delete d;
    }
    dialog_button_messages.clear();
}
void DialogsManager::OnNPCDialogUICallback(GW::UI::InteractionMessage* message, void* wparam, void* lparam)
{
    GW::HookBase::EnterHook();
    if (message->message_id == 0xb)
    {
        ResetDialog();
        if (dialog_info.agent_id)
            last_agent_id = dialog_info.agent_id;
        dialog_info.agent_id = 0;
    }
    NPCDialogUICallback_Ret(message, wparam, lparam);
    GW::HookBase::LeaveHook();
}
void DialogsManager::OnDialogClosedByServer()
{
    if (queued_dialogs_to_send.empty())
        return;
    const GW::Agent* npc = GW::Agents::GetAgentByID(last_agent_id);
    const GW::Agent* me = GW::Agents::GetPlayer();
    if (npc && me && GW::GetDistance(npc->pos, me->pos) < GW::Constants::Range::Area)
        GW::Agents::GoNPC(npc);
}
bool DialogsManager::IsDialogButtonAvailable(uint32_t dialog_id)
{
    return std::ranges::any_of(
      dialog_buttons, [dialog_id](const GW::UI::DialogButtonInfo* d) { return d->dialog_id == dialog_id; });
}

void DialogsManager::OnPreUIMessage(GW::HookStatus* status, GW::UI::UIMessage message_id, void* wparam, void*)
{
    switch (message_id)
    {
    case GW::UI::UIMessage::kDialogBody:
    {
        const auto new_dialog_info = static_cast<GW::UI::DialogBodyInfo*>(wparam);
        if (! new_dialog_info->message_enc)
        {
            OnDialogClosedByServer();
        }
    }
    break;
    case GW::UI::UIMessage::kSendDialog:
    {
        const auto dialog_id = reinterpret_cast<uint32_t>(wparam);
        if ((dialog_id & 0xff000000) != 0)
            break; // Don't handle merchant interaction dialogs.
        if (! IsDialogButtonAvailable(dialog_id))
        {
            status->blocked = true;
        }
        else
        {
            ResetDialog();
        }
    }
    break;
    }
}

bool DialogsManager::ParseUInt(const char* str, unsigned int* val, int base)
{
    char* end;
    if (! str)
        return false;
    *val = strtoul(str, &end, base);
    if (str == end || errno == ERANGE)
        return false;
    else
        return true;
}

bool DialogsManager::ParseUInt(const wchar_t* str, unsigned int* val, int base)
{
    wchar_t* end;
    if (! str)
        return false;
    *val = wcstoul(str, &end, base);
    if (str == end || errno == ERANGE)
        return false;
    else
        return true;
}

void DialogsManager::OnPostUIMessage(GW::HookStatus* status, GW::UI::UIMessage message_id, void* wparam,
                                     void*)
{
    if (status->blocked)
    {
        // Blocked elsewhere.
        return;
    }
    switch (message_id)
    {
    case GW::UI::UIMessage::kDialogBody:
    {
        ResetDialog();
        const auto new_dialog_info = static_cast<GW::UI::DialogBodyInfo*>(wparam);
        if (! new_dialog_info->message_enc)
        {
            return; // Dialog closed.
        }
        memcpy(&dialog_info, wparam, sizeof(dialog_info));
        dialog_body.reset(dialog_info.message_enc);

        // Decode
        dialog_body.string();

        GW::UI::AsyncDecodeStr(dialog_info.message_enc, DialogsManager::OnDialogBodyDecoded);
    }
    break;
    case GW::UI::UIMessage::kDialogButton:
    {
        OnDialogButtonAdded(static_cast<GW::UI::DialogButtonInfo*>(wparam));
    }
    break;
    case GW::UI::UIMessage::kSendDialog:
    {
        OnDialogSent(reinterpret_cast<uint32_t>(wparam));
    }
    break;
    }
}

void DialogsManager::OnDialogSent(const uint32_t dialog_id)
{
    const auto queued_at =
      queued_dialogs_to_send.contains(dialog_id) ? queued_dialogs_to_send.at(dialog_id) : 0;
    queued_dialogs_to_send.erase(dialog_id);
    if (IsQuest(dialog_id))
    {
        const auto quest_id = GetQuestID(dialog_id);
        switch (GetQuestDialogType(dialog_id))
        {
        case QuestDialogType::TAKE:
        case QuestDialogType::REWARD:
            break;
        default:
            return;
        }
        for (auto it = queued_dialogs_to_send.begin(); it != queued_dialogs_to_send.end();)
        {
            const auto other_dialog_id = it->first;
            // make sure we don't delete dialogs for the same quest queued up earlier or later, e.g. separate reward dialog!
            if (GetQuestID(other_dialog_id) == quest_id && queued_at == it->second)
            {
                it = queued_dialogs_to_send.erase(it);
            }
            else
            {
                it++;
            }
        }
    }
    if (IsUWTele(dialog_id))
    {
        queued_dialogs_to_send.erase(GW::Constants::DialogID::UwTeleEnquire);
        queued_dialogs_to_send.erase(dialog_id - 0x7);
    }
}

bool DialogsManager::Initialize()
{
    constexpr GW::UI::UIMessage dialog_ui_messages[] = {
      GW::UI::UIMessage::kSendDialog, GW::UI::UIMessage::kDialogBody, GW::UI::UIMessage::kDialogButton};
    for (const auto message_id : dialog_ui_messages)
    {
        GW::UI::RegisterUIMessageCallback(
          &dialog_hook, message_id,
          [this](GW::HookStatus* status, GW::UI::UIMessage message_id, void* wparam, void* lparam)
          { DialogsManager::OnPreUIMessage(status, message_id, wparam, nullptr); },
          -0x1);
        GW::UI::RegisterUIMessageCallback(
          &dialog_hook, message_id,
          [this](GW::HookStatus* status, GW::UI::UIMessage message_id, void* wparam, void* lparam)
          { DialogsManager::OnPostUIMessage(status, message_id, wparam, nullptr); },
          0x500);
    }
    if (NPCDialogUICallback_Func)
    {
        auto on_npc_dialog_lambda = [this](GW::UI::InteractionMessage* message, void* wparam, void* lparam)
        { DialogsManager::OnNPCDialogUICallback(message, wparam, lparam); };

        GW::HookBase::CreateHook(NPCDialogUICallback_Func, &on_npc_dialog_lambda,
                                 reinterpret_cast<void**>(&NPCDialogUICallback_Ret));
        GW::HookBase::EnableHooks(NPCDialogUICallback_Func);

        return true;
    }

    return false;
}

void DialogsManager::Terminate()
{
    GW::UI::RemoveUIMessageCallback(&dialog_hook);
    GW::HookBase::RemoveHook(NPCDialogUICallback_Func);
}

void DialogsManager::SendDialog(const uint32_t dialog_id, clock_t time)
{
    time = time ? time : TIMER_INIT();
    queued_dialogs_to_send[dialog_id] = time;

    if (IsQuest(dialog_id))
    {
        const uint32_t quest_id = GetQuestID(dialog_id);
        switch (GetQuestDialogType(dialog_id))
        {
        case QuestDialogType::TAKE: // Dialog is for taking a quest
            queued_dialogs_to_send[SetQuestDialogType(quest_id, QuestDialogType::ENQUIRE_NEXT)] = time;
            queued_dialogs_to_send[SetQuestDialogType(quest_id, QuestDialogType::ENQUIRE)] = time;
            break;
        case QuestDialogType::REWARD: // Dialog is for accepting a quest reward
            queued_dialogs_to_send[SetQuestDialogType(quest_id, QuestDialogType::ENQUIRE_NEXT)] = time;
            queued_dialogs_to_send[SetQuestDialogType(quest_id, QuestDialogType::ENQUIRE_REWARD)] = time;
            break;
        default:
            return;
        }
    }

    if ((dialog_id & 0xf84) == dialog_id && (dialog_id & 0xfff) >> 8 != 0)
    {
        // Dialog is for changing profession; queue up the enquire dialog option aswell
        const uint32_t profession_id = (dialog_id & 0xfff) >> 8;
        const uint32_t enquire_dialog_id = (profession_id << 8) | 0x85;
        queued_dialogs_to_send[enquire_dialog_id] = time;
        return;
    }

    if (IsUWTele(dialog_id))
    {
        // Reaper teleport dialog; queue up prerequisites.
        queued_dialogs_to_send[GW::Constants::DialogID::UwTeleEnquire] = time;
        queued_dialogs_to_send[dialog_id - 0x7] = time;
    }
}

void DialogsManager::SendDialog(uint32_t dialog_id) { SendDialog(dialog_id, TIMER_INIT()); }

void DialogsManager::SendDialogs(std::initializer_list<uint32_t> dialog_ids)
{
    const auto timestamp = TIMER_INIT();
    for (const auto dialog_id : dialog_ids)
    {
        SendDialog(dialog_id, timestamp);
    }
}

void DialogsManager::Update(float)
{
    for (auto it = queued_dialogs_to_send.begin(); it != queued_dialogs_to_send.end(); it++)
    {
        if (TIMER_DIFF(it->second) > 3000)
        {
            // NB: Show timeout error message?
            queued_dialogs_to_send.erase(it);
            break;
        }
        if (! GetDialogAgentId())
            continue;
        if (it->first == 0)
        {
            // If dialog queued is id 0, this means the player wants to take (or accept reward for) the first available quest.
            AcceptFirstAvailableQuest();
            queued_dialogs_to_send.erase(it);
            break;
        }
        if (IsDialogButtonAvailable(it->first))
        {
            GW::Agents::SendDialog(it->first);
            break;
        }
    }
}

const wchar_t* DialogsManager::GetDialogBody() const { return dialog_body.encoded().c_str(); }

const std::vector<EncString*>& DialogsManager::GetDialogButtonMessages() const
{
    return dialog_button_messages;
}

const uint32_t DialogsManager::AcceptFirstAvailableQuest()
{
    if (dialog_buttons.empty())
        return 0;
    std::vector<uint32_t> available_quests;
    for (const auto dialog_button : dialog_buttons)
    {
        const uint32_t dialog_id = dialog_button->dialog_id;
        if (! IsQuest(dialog_id))
            continue;
        // Quest related dialog
        uint32_t quest_id = GetQuestID(dialog_id);
        switch (GetQuestDialogType(dialog_id))
        {
        case QuestDialogType::TAKE:
        case QuestDialogType::ENQUIRE:
        case QuestDialogType::ENQUIRE_NEXT:
        case QuestDialogType::ENQUIRE_REWARD:
        case QuestDialogType::REWARD:
            available_quests.push_back(quest_id);
            break;
        default:
            break;
        }
    }
    for (const auto quest_id : available_quests)
    {
        if (quest_id == static_cast<uint32_t>(GW::Constants::QuestID::UW_UWG) && available_quests.size() > 1)
        {
            // skip unless it's the only available dialog - certain quests almost always want to be taken last
            continue;
        }
        SendDialogs({SetQuestDialogType(quest_id, QuestDialogType::TAKE),
                     SetQuestDialogType(quest_id, QuestDialogType::REWARD)});
        return quest_id;
    }
    return 0;
}

const uint32_t DialogsManager::GetDialogAgentId() const { return dialog_info.agent_id; }

const std::vector<GW::UI::DialogButtonInfo*>& DialogsManager::GetDialogButtons() const
{
    return dialog_buttons;
}

namespace GW
{
bool GW::InitDialogFuncs()
{
    NPCDialogUICallback_Func = reinterpret_cast<GW::UI::UIInteractionCallback>(GW::Scanner::FindAssertion(
      R"(p:\code\gw\ui\game\gmnpc.cpp)", "interactMsg.codedText && interactMsg.codedText[0]", -0xfb));

    if (NPCDialogUICallback_Func)
    {
        return true;
    }

    return false;
}
}
