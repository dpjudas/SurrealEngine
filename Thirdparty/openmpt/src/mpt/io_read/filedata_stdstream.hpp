/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_READ_FILEDATA_STDSTREAM_HPP
#define MPT_IO_READ_FILEDATA_STDSTREAM_HPP



#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/io/base.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "mpt/io_read/filedata.hpp"
#include "mpt/io_read/filedata_base_buffered.hpp"
#include "mpt/io_read/filedata_base_unseekable.hpp"

#include <ios>
#include <istream>
#include <ostream>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



class FileDataStdStream {

public:
	static bool IsSeekable(std::istream & stream) {
		return mpt::IO::IsReadSeekable(stream);
	}

	static IFileData::pos_type GetLength(std::istream & stream) {
		stream.clear();
		std::streampos oldpos = stream.tellg();
		stream.seekg(0, std::ios::end);
		std::streamoff length = static_cast<std::streamoff>(stream.tellg());
		stream.seekg(oldpos);
		return mpt::saturate_cast<IFileData::pos_type>(static_cast<int64>(length));
	}
};



class FileDataStdStreamSeekable : public FileDataSeekableBuffered {

private:
	std::istream & stream;

public:
	FileDataStdStreamSeekable(std::istream & s)
		: FileDataSeekableBuffered(FileDataStdStream::GetLength(s))
		, stream(s) {
		return;
	}

private:
	mpt::byte_span InternalReadBuffered(pos_type pos, mpt::byte_span dst) const override {
		stream.clear(); // tellg needs eof and fail bits unset
		std::streampos currentpos = stream.tellg();
		if ((currentpos == std::streampos(std::streamoff(-1))) || (static_cast<std::streamoff>(pos) != static_cast<std::streamoff>(currentpos))) {
			// inefficient istream implementations might invalidate their buffer when seeking, even when seeking to the current position
			stream.seekg(static_cast<std::streamoff>(pos), std::ios::beg);
		}
		std::size_t bytesToRead = dst.size();
		std::size_t bytesRead = 0;
		while (bytesToRead > 0) {
			std::streamsize bytesChunkToRead = mpt::saturate_cast<std::streamsize>(bytesToRead);
			stream.read(mpt::byte_cast<char *>(dst.data()) + bytesRead, bytesChunkToRead);
			std::streamsize bytesChunkRead = stream.gcount();
			bytesRead += static_cast<std::size_t>(bytesChunkRead);
			bytesToRead -= static_cast<std::size_t>(bytesChunkRead);
			if (bytesChunkRead != bytesChunkToRead) {
				break;
			}
		}
		return dst.first(bytesRead);
	}
};



class FileDataStdStreamUnseekable : public FileDataUnseekable {

private:
	std::istream & stream;

public:
	FileDataStdStreamUnseekable(std::istream & s)
		: stream(s) {
		return;
	}

private:
	bool InternalEof() const override {
		if (stream) {
			return false;
		} else {
			return true;
		}
	}

	mpt::byte_span InternalReadUnseekable(mpt::byte_span dst) const override {
		std::size_t bytesToRead = dst.size();
		std::size_t bytesRead = 0;
		while (bytesToRead > 0) {
			std::streamsize bytesChunkToRead = mpt::saturate_cast<std::streamsize>(bytesToRead);
			stream.read(mpt::byte_cast<char *>(dst.data()) + bytesRead, bytesChunkToRead);
			std::streamsize bytesChunkRead = stream.gcount();
			bytesRead += static_cast<std::size_t>(bytesChunkRead);
			bytesToRead -= static_cast<std::size_t>(bytesChunkRead);
			if (bytesChunkRead != bytesChunkToRead) {
				break;
			}
		}
		return dst.first(bytesRead);
	}
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_READ_FILEDATA_STDSTREAM_HPP
