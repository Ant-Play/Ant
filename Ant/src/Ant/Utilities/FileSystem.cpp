#include "antpch.h"
#include "FileSystem.h"
#include "StringUtils.h"

namespace Ant{

	bool FileSystem::CreateDirectory(const std::filesystem::path& directory)
	{
		return std::filesystem::create_directories(directory);
	}

	bool FileSystem::CreateDirectory(const std::string& directory)
	{
		return CreateDirectory(std::filesystem::path(directory));
	}

	bool FileSystem::Move(const std::filesystem::path& oldFilepath, const std::filesystem::path& newFilepath)
	{
		if (FileSystem::Exists(newFilepath))
			return false;

		std::filesystem::rename(oldFilepath, newFilepath);
		return true;
	}

	bool FileSystem::Copy(const std::filesystem::path& oldFilepath, const std::filesystem::path& newFilepath)
	{
		if (FileSystem::Exists(newFilepath))
			return false;

		std::filesystem::copy(oldFilepath, newFilepath);
		return true;
	}

	bool FileSystem::MoveFile(const std::filesystem::path& filepath, const std::filesystem::path& dest)
	{
		return Move(filepath, dest / filepath.filename());
	}

	bool FileSystem::CopyFile(const std::filesystem::path& filepath, const std::filesystem::path& dest)
	{
		return Copy(filepath, dest / filepath.filename());
	}

	bool FileSystem::Rename(const std::filesystem::path& oldFilepath, const std::filesystem::path& newFilepath)
	{
		return Move(oldFilepath, newFilepath);
	}

	bool FileSystem::RenameFilename(const std::filesystem::path& oldFilepath, const std::string& newName)
	{
		std::filesystem::path newPath = fmt::format("{0}\\{1}{2}", oldFilepath.parent_path().string(), newName, oldFilepath.extension().string());
		return Rename(oldFilepath, newPath);
	}

	bool FileSystem::Exists(const std::filesystem::path& filepath)
	{
		return std::filesystem::exists(filepath);
	}

	bool FileSystem::Exists(const std::string& filepath)
	{
		return std::filesystem::exists(std::filesystem::path(filepath));
	}

	bool FileSystem::DeleteFile(const std::filesystem::path& filepath)
	{
		if (!FileSystem::Exists(filepath))
			return false;

		if (std::filesystem::is_directory(filepath))
			return std::filesystem::remove_all(filepath) > 0;
		return std::filesystem::remove(filepath);
	}

	bool FileSystem::IsDirectory(const std::filesystem::path& filepath)
	{
		return std::filesystem::is_directory(filepath);
	}

	// returns true <=> fileA was last modified more recently than fileB
	bool FileSystem::IsNewer(const std::filesystem::path& fileA, const std::filesystem::path& fileB)
	{
		return std::filesystem::last_write_time(fileA) > std::filesystem::last_write_time(fileB);
	}

	bool FileSystem::ShowFileInExplorer(const std::filesystem::path& path)
	{
		auto absolutePath = std::filesystem::canonical(path);
		if (!Exists(absolutePath))
			return false;

		std::string cmd = fmt::format("explorer.exe /select,\"{0}\"", absolutePath.string());
		system(cmd.c_str());
		return true;
	}

	bool FileSystem::OpenDirectoryInExplorer(const std::filesystem::path& path)
	{
		auto absolutePath = std::filesystem::canonical(path);
		if (!Exists(absolutePath))
			return false;

		ShellExecute(NULL, L"explore", absolutePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
		return true;
	}

	bool FileSystem::OpenExternally(const std::filesystem::path& path)
	{
		auto absolutePath = std::filesystem::canonical(path);
		if (!Exists(absolutePath))
			return false;

		ShellExecute(NULL, L"open", absolutePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
		return true;
	}

	std::filesystem::path FileSystem::GetUniqueFileName(const std::filesystem::path& filepath)
	{
		if (!FileSystem::Exists(filepath))
			return filepath;

		int counter = 0;
		auto checkID = [&counter, filepath](auto checkID) -> std::filesystem::path
		{
			++counter;
			const std::string counterStr = [&counter] {
				if (counter < 10)
					return "0" + std::to_string(counter);
				else
					return std::to_string(counter);
			}();  // Pad with 0 if < 10;

			std::string newFileName = fmt::format("{} ({})", Utils::RemoveExtension(filepath.filename().string()), counterStr);

			if (filepath.has_extension())
				newFileName = fmt::format("{}{}", newFileName, filepath.extension().string());

			if (std::filesystem::exists(filepath.parent_path() / newFileName))
				return checkID(checkID);
			else
				return filepath.parent_path() / newFileName;
		};

		return checkID(checkID);
	}
}