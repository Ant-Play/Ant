#pragma once

#include "Ant/Core/PlatformDetection.h"

#ifdef ANT_PLATFORM_WINDOWS
#include <Windows.h>
#ifdef NOMINMAX
// See github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
#define NOMINMAX
#endif // NOMINMAX
#endif // ANT_PLATFORM_WINDOWS

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <fstream>

#include "Ant/Core/Base.h"
#include "Ant/Core/Log.h"
#include "Ant/Debug/Instrumentor.h"
#include "Ant/Core/Events/Event.h"


// Math
#include "Ant/Core/Math/Mat4.h"