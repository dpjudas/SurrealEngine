/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_PATH_NATIVE_PATH_HPP
#define MPT_PATH_NATIVE_PATH_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/path/basic_path.hpp"
#include "mpt/path/os_path.hpp"
#include "mpt/string/types.hpp"



namespace mpt {
inline namespace MPT_INLINE_NS {



struct NativePathStyleTag {
#if MPT_OS_WINDOWS_WINNT
	static inline constexpr mpt::PathStyle path_style = mpt::PathStyle::WindowsNT;
#elif MPT_OS_WINDOWS_WIN9X
	static inline constexpr mpt::PathStyle path_style = mpt::PathStyle::Windows9x;
#elif MPT_OS_WINDOWS
	static inline constexpr mpt::PathStyle path_style = mpt::PathStyle::Windows9x;
#elif MPT_OS_DJGPP
	static inline constexpr mpt::PathStyle path_style = mpt::PathStyle::DOS_DJGPP;
#else
	static inline constexpr mpt::PathStyle path_style = mpt::PathStyle::Posix;
#endif
};

struct NativePathTraits : public PathTraits<mpt::os_path, NativePathStyleTag> {
};

struct Utf8PathTraits : public PathTraits<mpt::utf8string, NativePathStyleTag> {
};

using native_path = BasicPathString<NativePathTraits, true>;

#define MPT_NATIVE_PATH(x) \
	mpt::native_path { \
		MPT_OS_PATH(x) \
	}



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_PATH_NATIVE_PATH_HPP
