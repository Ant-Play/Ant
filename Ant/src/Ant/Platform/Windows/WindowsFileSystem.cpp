#include "antpch.h"
#include "Ant/Utilities/FileSystem.h"
#include "Ant/Asset/AssetManager.h"

#include "Ant/Core/Application.h"

#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Windows.h>
#include <Shlobj.h>

#include <filesystem>

namespace Ant{

	std::vector<FileSystem::FileSystemChangedCallbackFn> FileSystem::s_Callbacks;

	static bool s_Watching = false;
	static bool s_IgnoreNextChange = false;
	static HANDLE s_WatcherThread;
	static std::filesystem::path s_PersistentStoragePath;

	void FileSystem::AddFileSystemChangedCallback(const FileSystemChangedCallbackFn& callback)
	{
		s_Callbacks.push_back(callback);
	}

	void FileSystem::ClearFileSystemChangedCallbacks()
	{
		s_Callbacks.clear();
	}

	void FileSystem::StartWatching()
	{
		s_Watching = true;
		DWORD threadId;
		s_WatcherThread = CreateThread(NULL, 0, Watch, 0, 0, &threadId);
		ANT_CORE_ASSERT(s_WatcherThread != NULL);
		SetThreadDescription(s_WatcherThread, L"Ant FileSystemWatcher");
	}

	void FileSystem::StopWatching()
	{
		if (!s_Watching)
			return;

		s_Watching = false;
		//DWORD result = WaitForSingleObject(s_WatcherThread, 5000);
		//if (result == WAIT_TIMEOUT)
		// NOTE: this is a little annoying, but it's a quick and dirty
		//       way to shutdown the file watching ASAP.
		TerminateThread(s_WatcherThread, 0);
		CloseHandle(s_WatcherThread);
	}

	std::filesystem::path FileSystem::OpenFileDialog(const char* filter)
	{
		OPENFILENAMEA ofn;       // common dialog box structure
		CHAR szFile[260] = { 0 };       // if using TCHAR macros

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			std::string fp = ofn.lpstrFile;
			std::replace(fp.begin(), fp.end(), '\\', '/');
			return std::filesystem::path(fp);
		}

		return std::filesystem::path();
	}

	std::filesystem::path FileSystem::OpenFolderDialog(const char* initialFolder)
	{
		std::filesystem::path result = "";
		IFileOpenDialog* dialog;
		if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, (void**)&dialog)))
		{
			DWORD options;
			dialog->GetOptions(&options);
			dialog->SetOptions(options | FOS_PICKFOLDERS);
			if (SUCCEEDED(dialog->Show(NULL)))
			{
				IShellItem* selectedItem;
				if (SUCCEEDED(dialog->GetResult(&selectedItem)))
				{
					PWSTR pszFilePath;
					if (SUCCEEDED(selectedItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszFilePath)))
					{
						result = std::filesystem::path(pszFilePath, std::filesystem::path::generic_format);
						CoTaskMemFree(pszFilePath);
					}

					selectedItem->Release();
				}
			}

			dialog->Release();
		}

		std::string fp = result.string();
		std::replace(fp.begin(), fp.end(), '\\', '/');
		return fp;
	}

	std::filesystem::path FileSystem::SaveFileDialog(const char* filter)
	{
		OPENFILENAMEA ofn;       // common dialog box structure
		CHAR szFile[260] = { 0 };       // if using TCHAR macros
		
		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			std::string fp = ofn.lpstrFile;
			std::replace(fp.begin(), fp.end(), '\\', '/');
			return std::filesystem::path(fp);
		}

		return std::filesystem::path();
	}

	void FileSystem::SkipNextFileSystemChange()
	{
		s_IgnoreNextChange = true;
	}

	unsigned long FileSystem::Watch(void* param)
	{
		auto assetDirectory = Project::GetActive()->GetAssetDirectory();
		std::wstring dirStr = assetDirectory.wstring();

		char buf[2048];
		DWORD bytesReturned;
		std::filesystem::path filepath;
		BOOL result = TRUE;

		HANDLE directoryHandle = CreateFile(
			dirStr.c_str(),
			GENERIC_READ | FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL
		);

		if (directoryHandle == INVALID_HANDLE_VALUE)
		{
			ANT_CORE_VERIFY(false, "Failed to open directory!");
			return 0;
		}

		OVERLAPPED pollingOverlap;
		pollingOverlap.OffsetHigh = 0;
		pollingOverlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		std::vector<FileSystemChangedEvent> eventBatch;
		eventBatch.reserve(10);

		while (s_Watching && result)
		{
			result = ReadDirectoryChangesW(
				directoryHandle,
				&buf,
				sizeof(buf),
				TRUE,
				FILE_NOTIFY_CHANGE_FILE_NAME |
				FILE_NOTIFY_CHANGE_DIR_NAME |
				FILE_NOTIFY_CHANGE_SIZE,
				&bytesReturned,
				&pollingOverlap,
				NULL
			);

			WaitForSingleObject(pollingOverlap.hEvent, INFINITE);

			if (s_IgnoreNextChange)
			{
				s_IgnoreNextChange = false;
				eventBatch.clear();
				continue;
			}

			FILE_NOTIFY_INFORMATION* pNotify;
			int offset = 0;
			std::wstring oldName;

			do
			{
				pNotify = (FILE_NOTIFY_INFORMATION*)((char*)buf + offset);
				size_t filenameLength = pNotify->FileNameLength / sizeof(wchar_t);

				FileSystemChangedEvent e;
				e.FilePath = std::filesystem::path(std::wstring(pNotify->FileName, filenameLength));
				e.IsDirectory = IsDirectory(e.FilePath);

				switch (pNotify->Action)
				{
					case FILE_ACTION_ADDED:
					{
						e.Action = FileSystemAction::Added;
						break;
					}
					case FILE_ACTION_REMOVED:
					{
						e.Action = FileSystemAction::Delete;
						break;
					}
					case FILE_ACTION_MODIFIED:
					{
						e.Action = FileSystemAction::Modified;
						break;
					}
					case FILE_ACTION_RENAMED_OLD_NAME:
					{
						oldName = e.FilePath.filename();
						break;
					}
					case FILE_ACTION_RENAMED_NEW_NAME:
					{
						e.OldName = oldName;
						e.Action = FileSystemAction::Rename;
						break;
					}
				}

				// NOTE: Fix for https://gitlab.com/chernoprojects/Ant-dev/-/issues/143
				bool hasAddedEvent = false;
				if (e.Action == FileSystemAction::Modified)
				{
					for (const auto& event : eventBatch)
					{
						if (event.FilePath == e.FilePath && event.Action == FileSystemAction::Added)
							hasAddedEvent = true;
					}
				}

				if (pNotify->Action != FILE_ACTION_RENAMED_OLD_NAME && !hasAddedEvent)
					eventBatch.push_back(e);

				offset += pNotify->NextEntryOffset;
			} while (pNotify->NextEntryOffset);

			if (eventBatch.size() > 0)
			{
				for (auto& callback : s_Callbacks)
					callback(eventBatch);
				eventBatch.clear();
			}
		}

		CloseHandle(directoryHandle);
		return 0;
	}

	bool FileSystem::WriteBytes(const std::filesystem::path& filepath, const Buffer& buffer)
	{
		std::ofstream stream(filepath, std::ios::binary | std::ios::trunc);

		if (!stream)
		{
			stream.close();
			return false;
		}

		stream.write((char*)buffer.Data, buffer.Size);
		stream.close();

		return true;
	}

	Buffer FileSystem::ReadBytes(const std::filesystem::path& filepath)
	{
		Buffer buffer;

		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
		ANT_CORE_ASSERT(stream);

		auto end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		auto size = end - stream.tellg();
		ANT_CORE_ASSERT(size != 0);

		buffer.Allocate((uint32_t)size);
		stream.read((char*)buffer.Data, buffer.Size);
		stream.close();

		return buffer;
	}

	std::filesystem::path FileSystem::GetPersistentStoragePath()
	{
		if (!s_PersistentStoragePath.empty())
			return s_PersistentStoragePath;

		PWSTR roamingFilePath;
		HRESULT result = SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT, NULL, &roamingFilePath);
		ANT_CORE_VERIFY(result == S_OK);
		s_PersistentStoragePath = roamingFilePath;
		s_PersistentStoragePath /= "AntPlay";

		if (!std::filesystem::exists(s_PersistentStoragePath))
			std::filesystem::create_directory(s_PersistentStoragePath);

		return s_PersistentStoragePath;
	}

	bool FileSystem::HasEnvironmentVariable(const std::string& key)
	{
		HKEY hKey;
		LPCSTR keyPath = "Environment";
		LSTATUS lOpenStatus = RegOpenKeyExA(HKEY_CURRENT_USER, keyPath, 0, KEY_ALL_ACCESS, &hKey);

		if (lOpenStatus == ERROR_SUCCESS)
		{
			lOpenStatus = RegQueryValueExA(hKey, key.c_str(), 0, NULL, NULL, NULL);
			RegCloseKey(hKey);
		}

		return lOpenStatus == ERROR_SUCCESS;
	}

	bool FileSystem::SetEnvironmentVariable(const std::string& key, const std::string& value)
	{
		HKEY hKey;
		LPCSTR keyPath = "Environment";
		DWORD createdNewKey;
		LSTATUS lOpenStatus = RegCreateKeyExA(HKEY_CURRENT_USER, keyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &createdNewKey);
		if (lOpenStatus == ERROR_SUCCESS)
		{
			LSTATUS lSetStatus = RegSetValueExA(hKey, key.c_str(), 0, REG_SZ, (LPBYTE)value.c_str(), (DWORD)(value.length() + 1));
			RegCloseKey(hKey);

			if (lSetStatus == ERROR_SUCCESS)
			{
				SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_BLOCK, 100, NULL);
				return true;
			}
		}

		return false;
	}

	std::string FileSystem::GetEnvironmentVariable(const std::string& key)
	{
		HKEY hKey;
		LPCSTR keyPath = "Environment";
		DWORD createdNewKey;
		LSTATUS lOpenStatus = RegCreateKeyExA(HKEY_CURRENT_USER, keyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &createdNewKey);
		if (lOpenStatus == ERROR_SUCCESS)
		{
			DWORD valueType;
			char* data = anew char[512];
			DWORD dataSize = 512;
			LSTATUS status = RegGetValueA(hKey, NULL, key.c_str(), RRF_RT_ANY, &valueType, (PVOID)data, &dataSize);

			RegCloseKey(hKey);

			if (status == ERROR_SUCCESS)
			{
				std::string result(data);
				delete[] data;
				return result;
			}
		}

		return std::string{};
	}
}