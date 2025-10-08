/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_LIBRARY_LIBRARY_HPP
#define MPT_LIBRARY_LIBRARY_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/base/utility.hpp"
#include "mpt/detect/dl.hpp"
#include "mpt/detect/ltdl.hpp"
#include "mpt/fs/common_directories.hpp"
#include "mpt/fs/fs.hpp"
#include "mpt/osinfo/windows_version.hpp"
#include "mpt/path/native_path.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"

#include <optional>
#include <string>
#include <type_traits>
#include <vector>

#if MPT_OS_WINDOWS
#include <windows.h>
#elif MPT_OS_ANDROID
#elif defined(MPT_WITH_DL)
#include <dlfcn.h>
#elif defined(MPT_WITH_LTDL)
#include <ltdl.h>
#endif



namespace mpt {
inline namespace MPT_INLINE_NS {



#if MPT_OS_WINDOWS

// KB2533623 / Win8
#ifdef LOAD_LIBRARY_SEARCH_DEFAULT_DIRS
#define MPT_LOAD_LIBRARY_SEARCH_DEFAULT_DIRS LOAD_LIBRARY_SEARCH_DEFAULT_DIRS
#else
#define MPT_LOAD_LIBRARY_SEARCH_DEFAULT_DIRS 0x00001000
#endif
#ifdef LOAD_LIBRARY_SEARCH_APPLICATION_DIR
#define MPT_LOAD_LIBRARY_SEARCH_APPLICATION_DIR LOAD_LIBRARY_SEARCH_APPLICATION_DIR
#else
#define MPT_LOAD_LIBRARY_SEARCH_APPLICATION_DIR 0x00000200
#endif
#ifdef LOAD_LIBRARY_SEARCH_SYSTEM32
#define MPT_LOAD_LIBRARY_SEARCH_SYSTEM32 LOAD_LIBRARY_SEARCH_SYSTEM32
#else
#define MPT_LOAD_LIBRARY_SEARCH_SYSTEM32 0x00000800
#endif
#ifdef LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR
#define MPT_LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR
#else
#define MPT_LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR 0x00000100
#endif

#endif // MPT_OS_WINDOWS



class library {

public:
	typedef void * (*func_ptr)(); // pointer to function returning void *
	static_assert(sizeof(func_ptr) == sizeof(void *));

	enum class path_search {
		invalid,
		unsafe,
		default_,
		system,
		application,
		none,
	};

	enum class path_prefix {
		none,
		default_,
	};

	enum class path_suffix {
		none,
		default_,
	};

	struct path {

	public:
		path_search search = path_search::invalid;
		path_prefix prefix = path_prefix::default_;
		mpt::native_path filename{};
		path_suffix suffix = path_suffix::default_;

		static mpt::native_path default_prefix() {
#if MPT_OS_WINDOWS
			return MPT_NATIVE_PATH("");
#else
			return MPT_NATIVE_PATH("lib");
#endif
		}

		static mpt::native_path default_suffix() {
#if MPT_OS_WINDOWS
			return MPT_NATIVE_PATH(".dll");
#elif MPT_OS_ANDROID || defined(MPT_WITH_DL)
			return MPT_NATIVE_PATH(".so");
#else
			return MPT_NATIVE_PATH("");
#endif
		}

		std::optional<mpt::native_path> get_effective_filename() const {
			switch (search) {
				case library::path_search::unsafe:
					break;
				case library::path_search::default_:
					break;
				case library::path_search::system:
					if (filename.is_absolute()) {
						return std::nullopt;
					}
					break;
				case library::path_search::application:
					if (filename.is_absolute()) {
						return std::nullopt;
					}
					break;
				case library::path_search::none:
					if (filename.is_relative()) {
						return std::nullopt;
					}
					break;
				case library::path_search::invalid:
					return std::nullopt;
					break;
			}
			mpt::native_path result{};
			result += filename.GetDirectoryWithDrive();
			result += ((prefix == path_prefix::default_) ? default_prefix() : mpt::native_path{});
			result += filename.GetFilenameBase();
			result += ((suffix == path_suffix::default_) ? default_suffix() : mpt::native_path{});
			return result;
		}
	};

#if MPT_OS_WINDOWS

private:
	HMODULE m_hModule = NULL;

	library(HMODULE hModule)
		: m_hModule(hModule) {
		return;
	}

public:
	library(const library &) = delete;
	library & operator=(const library &) = delete;

	library(library && other) noexcept
		: m_hModule(other.m_hModule) {
		other.m_hModule = NULL;
	}

	library & operator=(library && other) noexcept {
		FreeLibrary(m_hModule);
		m_hModule = other.m_hModule;
		other.m_hModule = NULL;
		return *this;
	}

public:
	static std::optional<library> load(mpt::library::path path) {

		HMODULE hModule = NULL;

		std::optional<mpt::native_path> optionalfilename = path.get_effective_filename();
		if (!optionalfilename) {
			return std::nullopt;
		}
#if defined(MPT_LIBCXX_QUIRK_NO_OPTIONAL_VALUE)
		mpt::native_path & filename = *optionalfilename;
#else
		mpt::native_path & filename = optionalfilename.value();
#endif
		if (filename.empty()) {
			return std::nullopt;
		}

#if MPT_OS_WINDOWS_WINRT

#if MPT_WINRT_BEFORE(MPT_WIN_8)
		MPT_UNUSED(path);
		return std::nullopt;
#else  // Windows 8
		switch (path.search) {
			case library::path_search::unsafe:
				hModule = ::LoadPackagedLibrary(filename.AsNative().c_str(), 0);
				break;
			case library::path_search::default_:
				hModule = ::LoadPackagedLibrary(filename.AsNative().c_str(), 0);
				break;
			case library::path_search::system:
				hModule = NULL; // Only application packaged libraries can be loaded dynamically in WinRT
				break;
			case library::path_search::application:
				hModule = ::LoadPackagedLibrary(filename.AsNative().c_str(), 0);
				break;
			case library::path_search::none:
				hModule = NULL; // Absolute path is not supported in WinRT
				break;
			case library::path_search::invalid:
				hModule = NULL;
				break;
		}
#endif // Windows Version

#else // !MPT_OS_WINDOWS_WINRT

#if MPT_WINNT_AT_LEAST(MPT_WIN_8) && 0
		bool hasKB2533623 = true;
#else
		bool hasKB2533623 = false;
		mpt::osinfo::windows::Version WindowsVersion = mpt::osinfo::windows::Version::Current();
		if (WindowsVersion.IsAtLeast(mpt::osinfo::windows::Version::Win8)) {
			hasKB2533623 = true;
		} else if (WindowsVersion.IsAtLeast(mpt::osinfo::windows::Version::WinVista)) {
			HMODULE hKernel32DLL = ::LoadLibrary(TEXT("kernel32.dll"));
			if (hKernel32DLL) {
				if (::GetProcAddress(hKernel32DLL, "SetDefaultDllDirectories") != nullptr) {
					hasKB2533623 = true;
				}
				::FreeLibrary(hKernel32DLL);
				hKernel32DLL = NULL;
			}
		}
#endif

		MPT_MAYBE_CONSTANT_IF (hasKB2533623) {

			switch (path.search) {
				case library::path_search::unsafe:
					hModule = ::LoadLibrary(filename.AsNative().c_str());
					break;
				case library::path_search::default_:
					hModule = ::LoadLibraryEx(filename.AsNative().c_str(), NULL, MPT_LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
					break;
				case library::path_search::system:
					hModule = ::LoadLibraryEx(filename.AsNative().c_str(), NULL, MPT_LOAD_LIBRARY_SEARCH_SYSTEM32);
					break;
				case library::path_search::application:
					hModule = ::LoadLibraryEx(filename.AsNative().c_str(), NULL, MPT_LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
					break;
				case library::path_search::none:
					hModule = ::LoadLibraryEx(filename.AsNative().c_str(), NULL, MPT_LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
					break;
				case library::path_search::invalid:
					hModule = NULL;
					break;
			}

		} else {

			switch (path.search) {
				case library::path_search::unsafe:
					hModule = ::LoadLibrary(filename.AsNative().c_str());
					break;
				case library::path_search::default_:
					hModule = ::LoadLibrary(filename.AsNative().c_str());
					break;
				case library::path_search::system:
					{
						mpt::native_path system_path = mpt::common_directories::get_system_directory();
						if (system_path.empty()) {
							hModule = NULL;
						} else {
							hModule = ::LoadLibrary((system_path.WithTrailingSlash() + filename).AsNative().c_str());
						}
					}
					break;
				case library::path_search::application:
					{
						mpt::native_path application_path = mpt::common_directories::get_application_directory();
						if (application_path.empty()) {
							hModule = NULL;
						} else {
							hModule = ::LoadLibrary((application_path.WithTrailingSlash() + filename).AsNative().c_str());
						}
					}
					break;
				case library::path_search::none:
					hModule = ::LoadLibrary(filename.AsNative().c_str());
					break;
				case library::path_search::invalid:
					hModule = NULL;
					break;
			}
		}

#endif // MPT_OS_WINDOWS_WINRT

		if (!hModule) {
			return std::nullopt;
		}

		return library{hModule};
	}

	auto get_address(const std::string & symbol) const {
		return ::GetProcAddress(m_hModule, symbol.c_str());
	}

	~library() {
		FreeLibrary(m_hModule);
	}

#elif defined(MPT_WITH_DL)

private:
	void * handle = nullptr;

	library(void * handle_)
		: handle(handle_) {
		return;
	}

public:
	library(const library &) = delete;
	library & operator=(const library &) = delete;

	library(library && other) noexcept
		: handle(other.handle) {
		other.handle = NULL;
	}

	library & operator=(library && other) noexcept {
		dlclose(handle);
		m_hModule = other.handle;
		other.handle = NULL;
		return *this;
	}

public:
	static std::optional<library> load(mpt::library::path path) {
		std::optional<mpt::native_path> optionalfilename = path.get_effective_filename();
		if (!optionalfilename) {
			return std::nullopt;
		}
#if defined(MPT_LIBCXX_QUIRK_NO_OPTIONAL_VALUE)
		mpt::native_path & filename = *optionalfilename;
#else
		mpt::native_path & filename = optionalfilename.value();
#endif
		if (filename.empty()) {
			return std::nullopt;
		}
		void * handle = dlopen(filename.AsNative().c_str(), RTLD_NOW);
		if (!handle) {
			return std::nullopt;
		}
		return library{handle};
	}

	auto get_address(const std::string & symbol) const {
		return dlsym(handle, symbol.c_str());
	}

	~library() {
		dlclose(handle);
	}

#elif defined(MPT_WITH_LTDL)

private:
	lt_dlhandle handle{};

	library(lt_dlhandle handle_)
		: handle(handle_) {
		return;
	}

public:
	library(const library &) = delete;
	library & operator=(const library &) = delete;

	library(library && other) noexcept
		: handle(other.handle) {
		other.handle = NULL;
	}

	library & operator=(library && other) noexcept {
		dlclose(handle);
		m_hModule = other.handle;
		other.handle = NULL;
		return *this;
	}

public:
	static std::optional<library> load(mpt::library::path path) {
		if (lt_dlinit() != 0) {
			return std::nullopt;
		}
		std::optional<mpt::native_path> optionalfilename = path.get_effective_filename();
		if (!optionalfilename) {
			return std::nullopt;
		}
#if defined(MPT_LIBCXX_QUIRK_NO_OPTIONAL_VALUE)
		mpt::native_path & filename = *optionalfilename;
#else
		mpt::native_path & filename = optionalfilename.value();
#endif
		if (filename.empty()) {
			return std::nullopt;
		}
		lt_dlhandle handle = lt_dlopenext(filename.AsNative().c_str());
		if (!handle) {
			return std::nullopt;
		}
		return library{handle};
	}

	auto get_address(const std::string & symbol) const {
		return lt_dlsym(handle, symbol.c_str());
	}

	~library() {
		lt_dlclose(handle);
		lt_dlexit();
	}

#else

private:
	void * handle = nullptr;

	library(void * handle_)
		: handle(handle_) {
		return;
	}

public:
	library(const library &) = delete;
	library & operator=(const library &) = delete;

	library(library && other) noexcept
		: handle(other.handle) {
		other.handle = nullptr;
	}

	library & operator=(library && other) noexcept {
		handle = other.handle;
		other.handle = nullptr;
		return *this;
	}

public:
	static std::optional<library> load(mpt::library::path path) {
		MPT_UNUSED(path);
		return std::nullopt;
	}

	void * get_address(const std::string & symbol) const {
		MPT_UNUSED(symbol);
		return nullptr;
	}

	~library() {
		return;
	}

#endif

	template <typename Tfunc>
	bool bind_function(Tfunc *& f, const std::string & symbol) const {
#if !defined(MPT_LIBCXX_QUIRK_INCOMPLETE_IS_FUNCTION)
		// MinGW64 std::is_function is always false for non __cdecl functions.
		// Issue is similar to <https://connect.microsoft.com/VisualStudio/feedback/details/774720/stl-is-function-bug>.
		static_assert(std::is_function<Tfunc>::value);
#endif
		auto sym_ptr = get_address(symbol);
		if (!sym_ptr) {
			return false;
		}
		if constexpr (std::is_same<decltype(sym_ptr), void *>::value) {
			f = reinterpret_cast<Tfunc *>(sym_ptr);
		} else {
			f = mpt::function_pointer_cast<Tfunc *>(sym_ptr);
		}
		return true;
	}

	template <typename Tdata>
	bool bind_data(Tdata *& d, const std::string & symbol) const {
#if !defined(MPT_LIBCXX_QUIRK_INCOMPLETE_IS_FUNCTION)
		// MinGW64 std::is_function is always false for non __cdecl functions.
		// Issue is similar to <https://connect.microsoft.com/VisualStudio/feedback/details/774720/stl-is-function-bug>.
		static_assert(!std::is_function<Tdata>::value);
#endif
		auto sym_ptr = get_address(symbol);
		if (!sym_ptr) {
			return false;
		}
		if constexpr (std::is_same<decltype(sym_ptr), void *>::value) {
			d = static_cast<Tdata *>(sym_ptr);
		} else {
			d = reinterpret_cast<Tdata *>(sym_ptr);
		}
		return true;
	}
};



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_LIBRARY_LIBRARY_HPP
