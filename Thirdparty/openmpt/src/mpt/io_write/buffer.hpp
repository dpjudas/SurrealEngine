/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_WRITE_BUFFER_HPP
#define MPT_IO_WRITE_BUFFER_HPP



#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/span.hpp"
#include "mpt/io/base.hpp"

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



// WriteBuffer class that avoids calling to the underlying file writing
// functions for every operation, which would involve rather slow un-inlinable
// virtual calls in the iostream and FILE* cases. It is the users responabiliy
// to call HasWriteError() to check for writeback errors at this buffering
// level.

template <typename Tfile>
class WriteBuffer {
private:
	mpt::byte_span buffer;
	std::size_t size = 0;
	Tfile & f;
	bool writeError = false;

public:
	WriteBuffer(const WriteBuffer &) = delete;
	WriteBuffer & operator=(const WriteBuffer &) = delete;

public:
	inline WriteBuffer(Tfile & f_, mpt::byte_span buffer_)
		: buffer(buffer_)
		, f(f_) {
		return;
	}
	inline ~WriteBuffer() noexcept(false) {
		if (!writeError) {
			FlushLocal();
		}
	}

public:
	inline Tfile & file() const {
		if (IsDirty()) {
			FlushLocal();
		}
		return f;
	}

public:
	inline bool HasWriteError() const {
		return writeError;
	}
	inline void ClearError() {
		writeError = false;
	}
	inline bool IsDirty() const {
		return size > 0;
	}
	inline bool IsClean() const {
		return size == 0;
	}
	inline bool IsFull() const {
		return size == buffer.size();
	}
	inline std::size_t GetCurrentSize() const {
		return size;
	}
	inline bool Write(mpt::const_byte_span data) {
		bool result = true;
		for (std::size_t i = 0; i < data.size(); ++i) {
			buffer[size] = data[i];
			size++;
			if (IsFull()) {
				FlushLocal();
			}
		}
		return result;
	}
	inline void FlushLocal() {
		if (IsClean()) {
			return;
		}
		try {
			if (!mpt::IO::WriteRaw(f, mpt::as_span(buffer.data(), size))) {
				writeError = true;
			}
		} catch (const std::exception &) {
			writeError = true;
			throw;
		}
		size = 0;
	}
};

template <typename Tfile>
struct FileOperations<WriteBuffer<Tfile>> {

private:
	WriteBuffer<Tfile> & f;

public:
	FileOperations(WriteBuffer<Tfile> & f_)
		: f(f_) {
		return;
	}

public:
	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool IsValid() {
		return IsValid(f.file());
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool IsReadSeekable() {
		return IsReadSeekable(f.file());
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool IsWriteSeekable() {
		return IsWriteSeekable(f.file());
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline IO::Offset TellRead() {
		f.FlushLocal();
		return TellRead(f.file());
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline IO::Offset TellWrite() {
		return TellWrite(f.file()) + f.GetCurrentSize();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekBegin() {
		f.FlushLocal();
		return SeekBegin(f.file());
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekEnd() {
		f.FlushLocal();
		return SeekEnd(f.file());
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekAbsolute(IO::Offset pos) {
		f.FlushLocal();
		return SeekAbsolute(f.file(), pos);
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekRelative(IO::Offset off) {
		f.FlushLocal();
		return SeekRelative(f.file(), off);
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline mpt::byte_span ReadRawImpl(mpt::byte_span data) {
		f.FlushLocal();
		return ReadRawImpl(f.file(), data);
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool WriteRawImpl(mpt::const_byte_span data) {
		return f.Write(data);
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool IsEof() {
		f.FlushLocal();
		return IsEof(f.file());
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool Flush() {
		f.FlushLocal();
		return Flush(f.file());
	}
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_WRITE_BUFFER_HPP
