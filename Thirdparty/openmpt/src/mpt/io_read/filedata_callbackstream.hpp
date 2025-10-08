/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_READ_FILEDATA_CALLBACKSTREAM_HPP
#define MPT_IO_READ_FILEDATA_CALLBACKSTREAM_HPP



#include "mpt/base/integer.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/io_read/callbackstream.hpp"
#include "mpt/io_read/filedata.hpp"
#include "mpt/io_read/filedata_base_seekable.hpp"
#include "mpt/io_read/filedata_base_unseekable.hpp"

#include <algorithm>
#include <memory>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



template <typename Tstream>
class FileDataCallbackStreamTemplate {
public:
	static bool IsSeekable(CallbackStreamTemplate<Tstream> stream) {
		if (!stream.stream) {
			return false;
		}
		if (!stream.seek) {
			return false;
		}
		if (!stream.tell) {
			return false;
		}
		int64 oldpos = stream.tell(stream.stream);
		if (oldpos < 0) {
			return false;
		}
		if (stream.seek(stream.stream, 0, CallbackStream::SeekSet) < 0) {
			stream.seek(stream.stream, oldpos, CallbackStream::SeekSet);
			return false;
		}
		if (stream.seek(stream.stream, 0, CallbackStream::SeekEnd) < 0) {
			stream.seek(stream.stream, oldpos, CallbackStream::SeekSet);
			return false;
		}
		int64 length = stream.tell(stream.stream);
		if (length < 0) {
			stream.seek(stream.stream, oldpos, CallbackStream::SeekSet);
			return false;
		}
		stream.seek(stream.stream, oldpos, CallbackStream::SeekSet);
		return true;
	}

	static IFileData::pos_type GetLength(CallbackStreamTemplate<Tstream> stream) {
		if (!stream.stream) {
			return 0;
		}
		if (!stream.seek) {
			return false;
		}
		if (!stream.tell) {
			return false;
		}
		int64 oldpos = stream.tell(stream.stream);
		if (oldpos < 0) {
			return 0;
		}
		if (stream.seek(stream.stream, 0, CallbackStream::SeekSet) < 0) {
			stream.seek(stream.stream, oldpos, CallbackStream::SeekSet);
			return 0;
		}
		if (stream.seek(stream.stream, 0, CallbackStream::SeekEnd) < 0) {
			stream.seek(stream.stream, oldpos, CallbackStream::SeekSet);
			return 0;
		}
		int64 length = stream.tell(stream.stream);
		if (length < 0) {
			stream.seek(stream.stream, oldpos, CallbackStream::SeekSet);
			return 0;
		}
		stream.seek(stream.stream, oldpos, CallbackStream::SeekSet);
		return mpt::saturate_cast<IFileData::pos_type>(length);
	}
};

using FileDataCallbackStream = FileDataCallbackStreamTemplate<void *>;



template <typename Tstream>
class FileDataCallbackStreamSeekableTemplate : public FileDataSeekable {
private:
	CallbackStreamTemplate<Tstream> stream;

public:
	FileDataCallbackStreamSeekableTemplate(CallbackStreamTemplate<Tstream> s)
		: FileDataSeekable(FileDataCallbackStream::GetLength(s))
		, stream(s) {
		return;
	}

private:
	mpt::byte_span InternalReadSeekable(pos_type pos, mpt::byte_span dst) const override {
		if (!stream.read) {
			return dst.first(0);
		}
		if (stream.seek(stream.stream, pos, CallbackStream::SeekSet) < 0) {
			return dst.first(0);
		}
		int64 totalread = 0;
		std::byte * pdst = dst.data();
		std::size_t count = dst.size();
		while (count > 0) {
			int64 readcount = stream.read(stream.stream, pdst, count);
			if (readcount <= 0) {
				break;
			}
			pdst += static_cast<std::size_t>(readcount);
			count -= static_cast<std::size_t>(readcount);
			totalread += readcount;
		}
		return dst.first(static_cast<std::size_t>(totalread));
	}
};

using FileDataCallbackStreamSeekable = FileDataCallbackStreamSeekableTemplate<void *>;



template <typename Tstream>
class FileDataCallbackStreamUnseekableTemplate : public FileDataUnseekable {
private:
	CallbackStreamTemplate<Tstream> stream;
	mutable bool eof_reached;

public:
	FileDataCallbackStreamUnseekableTemplate(CallbackStreamTemplate<Tstream> s)
		: FileDataUnseekable()
		, stream(s)
		, eof_reached(false) {
		return;
	}

private:
	bool InternalEof() const override {
		return eof_reached;
	}

	mpt::byte_span InternalReadUnseekable(mpt::byte_span dst) const override {
		if (eof_reached) {
			return dst.first(0);
		}
		if (!stream.read) {
			eof_reached = true;
			return dst.first(0);
		}
		int64 totalread = 0;
		std::byte * pdst = dst.data();
		std::size_t count = dst.size();
		while (count > 0) {
			int64 readcount = stream.read(stream.stream, pdst, count);
			if (readcount <= 0) {
				eof_reached = true;
				break;
			}
			pdst += static_cast<std::size_t>(readcount);
			count -= static_cast<std::size_t>(readcount);
			totalread += readcount;
		}
		return dst.first(static_cast<std::size_t>(totalread));
	}
};

using FileDataCallbackStreamUnseekable = FileDataCallbackStreamUnseekableTemplate<void *>;



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_READ_FILEDATA_CALLBACKSTREAM_HPP
