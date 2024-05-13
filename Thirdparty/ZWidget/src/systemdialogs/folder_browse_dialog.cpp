
#include "systemdialogs/folder_browse_dialog.h"
#include "core/widget.h"
#include "window/window.h"
#include <stdexcept>

#if defined(WIN32)
#include <Shlobj.h>

namespace
{
	static std::string from_utf16(const std::wstring& str)
	{
		if (str.empty()) return {};
		int needed = WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0, nullptr, nullptr);
		if (needed == 0)
			throw std::runtime_error("WideCharToMultiByte failed");
		std::string result;
		result.resize(needed);
		needed = WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], (int)result.size(), nullptr, nullptr);
		if (needed == 0)
			throw std::runtime_error("WideCharToMultiByte failed");
		return result;
	}

	static std::wstring to_utf16(const std::string& str)
	{
		if (str.empty()) return {};
		int needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
		if (needed == 0)
			throw std::runtime_error("MultiByteToWideChar failed");
		std::wstring result;
		result.resize(needed);
		needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], (int)result.size());
		if (needed == 0)
			throw std::runtime_error("MultiByteToWideChar failed");
		return result;
	}

	template<typename T>
	class ComPtr
	{
	public:
		ComPtr() { Ptr = nullptr; }
		ComPtr(const ComPtr& other) { Ptr = other.Ptr; if (Ptr) Ptr->AddRef(); }
		ComPtr(ComPtr&& move) { Ptr = move.Ptr; move.Ptr = nullptr; }
		~ComPtr() { reset(); }
		ComPtr& operator=(const ComPtr& other) { if (this != &other) { if (Ptr) Ptr->Release(); Ptr = other.Ptr; if (Ptr) Ptr->AddRef(); } return *this; }
		void reset() { if (Ptr) Ptr->Release(); Ptr = nullptr; }
		T* get() { return Ptr; }
		static IID GetIID() { return __uuidof(T); }
		void** InitPtr() { return (void**)TypedInitPtr(); }
		T** TypedInitPtr() { reset(); return &Ptr; }
		operator T* () const { return Ptr; }
		T* operator ->() const { return Ptr; }
		T* Ptr;
	};
}

class BrowseFolderDialogImpl : public BrowseFolderDialog
{
public:
	BrowseFolderDialogImpl(Widget *owner) : owner(owner)
	{
	}

	Widget *owner = nullptr;

	std::string selected_path;
	std::string initial_directory;
	std::string title;

	bool Show() override
	{
		std::wstring title16 = to_utf16(title);
		std::wstring initial_directory16 = to_utf16(initial_directory);

		HRESULT result;
		ComPtr<IFileOpenDialog> open_dialog;

		result = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, open_dialog.GetIID(), open_dialog.InitPtr());
		throw_if_failed(result, "CoCreateInstance(FileOpenDialog) failed");

		result = open_dialog->SetTitle(title16.c_str());
		throw_if_failed(result, "IFileOpenDialog.SetTitle failed");

		result = open_dialog->SetOptions(FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
		throw_if_failed(result, "IFileOpenDialog.SetOptions((FOS_PICKFOLDERS) failed");

		if (initial_directory16.length() > 0)
		{
			LPITEMIDLIST item_id_list = nullptr;
			SFGAOF flags = 0;
			result = SHParseDisplayName(initial_directory16.c_str(), nullptr, &item_id_list, SFGAO_FILESYSTEM, &flags);
			throw_if_failed(result, "SHParseDisplayName failed");

			ComPtr<IShellItem> folder_item;
			result = SHCreateShellItem(nullptr, nullptr, item_id_list, folder_item.TypedInitPtr());
			ILFree(item_id_list);
			throw_if_failed(result, "SHCreateItemFromParsingName failed");

			/* This code requires Windows Vista or newer:
			ComPtr<IShellItem> folder_item;
			result = SHCreateItemFromParsingName(initial_directory16.c_str(), nullptr, folder_item.GetIID(), folder_item.InitPtr());
			throw_if_failed(result, "SHCreateItemFromParsingName failed");
			*/

			if (folder_item)
			{
				result = open_dialog->SetFolder(folder_item);
				throw_if_failed(result, "IFileOpenDialog.SetFolder failed");
			}
		}

		if (owner && owner->Window())
			result = open_dialog->Show((HWND)owner->Window()->GetNativeHandle());
		else
			result = open_dialog->Show(0);

		if (SUCCEEDED(result))
		{
			ComPtr<IShellItem> chosen_folder;
			result = open_dialog->GetResult(chosen_folder.TypedInitPtr());
			throw_if_failed(result, "IFileOpenDialog.GetResult failed");

			WCHAR *buffer = nullptr;
			result = chosen_folder->GetDisplayName(SIGDN_FILESYSPATH, &buffer);
			throw_if_failed(result, "IShellItem.GetDisplayName failed");

			std::wstring output_directory16;
			if (buffer)
			{
				try
				{
					output_directory16 = buffer;
				}
				catch (...)
				{
					CoTaskMemFree(buffer);
					throw;
				}
			}

			CoTaskMemFree(buffer);
			selected_path = from_utf16(output_directory16);
			return true;
		}
		else
		{
			return false;
		}
	}

	std::string BrowseFolderDialog::SelectedPath() const override
	{
		return selected_path;
	}

	void BrowseFolderDialog::SetInitialDirectory(const std::string& path) override
	{
		initial_directory = path;
	}

	void BrowseFolderDialog::SetTitle(const std::string& newtitle) override
	{
		title = newtitle;
	}

	void throw_if_failed(HRESULT result, const std::string &error)
	{
		if (FAILED(result))
			throw std::runtime_error(error);
	}
};

std::unique_ptr<BrowseFolderDialog> BrowseFolderDialog::Create(Widget* owner)
{
	return std::make_unique<BrowseFolderDialogImpl>(owner);
}

#else

std::unique_ptr<BrowseFolderDialog> BrowseFolderDialog::Create(Widget* owner)
{
	return {};
}

#endif
