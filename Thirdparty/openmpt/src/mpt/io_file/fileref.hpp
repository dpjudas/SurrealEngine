/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_FILE_FILEREF_HPP
#define MPT_IO_FILE_FILEREF_HPP



#include "mpt/base/alloc.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/base/span.hpp"
#include "mpt/io/base.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "mpt/io_file/fstream.hpp"
#include "mpt/path/os_path.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



// FileRef is a simple reference to an on-disk file by the means of a
// filename which allows easy assignment of the whole file contents to and from
// byte buffers.

class FileRef {
private:
	const mpt::os_path m_Filename;

public:
	FileRef(const mpt::os_path & filename)
		: m_Filename(filename) {
		return;
	}

public:
	FileRef & operator=(const std::vector<std::byte> & data) {
		mpt::IO::ofstream file(m_Filename, std::ios::binary);
		file.exceptions(std::ios_base::failbit | std::ios_base::badbit);
		mpt::IO::WriteRaw(file, mpt::as_span(data));
		mpt::IO::Flush(file);
		return *this;
	}

	FileRef & operator=(const std::vector<char> & data) {
		mpt::IO::ofstream file(m_Filename, std::ios::binary);
		file.exceptions(std::ios_base::failbit | std::ios_base::badbit);
		mpt::IO::WriteRaw(file, mpt::as_span(data));
		mpt::IO::Flush(file);
		return *this;
	}

	FileRef & operator=(const std::string & data) {
		mpt::IO::ofstream file(m_Filename, std::ios::binary);
		file.exceptions(std::ios_base::failbit | std::ios_base::badbit);
		mpt::IO::WriteRaw(file, mpt::as_span(data));
		mpt::IO::Flush(file);
		return *this;
	}

	operator std::vector<std::byte>() const {
		mpt::IO::ifstream file(m_Filename, std::ios::binary);
		if (!mpt::IO::IsValid(file)) {
			return std::vector<std::byte>();
		}
		file.exceptions(std::ios_base::failbit | std::ios_base::badbit);
		mpt::IO::SeekEnd(file);
		std::vector<std::byte> buf(mpt::saturate_cast<std::size_t>(mpt::IO::TellRead(file)));
		mpt::IO::SeekBegin(file);
		mpt::IO::ReadRaw(file, mpt::as_span(buf));
		return buf;
	}

	operator std::vector<char>() const {
		mpt::IO::ifstream file(m_Filename, std::ios::binary);
		if (!mpt::IO::IsValid(file)) {
			return std::vector<char>();
		}
		file.exceptions(std::ios_base::failbit | std::ios_base::badbit);
		mpt::IO::SeekEnd(file);
		std::vector<char> buf(mpt::saturate_cast<std::size_t>(mpt::IO::TellRead(file)));
		mpt::IO::SeekBegin(file);
		mpt::IO::ReadRaw(file, mpt::as_span(buf));
		return buf;
	}

	operator std::string() const {
		mpt::IO::ifstream file(m_Filename, std::ios::binary);
		if (!mpt::IO::IsValid(file)) {
			return std::string();
		}
		file.exceptions(std::ios_base::failbit | std::ios_base::badbit);
		mpt::IO::SeekEnd(file);
		std::vector<char> buf(mpt::saturate_cast<std::size_t>(mpt::IO::TellRead(file)));
		mpt::IO::SeekBegin(file);
		mpt::IO::ReadRaw(file, mpt::as_span(buf));
		return mpt::buffer_cast<std::string>(buf);
	}
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_FILE_FILEREF_HPP
