FileVersion = 1.2

AntRootDirectory = os.getenv("ANT_DIR")
include (path.join(AntRootDirectory, "AntPlay", "Resources", "LUA", "Ant.lua"))

workspace "Sandbox"
	targetdir "build"
	startproject "Sandbox"
	
	configurations 
	{ 
		"Debug", 
		"Release",
		"Dist"
	}

group "Ant"
project "Ant-ScriptCore"
	location "%{AntRootDirectory}/Ant-ScriptCore"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetdir ("%{AntRootDirectory}/AntPlay/Resources/Scripts")
	objdir ("%{AntRootDirectory}/AntPlay/Resources/Scripts/Intermediates")

	files
	{
		"%{AntRootDirectory}/Ant-ScriptCore/Source/**.cs",
		"%{AntRootDirectory}/Ant-ScriptCore/Properties/**.cs"
	}

	filter "configurations:Debug"
		optimize "Off"
		symbols "Default"

	filter "configurations:Release"
		optimize "On"
		symbols "Default"

	filter "configurations:Dist"
		optimize "Full"
		symbols "Off"

group ""

project "Sandbox"
	location "Assets/Scripts"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetname "Sandbox"
	targetdir ("%{prj.location}/Binaries")
	objdir ("%{prj.location}/Intermediates")

	files 
	{
		"Assets/Scripts/Source/**.cs", 
	}

	linkAppReferences()

	filter "configurations:Debug"
		optimize "Off"
		symbols "Default"

	filter "configurations:Release"
		optimize "On"
		symbols "Default"

	filter "configurations:Dist"
		optimize "Full"
		symbols "Off"
