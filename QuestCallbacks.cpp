#include "pch.h"
#include "QuestCallbacks.h"

void QuestCallbacks::on_quest_general_info(GW::HookStatus* status, void* packet) { quests_changed = true; }

void QuestCallbacks::on_quest_description(GW::HookStatus* status, void* packet) { quests_changed = true; }

void QuestCallbacks::on_quest_add(GW::HookStatus* status, void* packet) { quests_changed = true; }

void QuestCallbacks::on_quest_update_marker(GW::HookStatus* status, void* packet) { quests_changed = true; }

void QuestCallbacks::on_quest_remove(GW::HookStatus* status, void* packet) { quests_changed = true; }

void QuestCallbacks::on_quest_add_marker(GW::HookStatus* status, void* packet) { quests_changed = true; }

void QuestCallbacks::on_quest_update_name(GW::HookStatus* status, void* packet) { quests_changed = true; }
