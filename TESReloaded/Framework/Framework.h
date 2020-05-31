#pragma once

#define DETOURS_INTERNAL
#define assert(a) static_assert(a, "Assertion failed")

#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <d3d9.h>
#include <d3dx9math.h>
#include <dinput.h>
#include <dsound.h>
#include "Detours\detours.h"
#include "NVAPI\nvapi.h"
#include "Bink\Binker.h"
#include "Types.h"
#include "SafeWrite.h"
#include "Game.h"
#include "Logger.h"
#include "Plugin.h"
#include "Managers.h"