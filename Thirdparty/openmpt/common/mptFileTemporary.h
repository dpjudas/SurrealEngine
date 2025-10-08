/*
 * mptFileTemporary.h
 * ------------------
 * Purpose:
 * Notes  : Currently none.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/namespace.hpp"

#if defined(MODPLUG_TRACKER) && MPT_OS_WINDOWS

#include "mpt/fs/common_directories.hpp"
#include "mpt/fs/fs.hpp"
#include "mpt/io_file_unique/unique_basename.hpp"
#include "mpt/io_file_unique/unique_tempfilename.hpp"
#include "mpt/uuid/uuid.hpp"

#include "mptPathString.h"
#include "mptRandom.h"

#if MPT_OS_WINDOWS
#include <windows.h>
#endif

#endif // MODPLUG_TRACKER && MPT_OS_WINDOWS



OPENMPT_NAMESPACE_BEGIN



namespace mpt
{



#if defined(MODPLUG_TRACKER) && MPT_OS_WINDOWS



// Returns a new unique absolute path.
class TemporaryPathname
{
private:
	mpt::PathString m_Path;
public:
	TemporaryPathname(const mpt::PathString &fileNameExtension = P_("tmp"))
	{
		mpt::PathString prefix;
#if defined(LIBOPENMPT_BUILD)
		prefix = P_("libopenmpt");
#else
		prefix = P_("OpenMPT");
#endif
		m_Path = mpt::PathString::FromNative(mpt::IO::unique_tempfilename{mpt::IO::unique_basename{prefix, mpt::UUID::GenerateLocalUseOnly(mpt::global_prng())}, fileNameExtension});
	}
public:
	mpt::PathString GetPathname() const
	{
		return m_Path;
	}
};



// Scoped temporary file guard. Deletes the file when going out of scope.
// The file itself is not created automatically.
class TempFileGuard
{
private:
	const mpt::PathString filename;
public:
	TempFileGuard(const mpt::TemporaryPathname &pathname = mpt::TemporaryPathname{})
		: filename(pathname.GetPathname())
	{
		return;
	}
	mpt::PathString GetFilename() const
	{
		return filename;
	}
	~TempFileGuard()
	{
		if(!filename.empty())
		{
			DeleteFile(mpt::support_long_path(filename.AsNative()).c_str());
		}
	}
};



// Scoped temporary directory guard. Deletes the directory when going out of scope.
// The directory itself is created automatically.
class TempDirGuard
{
private:
	mpt::PathString dirname;
public:
	TempDirGuard(const mpt::TemporaryPathname &pathname = mpt::TemporaryPathname{})
		: dirname(pathname.GetPathname().WithTrailingSlash())
	{
		if(dirname.empty())
		{
			return;
		}
		if(::CreateDirectory(mpt::support_long_path(dirname.AsNative()).c_str(), NULL) == 0)
		{ // fail
			dirname = mpt::PathString();
		}
	}
	mpt::PathString GetDirname() const
	{
		return dirname;
	}
	~TempDirGuard()
	{
		if(!dirname.empty())
		{
			mpt::native_fs{}.delete_tree(dirname);
		}
	}
};



#endif // MODPLUG_TRACKER && MPT_OS_WINDOWS



} // namespace mpt



OPENMPT_NAMESPACE_END
