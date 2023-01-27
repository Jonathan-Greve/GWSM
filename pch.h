// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#define NOMINMAX

// add headers that you want to pre-compile here
#include "framework.h"
#include <string>
#include <format>
#include <map>
#include <set>
#include <unordered_set>
#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <functional>
#include <limits>

// Other stuff

#include "GWIPC.h"
#include "string_utils.h"
#include "AgentUtils.h"
#include "UpdateStatus.h"

// GWCA includes
#include "Constants/Maps.h"
#include "Constants/Constants.h"

#include "Context/CharContext.h"
#include "Context/PreGameContext.h"
#include "Context/MapContext.h"
#include "Context/PartyContext.h"
#include "Context/WorldContext.h"
#include "Context/AgentContext.h"
#include "Context/PartyContext.h"
#include "Context/GameContext.h"
#include "Context/ItemContext.h"
#include "Context/TradeContext.h"

#include "Packets/StoC.h"

#include "GameEntities/Camera.h"
#include "GameEntities/Agent.h"
#include "GameEntities/Party.h"
#include "GameEntities/Skill.h"
#include "GameEntities/Hero.h"
#include "GameEntities/Quest.h"
#include "GameEntities/Item.h"
#include "GameEntities/Guild.h"
#include "GameEntities/Pathing.h"

#include "Managers/GameThreadMgr.h"
#include "Managers/MemoryMgr.h"
#include "Managers/ChatMgr.h"
#include "Managers/StoCMgr.h"
#include "Managers/MapMgr.h"
#include "Managers/CameraMgr.h"
#include "Managers/UIMgr.h"
#include "Managers/AgentMgr.h"
#include "Managers/PartyMgr.h"
#include "Managers/EffectMgr.h"
#include "Managers/SkillbarMgr.h"
#include "Managers/ItemMgr.h"
#include "Managers/PlayerMgr.h"
#include "Managers/CtoSMgr.h"

#include "Utilities/Hook.h"
#include "Utilities/Hooker.h"
#include "Utilities/Scanner.h"

#include "bag_and_item_utils.h"
// My header that depend on GWCA
#include "ChatWriter.h"
#include "ItemCallbacks.h"
#include "QuestCallbacks.h"
#include "GWCA_UIMgr_Extensions.h"

#endif //PCH_H
