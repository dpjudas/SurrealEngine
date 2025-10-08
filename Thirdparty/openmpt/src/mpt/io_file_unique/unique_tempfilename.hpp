/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_FILE_UNIQUE_UNIQUE_TEMPFILENAME_HPP
#define MPT_IO_FILE_UNIQUE_UNIQUE_TEMPFILENAME_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/io_file_unique/unique_basename.hpp"
#include "mpt/fs/common_directories.hpp"
#include "mpt/fs/fs.hpp"
#include "mpt/path/native_path.hpp"
#include "mpt/path/os_path.hpp"



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



class unique_tempfilename {

private:
	mpt::os_path m_Filename;

public:
	unique_tempfilename(const unique_basename & basename, const mpt::os_path & extension = MPT_OS_PATH("tmp"))
		: m_Filename(mpt::common_directories::get_temp_directory().WithTrailingSlash() + static_cast<mpt::os_path>(basename) + (extension.empty() ? extension : MPT_OS_PATH(".") + extension)) {
		return;
	}

public:
	operator mpt::os_path() const {
		return m_Filename;
	}

}; // class unique_tempfilename



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_FILE_UNIQUE_UNIQUE_TEMPFILENAME_HPP
