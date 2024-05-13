
#include "systemdialogs/open_file_dialog.h"
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

class OpenFileDialogImpl : public OpenFileDialog
{
public:
	OpenFileDialogImpl(Widget* owner) : owner(owner)
	{
	}

	Widget* owner = nullptr;

	std::string initial_directory;
	std::string initial_filename;
	std::string title;
	std::vector<std::string> filenames;
	bool multi_select = false;

	struct Filter
	{
		std::string description;
		std::string extension;
	};
	std::vector<Filter> filters;
	int filterindex = 0;
	std::string defaultext;

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

		if (!initial_filename.empty())
		{
			result = open_dialog->SetFileName(to_utf16(initial_filename).c_str());
			throw_if_failed(result, "IFileOpenDialog.SetFileName failed");
		}

		FILEOPENDIALOGOPTIONS options = FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST;
		if (multi_select)
			options |= FOS_ALLOWMULTISELECT;
		result = open_dialog->SetOptions(options);
		throw_if_failed(result, "IFileOpenDialog.SetOptions() failed");

		if (!filters.empty())
		{
			std::vector<COMDLG_FILTERSPEC> filterspecs(filters.size());
			std::vector<std::wstring> descriptions(filters.size());
			std::vector<std::wstring> extensions(filters.size());
			for (size_t i = 0; i < filters.size(); i++)
			{
				descriptions[i] = to_utf16(filters[i].description);
				extensions[i] = to_utf16(filters[i].extension);
				COMDLG_FILTERSPEC& spec = filterspecs[i];
				spec.pszName = descriptions[i].c_str();
				spec.pszSpec = extensions[i].c_str();
			}
			result = open_dialog->SetFileTypes((UINT)filterspecs.size(), filterspecs.data());
			throw_if_failed(result, "IFileOpenDialog.SetFileTypes() failed");

			if ((size_t)filterindex < filters.size())
			{
				result = open_dialog->SetFileTypeIndex(filterindex);
				throw_if_failed(result, "IFileOpenDialog.SetFileTypeIndex() failed");
			}
		}

		if (!defaultext.empty())
		{
			result = open_dialog->SetDefaultExtension(to_utf16(defaultext).c_str());
			throw_if_failed(result, "IFileOpenDialog.SetDefaultExtension() failed");
		}

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
			ComPtr<IShellItemArray> items;
			result = open_dialog->GetSelectedItems(items.TypedInitPtr());
			throw_if_failed(result, "IFileOpenDialog.GetSelectedItems failed");

			DWORD num_items = 0;
			result = items->GetCount(&num_items);
			throw_if_failed(result, "IShellItemArray.GetCount failed");

			for (DWORD i = 0; i < num_items; i++)
			{
				ComPtr<IShellItem> item;
				result = items->GetItemAt(i, item.TypedInitPtr());
				throw_if_failed(result, "IShellItemArray.GetItemAt failed");

				WCHAR* buffer = nullptr;
				result = item->GetDisplayName(SIGDN_FILESYSPATH, &buffer);
				throw_if_failed(result, "IShellItem.GetDisplayName failed");

				std::wstring output16;
				if (buffer)
				{
					try
					{
						output16 = buffer;
					}
					catch (...)
					{
						CoTaskMemFree(buffer);
						throw;
					}
				}

				CoTaskMemFree(buffer);
				filenames.push_back(from_utf16(output16));
			}
			return true;
		}
		else
		{
			return false;
		}
	}

	std::string Filename() const override
	{
		return !filenames.empty() ? filenames.front() : std::string();
	}

	std::vector<std::string> Filenames() const override
	{
		return filenames;
	}

	void SetMultiSelect(bool new_multi_select) override
	{
		multi_select = new_multi_select;
	}

	void SetFilename(const std::string& filename) override
	{
		initial_filename = filename;
	}

	void AddFilter(const std::string& filter_description, const std::string& filter_extension) override
	{
		Filter f;
		f.description = filter_description;
		f.extension = filter_extension;
		filters.push_back(std::move(f));
	}

	void ClearFilters() override
	{
		filters.clear();
	}

	void SetFilterIndex(int filter_index) override
	{
		filterindex = filter_index;
	}

	void SetInitialDirectory(const std::string& path) override
	{
		initial_directory = path;
	}

	void SetTitle(const std::string& newtitle) override
	{
		title = newtitle;
	}

	void SetDefaultExtension(const std::string& extension) override
	{
		defaultext = extension;
	}

	void throw_if_failed(HRESULT result, const std::string& error)
	{
		if (FAILED(result))
			throw std::runtime_error(error);
	}
};

std::unique_ptr<OpenFileDialog> OpenFileDialog::Create(Widget* owner)
{
	return std::make_unique<OpenFileDialogImpl>(owner);
}

#else

std::unique_ptr<OpenFileDialog> OpenFileDialog::Create(Widget* owner)
{
	return {};
}

#endif
