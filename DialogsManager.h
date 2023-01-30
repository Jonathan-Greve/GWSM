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

bool ParseUInt(const char* str, unsigned int* val, int base = 0);
bool ParseUInt(const wchar_t* str, unsigned int* val, int base = 0);

class DialogsManager
{
    DialogsManager() = default;
    ~DialogsManager() = default;

public:
    static DialogsManager& Instance()
    {
        static DialogsManager instance;
        return instance;
    }

    static bool Initialize();
    static void Terminate();
    static void Update(float);

    static EncString* GetDialogBody();
    static const uint32_t GetDialogAgentId();
    static const uint32_t GetLastDialogAgentId();
    static const std::vector<GW::UI::DialogButtonInfo*>& GetDialogButtons();
    static const std::vector<EncString*>& GetDialogButtonMessages();

    static void SendDialog(uint32_t dialog_id);
    static void SendDialogs(std::initializer_list<uint32_t> dialog_ids);
    // Find and take the first available quest from the current dialog. Returns quest_id requested.
    static const uint32_t AcceptFirstAvailableQuest();

private:
    // private method to queue multiple dialogs with the same timestamp
    static void SendDialog(uint32_t dialog_id, clock_t time);

    enum class QuestDialogType
    {
        TAKE = 0x800001,
        ENQUIRE = 0x800003,
        ENQUIRE_NEXT = 0x800004,
        ENQUIRE_REWARD = 0x800006,
        REWARD = 0x800007
    };
    static bool IsQuest(const uint32_t dialog_id) { return (dialog_id & 0x800000) != 0; }
    static uint32_t GetQuestID(const uint32_t dialog_id) { return (dialog_id ^ 0x800000) >> 8; }
    static uint32_t SetQuestDialogType(const uint32_t quest_id, QuestDialogType type)
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
    static void OnDialogSent(uint32_t dialog_id);
    static void OnPostUIMessage(GW::HookStatus* status, GW::UI::UIMessage message_id, void* wparam, void*);
    static void OnPreUIMessage(GW::HookStatus* status, GW::UI::UIMessage message_id, void* wparam, void*);
};
