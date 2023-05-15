include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "Ant"
    architecture "x64"
    targetdir "build"
    
    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    flags
    {
        "MultiProcessorCompile"
    }
    
    startproject "AntPlay"
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
    include "Ant/vendor/GLFW"
    include "Ant/vendor/Box2D"
    include "Ant/vendor/Glad"
    include "Ant/vendor/imgui"
    include "Ant/vendor/yaml-cpp"
group ""

group "Core"
    include "Ant"
    include "Ant-ScriptCore"
group ""

group "Examples"
    include "ExampleApp"
group ""

group "Tools"
	include "AntPlay"
group ""

group "Misc"
	include "Sandbox"
group ""