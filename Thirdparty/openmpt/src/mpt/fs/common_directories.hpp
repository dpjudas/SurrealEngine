/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_FS_COMMON_DIRECTORIES_HPP
#define MPT_FS_COMMON_DIRECTORIES_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#if MPT_OS_WINDOWS
#include "mpt/base/saturate_cast.hpp"
#endif // MPT_OS_WINDOWS
#include "mpt/fs/fs.hpp"
#include "mpt/path/native_path.hpp"

#if MPT_OS_WINDOWS
#include <vector>
#endif // MPT_OS_WINDOWS

#if MPT_OS_WINDOWS
#include <windows.h>
#endif // MPT_OS_WINDOWS



namespace mpt {
inline namespace MPT_INLINE_NS {



#if MPT_OS_WINDOWS



class common_directories {

public:

	static inline mpt::native_path get_application_directory() {
		std::vector<TCHAR> exeFileName(MAX_PATH);
		while (::GetModuleFileName(0, exeFileName.data(), mpt::saturate_cast<DWORD>(exeFileName.size())) >= exeFileName.size()) {
#if MPT_WIN_AT_LEAST(MPT_WIN_VISTA)
			if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
				return mpt::native_path();
			}
#else
			// Windows XP and earlier return ERROR_SUCCESS even when the string does not fit.
			// See <https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamea>.
			DWORD errc = ::GetLastError();
			if ((errc != ERROR_INSUFFICIENT_BUFFER) && (errc != ERROR_SUCCESS)) {
				return mpt::native_path();
			}
#endif
			exeFileName.resize(exeFileName.size() * 2);
		}
		return mpt::native_fs{}.absolute(mpt::native_path::FromNative(exeFileName.data()).GetDirectoryWithDrive());
	}

#if !MPT_OS_WINDOWS_WINRT
	static inline mpt::native_path get_system_directory() {
		DWORD size = ::GetSystemDirectory(nullptr, 0);
		std::vector<TCHAR> path(size + 1);
		if (!::GetSystemDirectory(path.data(), size + 1)) {
			return mpt::native_path();
		}
		return mpt::native_path::FromNative(path.data()) + MPT_NATIVE_PATH("\\");
	}
#endif // !MPT_OS_WINDOWS_WINRT

	static inline mpt::native_path get_temp_directory() {
		DWORD size = ::GetTempPath(0, nullptr);
		if (size) {
			std::vector<TCHAR> tempPath(size + 1);
			if (::GetTempPath(size + 1, tempPath.data())) {
				return mpt::native_path::FromNative(tempPath.data());
			}
		}
		// use app directory as fallback
		return get_application_directory();
	}

}; // class common_directories



#endif // MPT_OS_WINDOWS



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_FS_COMMON_DIRECTORIES_HPP
