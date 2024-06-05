
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
			result = open_dialog->GetResults(items.TypedInitPtr());
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

#elif defined(__APPLE__)

class OpenFileDialogImpl : public OpenFileDialog
{
public:
	OpenFileDialogImpl(Widget* owner)
	{
	}

	std::string Filename() const override
	{
		return {};
	}

	std::vector<std::string> Filenames() const override
	{
		return {};
	}

	void SetMultiSelect(bool multiselect) override
	{
	}

	void SetFilename(const std::string &filename) override
	{
	}

	void SetDefaultExtension(const std::string& extension) override
	{
	}

	void AddFilter(const std::string &filter_description, const std::string &filter_extension) override
	{
	}

	void ClearFilters() override
	{
	}

	void SetFilterIndex(int filter_index) override
	{
	}

	void SetInitialDirectory(const std::string &path) override
	{
	}

	void SetTitle(const std::string &title) override
	{
	}

	bool Show() override
	{
		return false;
	}
};

std::unique_ptr<OpenFileDialog> OpenFileDialog::Create(Widget* owner)
{
	return std::make_unique<OpenFileDialogImpl>(owner);
}

#else

#ifdef USE_DBUS
#include <dbus/dbus.h>
#include <iostream>
#endif

class OpenFileDialogImpl : public OpenFileDialog
{
public:
	OpenFileDialogImpl(Widget* owner) : owner(owner)
	{
	}

	std::string Filename() const override
	{
		return outputFilenames.empty() ? std::string() : outputFilenames.front();
	}

	std::vector<std::string> Filenames() const override
	{
		return outputFilenames;
	}

	void SetMultiSelect(bool multiselect) override
	{
		this->multiSelect = multiSelect;
	}

	void SetFilename(const std::string &filename) override
	{
		inputFilename = filename;
	}

	void SetDefaultExtension(const std::string& extension) override
	{
		defaultExt = extension;
	}

	void AddFilter(const std::string &filter_description, const std::string &filter_extension) override
	{
		filters.push_back({ filter_description, filter_extension });
	}

	void ClearFilters() override
	{
		filters.clear();
	}

	void SetFilterIndex(int filter_index) override
	{
		this->filter_index = filter_index;
	}

	void SetInitialDirectory(const std::string &path) override
	{
		initialDirectory = path;
	}

	void SetTitle(const std::string &title) override
	{
		this->title = title;
	}

#ifdef USE_DBUS
	bool Show() override
	{
		// https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.FileChooser.html
		
		std::string ownerHandle;
		if (owner)
		{
			// To do: create a way to detect if the window is x11 vs wayland
			ownerHandle = "x11:" + std::to_string(reinterpret_cast<unsigned long>(owner->GetNativeHandle()));
			// ownerHandle = "wayland:" + owner->GetNativeHandle(); // "wayland:~12l9jdl.-a"
		}

		dbus_bool_t bresult = {};
		DBusError error = {};
		dbus_error_init(&error);
		
		DBusConnection* connection = dbus_bus_get(DBUS_BUS_SESSION, &error);
		if (!connection)
		{
			dbus_error_free(&error);
			return false;
		}

		std::string busname = dbus_bus_get_unique_name(connection);

		DBusMessage* request = dbus_message_new_method_call("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.freedesktop.portal.FileChooser", "OpenFile");
		if (!request)
		{
			dbus_connection_unref(connection);
			dbus_error_free(&error);
			return false;
		}

		const char* parentWindow = ownerHandle.c_str();
		const char* title = this->title.c_str();

		DBusMessageIter requestArgs = {};
		dbus_message_iter_init_append(request, &requestArgs);

		bresult = dbus_message_iter_append_basic(&requestArgs, DBUS_TYPE_STRING, &parentWindow);
		bresult = dbus_message_iter_append_basic(&requestArgs, DBUS_TYPE_STRING, &title);

		DBusMessageIter requestOptions = {};
		bresult = dbus_message_iter_open_container(&requestArgs, DBUS_TYPE_ARRAY, "{sv}", &requestOptions);

		// handle_token - s         race condition prevention for signal (/org/freedesktop/portal/desktop/request/SENDER/TOKEN)
		// accept_label - s         text label for the OK button
		// modal - b                makes dialog modal. Defaults to true. Not really sure what it means since nothing stayed modal on my computer
		// directory - b            open folder mode, added in version 3
		// filters - a(sa(us))      [('Images', [(0, '*.ico'), (1, 'image/png')]), ('Text', [(0, '*.txt')])]
		// current_filter - sa(us)  filter from filters that should be current filter
		// choices - a(ssa(ss)s)    list of serialized combo boxes to add to the file chooser
		// current_name - s         suggested name

		// current_folder - ay
		if (!initialDirectory.empty())
		{
			// Note: the docs unfortunately says "The portal implementation is free to ignore this option"
			
			const char* key = "current_folder";
			const char* value = initialDirectory.c_str();
			int valueCount = (int)initialDirectory.size() + 1;

			DBusMessageIter entry = {};
			bresult = dbus_message_iter_open_container(&requestOptions, DBUS_TYPE_DICT_ENTRY, nullptr, &entry);
			bresult = dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);

			DBusMessageIter variant = {};
			DBusMessageIter array = {};
			bresult = dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "ay", &variant);
			bresult = dbus_message_iter_open_container(&variant, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &array);
			bresult = dbus_message_iter_append_fixed_array(&array, DBUS_TYPE_BYTE, &value, valueCount);
			bresult = dbus_message_iter_close_container(&variant, &array);
			bresult = dbus_message_iter_close_container(&entry, &variant);

			bresult = dbus_message_iter_close_container(&requestOptions, &entry);
		}

		// multiple - b
		{
			const char* key = "multiple";
			dbus_bool_t value = multiSelect;

			DBusMessageIter entry = {};
			bresult = dbus_message_iter_open_container(&requestOptions, DBUS_TYPE_DICT_ENTRY, nullptr, &entry);
			bresult = dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);

			DBusMessageIter variant = {};
			bresult = dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &variant);
			bresult = dbus_message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &value);
			bresult = dbus_message_iter_close_container(&entry, &variant);

			bresult = dbus_message_iter_close_container(&requestOptions, &entry);
		}

		bresult = dbus_message_iter_close_container(&requestArgs, &requestOptions);

		DBusMessage* response = dbus_connection_send_with_reply_and_block(connection, request, DBUS_TIMEOUT_USE_DEFAULT, &error);
		if (!response)
		{
			dbus_message_unref(request);
			dbus_connection_unref(connection);
			dbus_error_free(&error);
			return false;
		}

		const char* handle = nullptr;
		bresult = dbus_message_get_args(response, &error, DBUS_TYPE_OBJECT_PATH, &handle, DBUS_TYPE_INVALID);
		if (!bresult)
		{
			dbus_message_unref(response);
			dbus_message_unref(request);
			dbus_connection_unref(connection);
			dbus_error_free(&error);
			return false;
		}

		std::string signalObjectPath = handle;

		dbus_message_unref(response);
		dbus_message_unref(request);

		std::string rule = "type='signal',interface='org.freedesktop.portal.Request',member='Response',path='" + signalObjectPath + "'";
		dbus_bus_add_match(connection, rule.c_str(), &error);

		// Wait for the response signal
		//
		// To do: process the run loop while we wait
		//
		DBusMessage* signalmsg = nullptr;
		while (!signalmsg && dbus_connection_read_write(connection, -1))
		{
			while (true)
			{
				DBusMessage* message = dbus_connection_pop_message(connection);
				if (!message)
					break;

				if (dbus_message_is_signal(message, "org.freedesktop.portal.Request", "Response") && dbus_message_get_path(message) == signalObjectPath)
				{
					signalmsg = message;
					break;
				}
				else
				{
					dbus_message_unref(message);
				}
			}
		}
		dbus_bus_remove_match(connection, rule.c_str(), &error);

		// Read the response

		dbus_uint32_t responseCode = 0;
		std::vector<std::string> uris;

		DBusMessageIter signalArgs;
		bresult = dbus_message_iter_init(signalmsg, &signalArgs);

		// response code - u
		if (dbus_message_iter_get_arg_type(&signalArgs) == DBUS_TYPE_UINT32)
		{
			dbus_message_iter_get_basic(&signalArgs, &responseCode);
		}
		dbus_message_iter_next(&signalArgs);

		// results - a{sv}
		if (dbus_message_iter_get_arg_type(&signalArgs) == DBUS_TYPE_ARRAY)
		{
			DBusMessageIter resultsArray = {};
			dbus_message_iter_recurse(&signalArgs, &resultsArray);
			while (true)
			{
				int type = dbus_message_iter_get_arg_type(&resultsArray);
				if (type != DBUS_TYPE_DICT_ENTRY)
					break;

				DBusMessageIter entry = {};
				dbus_message_iter_recurse(&resultsArray, &entry);

				const char* key = nullptr;
				dbus_message_iter_get_basic(&entry, &key);
				dbus_message_iter_next(&entry);

				DBusMessageIter value = {};
				dbus_message_iter_recurse(&entry, &value);

				std::string k = key;
				if (k == "uris" && dbus_message_iter_get_arg_type(&value) == DBUS_TYPE_ARRAY) // as
				{
					DBusMessageIter uriArray = {};
					dbus_message_iter_recurse(&value, &uriArray);
					while (true)
					{
						int type = dbus_message_iter_get_arg_type(&uriArray);
						if (type != DBUS_TYPE_STRING)
							break;

						const char* uri = nullptr;
						dbus_message_iter_get_basic(&uriArray, &uri);
						uris.push_back(uri);

						dbus_message_iter_next(&uriArray);
					}
				}
				else if (k == "choices" && dbus_message_iter_get_arg_type(&value) == DBUS_TYPE_ARRAY) // a(ss)
				{
					// An array of pairs of strings,
					// the first string being the ID of a combobox that was passed into this call,
					// the second string being the selected option.
				}
				else if (k == "current_filter" && dbus_message_iter_get_arg_type(&value) == DBUS_TYPE_STRUCT) // sa(us)
				{
					// The filter that was selected.
					// This may match a filter in the filter list or another filter that was applied unconditionally.
				}
			
				dbus_message_iter_next(&entry);
				dbus_message_iter_next(&resultsArray);
			}
		}
		dbus_message_iter_next(&signalArgs);

		dbus_message_unref(signalmsg);
		dbus_connection_unref(connection);
		dbus_error_free(&error);

		if (responseCode != 0) // User cancelled
			return false;

		for (const std::string& uri : uris)
		{
			if (uri.size() > 7 && uri.substr(0, 7) == "file://")
				outputFilenames.push_back(uri.substr(7));
		}
		
		return !uris.empty();
	}
#else
	bool Show() override
	{
		return false;
	}
#endif

	Widget* owner = nullptr;
	std::string title;
	std::string initialDirectory;
	std::string inputFilename;
	std::string defaultExt;
	std::vector<std::string> outputFilenames;
	bool multiSelect = false;

	struct Filter
	{
		std::string description;
		std::string extension;
	};
	std::vector<Filter> filters;
	int filter_index = 0;
};

std::unique_ptr<OpenFileDialog> OpenFileDialog::Create(Widget* owner)
{
	return std::make_unique<OpenFileDialogImpl>(owner);
}

#endif
