#pragma once
#include <ctime>
/*
This makes using timers easier
clock type is clock_t
create a timer with TIMER_INIT()
find the difference in milliseconds with TIMER_DIFF(clock_t timer)
*/
#define TIMER_INIT() (clock())
#define TIMER_DIFF(t) (clock() - t)

namespace GW
{
bool InitDialogFuncs();
}

class DialogsManager
{
public:
    bool Initialize();
    void Terminate();
    void Update(float);

    const wchar_t* GetDialogBody() const;
    const uint32_t GetDialogAgentId() const;
    const std::vector<GW::UI::DialogButtonInfo*>& GetDialogButtons() const;
    const std::vector<EncString*>& GetDialogButtonMessages() const;

    void SendDialog(uint32_t dialog_id);
    void SendDialogs(std::initializer_list<uint32_t> dialog_ids);
    // Find and take the first available quest from the current dialog. Returns quest_id requested.
    const uint32_t AcceptFirstAvailableQuest();

    EncString dialog_body;
    uint32_t last_agent_id = 0;

private:
    GW::UI::DialogBodyInfo dialog_info;

    GW::HookEntry dialog_hook;

    std::map<uint32_t, clock_t> queued_dialogs_to_send;

    // private method to queue multiple dialogs with the same timestamp
    void SendDialog(uint32_t dialog_id, clock_t time);

    enum class QuestDialogType
    {
        TAKE = 0x800001,
        ENQUIRE = 0x800003,
        ENQUIRE_NEXT = 0x800004,
        ENQUIRE_REWARD = 0x800006,
        REWARD = 0x800007
    };
    bool IsQuest(const uint32_t dialog_id) { return (dialog_id & 0x800000) != 0; }
    uint32_t GetQuestID(const uint32_t dialog_id) { return (dialog_id ^ 0x800000) >> 8; }
    uint32_t SetQuestDialogType(const uint32_t quest_id, QuestDialogType type)
    {
        return (quest_id << 8) | (uint32_t)type;
    }
    static QuestDialogType GetQuestDialogType(const uint32_t dialog_id)
    {
        return static_cast<QuestDialogType>(dialog_id & 0xf0000f);
    }
    static bool IsUWTele(const uint32_t dialog_id)
    {
        switch (dialog_id)
        {
        case GW::Constants::DialogID::UwTeleLab:
        case GW::Constants::DialogID::UwTeleVale:
        case GW::Constants::DialogID::UwTelePits:
        case GW::Constants::DialogID::UwTelePools:
        case GW::Constants::DialogID::UwTelePlanes:
        case GW::Constants::DialogID::UwTeleWastes:
        case GW::Constants::DialogID::UwTeleMnt:
            return true;
        default:
            return false;
        }
    }
    void OnDialogSent(uint32_t dialog_id);
    void OnPostUIMessage(GW::HookStatus* status, GW::UI::UIMessage message_id, void* wparam, void*);
    static void OnDialogButtonAdded(GW::UI::DialogButtonInfo* wparam);
    static void OnDialogBodyDecoded(void*, wchar_t* decoded);
    void ResetDialog();
    void OnNPCDialogUICallback(GW::UI::InteractionMessage* message, void* wparam, void* lparam);
    void OnDialogClosedByServer();
    bool IsDialogButtonAvailable(uint32_t dialog_id);
    void OnPreUIMessage(GW::HookStatus* status, GW::UI::UIMessage message_id, void* wparam, void*);

    static bool ParseUInt(const char* str, unsigned int* val, int base = 0);
    static bool ParseUInt(const wchar_t* str, unsigned int* val, int base = 0);
};
