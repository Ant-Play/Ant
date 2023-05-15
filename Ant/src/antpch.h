#pragma once

#ifdef ANT_PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <memory>
#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include <functional>
#include <algorithm>

#include <fstream>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <Ant/Core/Base.h>
#include <Ant/Core/Log.h>
#include <Ant/Core/Events/Event.h>

// Math
#include <Ant/Core/Math/Mat4.h>