#pragma once
class QuestCallbacks
{
public:
    QuestCallbacks() = default;
    bool init()
    {
        bool success = true;
        success = success &&
          GW::StoC::RegisterPacketCallback(&Quests_HookEntry, GAME_SMSG_QUEST_GENERAL_INFO,
                                           [this](GW::HookStatus* status, void* packet)
                                           { return on_quest_general_info(status, packet); });
        success = success &&
          GW::StoC::RegisterPacketCallback(&Quests_HookEntry, GAME_SMSG_QUEST_DESCRIPTION,
                                           [this](GW::HookStatus* status, void* packet)
                                           { return on_quest_description(status, packet); });
        success = success &&
          GW::StoC::RegisterPacketCallback(&Quests_HookEntry, GAME_SMSG_QUEST_ADD,
                                           [this](GW::HookStatus* status, void* packet)
                                           { return on_quest_add(status, packet); });
        success = success &&
          GW::StoC::RegisterPacketCallback(&Quests_HookEntry, GAME_SMSG_QUEST_UPDATE_MARKER,
                                           [this](GW::HookStatus* status, void* packet)
                                           { return on_quest_update_marker(status, packet); });
        success = success &&
          GW::StoC::RegisterPacketCallback(&Quests_HookEntry, GAME_SMSG_QUEST_REMOVE,
                                           [this](GW::HookStatus* status, void* packet)
                                           { return on_quest_remove(status, packet); });
        success = success &&
          GW::StoC::RegisterPacketCallback(&Quests_HookEntry, GAME_SMSG_QUEST_ADD_MARKER,
                                           [this](GW::HookStatus* status, void* packet)
                                           { return on_quest_add_marker(status, packet); });
        success = success &&
          GW::StoC::RegisterPacketCallback(&Quests_HookEntry, GAME_SMSG_QUEST_UPDATE_NAME,
                                           [this](GW::HookStatus* status, void* packet)
                                           { return on_quest_update_name(status, packet); });

        return success;
    }

    std::atomic<bool> quests_changed = false;

private:
    GW::HookEntry Quests_HookEntry;

    void on_quest_general_info(GW::HookStatus* status, void* packet);
    void on_quest_description(GW::HookStatus* status, void* packet);
    void on_quest_add(GW::HookStatus* status, void* packet);
    void on_quest_update_marker(GW::HookStatus* status, void* packet);
    void on_quest_remove(GW::HookStatus* status, void* packet);
    void on_quest_add_marker(GW::HookStatus* status, void* packet);
    void on_quest_update_name(GW::HookStatus* status, void* packet);
};
