/*
 * mptPathString.h
 * ---------------
 * Purpose: Wrapper class around the platform-native representation of path names. Should be the only type that is used to store path names.
 * Notes  : Currently none.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/path/basic_path.hpp"
#include "mpt/path/native_path.hpp"
#include "mpt/path/os_path.hpp"
#include "mpt/string/types.hpp"

#if defined(MODPLUG_TRACKER)
#include "mpt/string_transcode/transcode.hpp"
#endif

#include "mptString.h"

#if defined(MODPLUG_TRACKER) && MPT_OS_WINDOWS
#include <windows.h>
#endif



OPENMPT_NAMESPACE_BEGIN



namespace mpt
{



#if defined(MPT_ENABLE_CHARSET_LOCALE)

using PathString = mpt::native_path;

#define MPT_PATHSTRING_LITERAL(x) MPT_OS_PATH_LITERAL( x )
#define MPT_PATHSTRING(x) mpt::PathString::FromNative(MPT_OS_PATH_LITERAL( x ))

#else // !MPT_ENABLE_CHARSET_LOCALE

using PathString = mpt::BasicPathString<mpt::Utf8PathTraits, false>;

#define MPT_PATHSTRING_LITERAL(x) ( x )
#define MPT_PATHSTRING(x) mpt::PathString::FromNative( x )

#endif // MPT_ENABLE_CHARSET_LOCALE

using RawPathString = PathString::raw_path_type;

#define PC_(x) MPT_PATHSTRING_LITERAL(x)
#define PL_(x) MPT_PATHSTRING_LITERAL(x)
#define P_(x) MPT_PATHSTRING(x)



template <typename T, typename std::enable_if<std::is_same<T, mpt::PathString>::value, bool>::type = true>
inline mpt::ustring ToUString(const T &x)
{
	return x.ToUnicode();
}



#if defined(MODPLUG_TRACKER)



#if MPT_OS_WINDOWS



// Relative / absolute paths conversion

mpt::PathString AbsolutePathToRelative(const mpt::PathString &p, const mpt::PathString &relativeTo); // similar to std::fs::path::lexically_approximate
	
mpt::PathString RelativePathToAbsolute(const mpt::PathString &p, const mpt::PathString &relativeTo);



#if !MPT_OS_WINDOWS_WINRT
inline int PathCompareNoCase(const PathString &a, const PathString &b)
{
	return lstrcmpi(a.AsNative().c_str(), b.AsNative().c_str());
}
#endif // !MPT_OS_WINDOWS_WINRT



#endif // MPT_OS_WINDOWS



template <typename Tstring>
inline Tstring SanitizePathComponent(const Tstring &str)
{
	return mpt::transcode<Tstring>(mpt::native_path::FromNative(mpt::transcode<mpt::native_path::raw_path_type>(str)).AsSanitizedComponent().AsNative());
}



#endif // MODPLUG_TRACKER



} // namespace mpt



OPENMPT_NAMESPACE_END
