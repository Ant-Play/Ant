#pragma once

#ifdef ANT_PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <Ant/Core/Application.h>
#include <Ant/Core/Assert.h>
#include <Ant/Core/Base.h>
#include <Ant/Core/Events/Event.h>
#include <Ant/Core/Log.h>
#include <Ant/Core/Math/Mat4.h>
#include <Ant/Core/Memory.h>
#include <Ant/Core/Delegate.h>