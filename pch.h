// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"
#include <boost/interprocess/managed_shared_memory.hpp>
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
#include "SharedMemory.h"

// Definitions
using PartyId = int;
using InstanceId = uint32_t;

// GWCA includes
#include "Constants/Maps.h"
#include "Constants/Constants.h"

#include "Context/CharContext.h"
#include "Context/PreGameContext.h"
#include "Context/MapContext.h"
#include "Context/PartyContext.h"
#include "Context/WorldContext.h"
#include "Context/AgentContext.h"

#include "Packets/StoC.h"

#include "GameEntities/Camera.h"

#include "Managers/GameThreadMgr.h"
#include "Managers/MemoryMgr.h"
#include "Managers/ChatMgr.h"
#include "Managers/StoCMgr.h"
#include "Managers/MapMgr.h"
#include "Managers/CameraMgr.h"

// My header that depend on GWCA
#include "ChatWriter.h"

#endif //PCH_H
