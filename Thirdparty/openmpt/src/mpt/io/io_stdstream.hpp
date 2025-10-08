/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_IO_STDSTREAM_HPP
#define MPT_IO_IO_STDSTREAM_HPP



#include "mpt/base/macros.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/base/utility.hpp"
#include "mpt/io/base.hpp"

#include <ios>
#include <istream>
#include <ostream>
#include <type_traits>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



struct FileOperationsStdIos {

private:
	std::ios & f;

public:
	inline FileOperationsStdIos(std::ios & f_)
		: f(f_) {
		return;
	}

public:
	inline bool IsValid() {
		return !f.fail();
	}
};

struct FileOperationsStdIstream
	: public FileOperationsStdIos {

private:
	std::istream & f;

public:
	inline FileOperationsStdIstream(std::istream & f_)
		: FileOperationsStdIos(f_)
		, f(f_) {
		return;
	}

public:
	inline bool IsReadSeekable() {
		f.clear();
		std::streampos oldpos = f.tellg();
		if (f.fail() || oldpos == std::streampos(std::streamoff(-1))) {
			f.clear();
			return false;
		}
		f.seekg(0, std::ios::beg);
		if (f.fail()) {
			f.clear();
			f.seekg(oldpos);
			f.clear();
			return false;
		}
		f.seekg(0, std::ios::end);
		if (f.fail()) {
			f.clear();
			f.seekg(oldpos);
			f.clear();
			return false;
		}
		std::streampos length = f.tellg();
		if (f.fail() || length == std::streampos(std::streamoff(-1))) {
			f.clear();
			f.seekg(oldpos);
			f.clear();
			return false;
		}
		f.seekg(oldpos);
		f.clear();
		return true;
	}

	inline IO::Offset TellRead() {
		return static_cast<std::streamoff>(f.tellg());
	}

	inline bool SeekBegin() {
		f.seekg(0, std::ios::beg);
		return !f.fail();
	}

	inline bool SeekEnd() {
		f.seekg(0, std::ios::end);
		return !f.fail();
	}

	inline bool SeekAbsolute(IO::Offset pos) {
		if (!mpt::in_range<std::streamoff>(pos)) {
			return false;
		}
		f.seekg(static_cast<std::streamoff>(pos), std::ios::beg);
		return !f.fail();
	}

	inline bool SeekRelative(IO::Offset off) {
		if (!mpt::in_range<std::streamoff>(off)) {
			return false;
		}
		f.seekg(static_cast<std::streamoff>(off), std::ios::cur);
		return !f.fail();
	}

	inline mpt::byte_span ReadRawImpl(mpt::byte_span data) {
		std::size_t bytesToRead = data.size();
		std::size_t bytesRead = 0;
		while (bytesToRead > 0) {
			std::streamsize bytesChunkToRead = mpt::saturate_cast<std::streamsize>(bytesToRead);
			f.read(mpt::byte_cast<char *>(data.data()) + bytesRead, bytesChunkToRead);
			std::streamsize bytesChunkRead = f.gcount();
			bytesRead += static_cast<std::size_t>(bytesChunkRead);
			bytesToRead -= static_cast<std::size_t>(bytesChunkRead);
			if (bytesChunkRead != bytesChunkToRead) {
				break;
			}
		}
		return data.first(bytesRead);
	}

	inline bool IsEof() {
		return f.eof();
	}
};

struct FileOperationsStdOstream
	: public FileOperationsStdIos {

private:
	std::ostream & f;

public:
	inline FileOperationsStdOstream(std::ostream & f_)
		: FileOperationsStdIos(f_)
		, f(f_) {
		return;
	}

public:
	inline bool IsWriteSeekable() {
		f.clear();
		std::streampos oldpos = f.tellp();
		if (f.fail() || oldpos == std::streampos(std::streamoff(-1))) {
			f.clear();
			return false;
		}
		f.seekp(0, std::ios::beg);
		if (f.fail()) {
			f.clear();
			f.seekp(oldpos);
			f.clear();
			return false;
		}
		f.seekp(0, std::ios::end);
		if (f.fail()) {
			f.clear();
			f.seekp(oldpos);
			f.clear();
			return false;
		}
		std::streampos length = f.tellp();
		if (f.fail() || length == std::streampos(std::streamoff(-1))) {
			f.clear();
			f.seekp(oldpos);
			f.clear();
			return false;
		}
		f.seekp(oldpos);
		f.clear();
		return true;
	}

	inline IO::Offset TellWrite() {
		return static_cast<std::streamoff>(f.tellp());
	}

	inline bool SeekBegin() {
		f.seekp(0, std::ios::beg);
		return !f.fail();
	}

	inline bool SeekEnd() {
		f.seekp(0, std::ios::end);
		return !f.fail();
	}

	inline bool SeekAbsolute(IO::Offset pos) {
		if (!mpt::in_range<std::streamoff>(pos)) {
			return false;
		}
		f.seekp(static_cast<std::streamoff>(pos), std::ios::beg);
		return !f.fail();
	}

	inline bool SeekRelative(IO::Offset off) {
		if (!mpt::in_range<std::streamoff>(off)) {
			return false;
		}
		f.seekp(static_cast<std::streamoff>(off), std::ios::cur);
		return !f.fail();
	}

	inline bool WriteRawImpl(mpt::const_byte_span data) {
		std::size_t bytesToWrite = data.size();
		std::size_t bytesWritten = 0;
		while (bytesToWrite > 0) {
			std::streamsize bytesChunk = mpt::saturate_cast<std::streamsize>(bytesToWrite);
			f.write(mpt::byte_cast<const char *>(data.data()) + bytesWritten, bytesChunk);
			if (f.fail()) {
				break;
			}
			bytesWritten += static_cast<std::size_t>(bytesChunk);
			bytesToWrite -= static_cast<std::size_t>(bytesChunk);
		}
		return !f.fail();
	}

	inline bool Flush() {
		f.flush();
		return !f.fail();
	}
};

struct FileOperationsStdIOstream
	: public FileOperationsStdIstream
	, public FileOperationsStdOstream {

private:
	std::iostream & f;

public:
	inline FileOperationsStdIOstream(std::iostream & f_)
		: FileOperationsStdIstream(f_)
		, FileOperationsStdOstream(f_)
		, f(f_) {
		return;
	}

public:
	// cppcheck-suppress duplInheritedMember
	inline bool SeekBegin() {
		FileOperationsStdIstream::SeekBegin();
		FileOperationsStdOstream::SeekBegin();
		return !f.fail();
	}

	// cppcheck-suppress duplInheritedMember
	inline bool SeekEnd() {
		FileOperationsStdIstream::SeekEnd();
		FileOperationsStdOstream::SeekEnd();
		return !f.fail();
	}

	// cppcheck-suppress duplInheritedMember
	inline bool SeekAbsolute(IO::Offset pos) {
		if (!mpt::in_range<std::streamoff>(pos)) {
			return false;
		}
		FileOperationsStdIstream::SeekAbsolute(pos);
		FileOperationsStdOstream::SeekAbsolute(pos);
		return !f.fail();
	}

	// cppcheck-suppress duplInheritedMember
	inline bool SeekRelative(IO::Offset off) {
		if (!mpt::in_range<std::streamoff>(off)) {
			return false;
		}
		FileOperationsStdIstream::SeekRelative(off);
		FileOperationsStdOstream::SeekRelative(off);
		return !f.fail();
	}
};



template <typename Tstream>
struct FileOperations<Tstream, typename std::enable_if_t<std::is_base_of<std::iostream, Tstream>::value>>
	: public FileOperationsStdIOstream {
public:
	inline FileOperations(Tstream & f)
		: FileOperationsStdIOstream(f) {
		return;
	}
};



template <typename Tstream>
struct FileOperations<Tstream, typename std::enable_if_t<std::is_base_of<std::istream, Tstream>::value && !std::is_base_of<std::iostream, Tstream>::value>>
	: public FileOperationsStdIstream {
public:
	inline FileOperations(Tstream & f)
		: FileOperationsStdIstream(f) {
		return;
	}
};



template <typename Tstream>
struct FileOperations<Tstream, typename std::enable_if_t<std::is_base_of<std::ostream, Tstream>::value && !std::is_base_of<std::iostream, Tstream>::value>>
	: public FileOperationsStdOstream {
public:
	inline FileOperations(Tstream & f)
		: FileOperationsStdOstream(f) {
		return;
	}
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_IO_STDSTREAM_HPP
