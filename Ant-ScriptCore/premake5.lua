project "Ant-ScriptCore"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.7.2"
    
    targetdir ("%{wks.location}/AntPlay/Resources/Scripts")
	objdir ("%{wks.location}/AntPlay/Resources/Scripts/Intermediates")

    files
    {
        "Source/**.cs",
        "Properties/**.cs",
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "GLFW_INCLUDE_NONE"
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