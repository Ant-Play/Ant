include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "Ant"
    architecture "x64"
    startproject "AntPlay"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    solution_items
    {
        ".editorconfig"
    }

    flags
    {
        "MultiProcessorCompile"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
    --include "vendor/premake"
    include "Ant/vendor/GLFW"
    include "Ant/vendor/Box2D"
    include "Ant/vendor/Glad"
    include "Ant/vendor/imgui"
    include "Ant/vendor/yaml-cpp"
group ""

group "Core"
    include "Ant"
group ""

group "Tools"
	include "AntPlay"
group ""

group "Misc"
	include "Sandbox"
group ""