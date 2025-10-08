/*
 * mptPathString.cpp
 * -----------------
 * Purpose: Wrapper class around the platform-native representation of path names. Should be the only type that is used to store path names.
 * Notes  : Currently none.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "stdafx.h"
#include "mptPathString.h"

#include <vector>

#if defined(MODPLUG_TRACKER) && MPT_OS_WINDOWS
#include <tchar.h>
#endif

OPENMPT_NAMESPACE_BEGIN



namespace mpt
{



#if defined(MODPLUG_TRACKER) && MPT_OS_WINDOWS



// Convert an absolute path to a path that's relative to "&relativeTo".
mpt::PathString AbsolutePathToRelative(const mpt::PathString &path, const mpt::PathString &relativeTo)
{
	using namespace path_literals;
	using char_type = RawPathString::value_type;
	mpt::PathString result = path;
	if(path.empty() || relativeTo.empty())
	{
		return result;
	}
	if(!_tcsncicmp(relativeTo.AsNative().c_str(), path.AsNative().c_str(), relativeTo.AsNative().length()))
	{
		// Path is OpenMPT's directory or a sub directory ("C:\OpenMPT\Somepath" => ".\Somepath")
		result = mpt::PathString::FromNative(L<char_type>(".\\")); // ".\"
		result += mpt::PathString::FromNative(path.AsNative().substr(relativeTo.AsNative().length()));
	} else if(!_tcsncicmp(relativeTo.AsNative().c_str(), path.AsNative().c_str(), 2))
	{
		// Path is on the same drive as OpenMPT ("C:\Somepath" => "\Somepath")
		result = mpt::PathString::FromNative(path.AsNative().substr(2));
	}
	return result;
}


// Convert a path that is relative to "&relativeTo" to an absolute path.
mpt::PathString RelativePathToAbsolute(const mpt::PathString &path, const mpt::PathString &relativeTo)
{
	using namespace path_literals;
	using char_type = RawPathString::value_type;
	mpt::PathString result = path;
	if(path.empty() || relativeTo.empty())
	{
		return result;
	}
	if(path.length() >= 2 && path.AsNative()[0] == L<char_type>('\\') && path.AsNative()[1] == L<char_type>('\\'))
	{
		// Network / UNC paths
		return result;
	} if(path.length() >= 1 && path.AsNative()[0] == L<char_type>('\\'))
	{
		// Path is on the same drive as relativeTo ("\Somepath\" => "C:\Somepath\")
		result = mpt::PathString::FromNative(relativeTo.AsNative().substr(0, 2));
		result += mpt::PathString(path);
	} else if(path.length() >= 2 && path.AsNative().substr(0, 2) == L<char_type>(".\\"))
	{
		// Path is in relativeTo or a sub directory (".\Somepath\" => "C:\OpenMPT\Somepath\")
		result = relativeTo; // "C:\OpenMPT\"
		result += mpt::PathString::FromNative(path.AsNative().substr(2));
	} else if(path.length() < 3 || path.AsNative()[1] != L<char_type>(':') || path.AsNative()[2] != L<char_type>('\\'))
	{
		// Any other path not starting with drive letter
		result = relativeTo;  // "C:\OpenMPT\"
		result += mpt::PathString(path);
	}
	return result;
}



#endif // MODPLUG_TRACKER && MPT_OS_WINDOWS



} // namespace mpt



OPENMPT_NAMESPACE_END
