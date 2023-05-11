project "ExampleApp"
	location "ExampleApp"
	kind "SharedLib"
	language "C#"

    targetdir ("%{wks.location}/AntPlay/assets/scripts")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files 
	{
		"%{prj.name}/src/**.cs", 
	}

	links
	{
		"Ant-ScriptCore"
	}