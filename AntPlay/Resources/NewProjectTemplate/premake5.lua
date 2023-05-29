AntRootDirectory = os.getenv("Ant_DIR")
include (path.join(AntRootDirectory, "AntPlay", "Resources", "LUA", "Ant.lua"))

workspace "$PROJECT_NAME$"
	targetdir "build"
	startproject "$PROJECT_NAME$"
	
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

project "$PROJECT_NAME$"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetname "$PROJECT_NAME$"
	targetdir ("%{prj.location}/Assets/Scripts/Binaries")
	objdir ("%{prj.location}/Intermediates")

	files 
	{
		"Assets/**.cs", 
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
