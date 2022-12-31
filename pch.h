// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

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

// Other stuff

#include "GWIPC.h";
#include "string_utils.h"

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

#include "Packets/StoC.h"

#include "GameEntities/Camera.h"
#include "GameEntities/Agent.h"
#include "GameEntities/Party.h"

#include "Managers/GameThreadMgr.h"
#include "Managers/MemoryMgr.h"
#include "Managers/ChatMgr.h"
#include "Managers/StoCMgr.h"
#include "Managers/MapMgr.h"
#include "Managers/CameraMgr.h"
#include "Managers/UIMgr.h"
#include "Managers/AgentMgr.h"
#include "Managers/PartyMgr.h"

// My header that depend on GWCA
#include "ChatWriter.h"

#endif //PCH_H
