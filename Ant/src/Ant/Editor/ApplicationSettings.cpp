#include "antpch.h"
#include "ApplicationSettings.h"
#include "Ant/Utilities/FileSystem.h"

#include <yaml-cpp/yaml.h>

#include <sstream>
#include <filesystem>

namespace Ant {

	static std::filesystem::path s_EditorSettingsPath;

	ApplicationSettings& ApplicationSettings::Get()
	{
		static ApplicationSettings s_Settings;
		return s_Settings;
	}

	void ApplicationSettingsSerializer::Init()
	{
		s_EditorSettingsPath = std::filesystem::absolute("Config");

		if (!FileSystem::Exists(s_EditorSettingsPath))
			FileSystem::CreateDirectory(s_EditorSettingsPath);
		s_EditorSettingsPath /= "ApplicationSettings.yaml";

		LoadSettings();
	}

#define ANT_ENTER_GROUP(name) currentGroup = rootNode[name];
#define ANT_READ_VALUE(name, type, var, defaultValue) var = currentGroup[name].as<type>(defaultValue)

	void ApplicationSettingsSerializer::LoadSettings()
	{
		// Generate default settings file if one doesn't exist
		if (!FileSystem::Exists(s_EditorSettingsPath))
		{
			SaveSettings();
			return;
		}

		std::ifstream stream(s_EditorSettingsPath);
		ANT_CORE_VERIFY(stream);
		std::stringstream ss;
		ss << stream.rdbuf();

		YAML::Node data = YAML::Load(ss.str());
		if (!data["ApplicationSettings"])
			return;

		YAML::Node rootNode = data["ApplicationSettings"];
		YAML::Node currentGroup = rootNode;

		auto& settings = ApplicationSettings::Get();

		ANT_ENTER_GROUP("AntPlay");
		{
			ANT_READ_VALUE("AdvancedMode", bool, settings.AdvancedMode, false);
		}

		ANT_ENTER_GROUP("Scripting");
		{
			ANT_READ_VALUE("ShowHiddenFields", bool, settings.ShowHiddenFields, false);
			ANT_READ_VALUE("DebuggerListenPort", int, settings.ScriptDebuggerListenPort, 2550);
		}

		ANT_ENTER_GROUP("ContentBrowser");
		{
			ANT_READ_VALUE("ShowAssetTypes", bool, settings.ContentBrowserShowAssetTypes, true);
			ANT_READ_VALUE("ThumbnailSize", int, settings.ContentBrowserThumbnailSize, 128);
		}

		stream.close();
	}

#define ANT_BEGIN_GROUP(name)\
		out << YAML::Key << name << YAML::Value << YAML::BeginMap;
#define ANT_END_GROUP() out << YAML::EndMap;

#define ANT_SERIALIZE_VALUE(name, value) out << YAML::Key << name << YAML::Value << value;

	void ApplicationSettingsSerializer::SaveSettings()
	{
		const auto& settings = ApplicationSettings::Get();

		YAML::Emitter out;
		out << YAML::BeginMap;
		ANT_BEGIN_GROUP("ApplicationSettings");
		{
			ANT_BEGIN_GROUP("AntPlay");
			{
				ANT_SERIALIZE_VALUE("AdvancedMode", settings.AdvancedMode);
			}
			ANT_END_GROUP();

			ANT_BEGIN_GROUP("Scripting");
			{
				ANT_SERIALIZE_VALUE("ShowHiddenFields", settings.ShowHiddenFields);
				ANT_SERIALIZE_VALUE("DebuggerListenPort", settings.ScriptDebuggerListenPort);
			}
			ANT_END_GROUP();

			ANT_BEGIN_GROUP("ContentBrowser");
			{
				ANT_SERIALIZE_VALUE("ShowAssetTypes", settings.ContentBrowserShowAssetTypes);
				ANT_SERIALIZE_VALUE("ThumbnailSize", settings.ContentBrowserThumbnailSize);
			}
			ANT_END_GROUP();
		}
		ANT_END_GROUP();
		out << YAML::EndMap;

		std::ofstream fout(s_EditorSettingsPath);
		fout << out.c_str();
		fout.close();
	}
}