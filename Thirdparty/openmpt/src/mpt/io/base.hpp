/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_BASE_HPP
#define MPT_IO_BASE_HPP



#include "mpt/base/integer.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



using Offset = int64;

inline constexpr std::size_t BUFFERSIZE_MINUSCULE = 1 * 256; // on stack usage, tuned for single word/line buffers
inline constexpr std::size_t BUFFERSIZE_TINY = 1 * 1024;     // on stack usage
inline constexpr std::size_t BUFFERSIZE_SMALL = 4 * 1024;    // on heap
inline constexpr std::size_t BUFFERSIZE_NORMAL = 64 * 1024;  // FILE I/O
inline constexpr std::size_t BUFFERSIZE_LARGE = 1024 * 1024;



template <typename Tfile, typename Enable = void>
struct FileOperations {
};

template <typename Tfile>
inline FileOperations<Tfile> FileOps(Tfile & f) {
	return FileOperations<Tfile>{f};
}



template <typename Tfile>
inline bool IsValid(Tfile & f) {
	return FileOps(f).IsValid();
}

template <typename Tfile>
inline bool IsReadSeekable(Tfile & f) {
	return FileOps(f).IsReadSeekable();
}

template <typename Tfile>
inline bool IsWriteSeekable(Tfile & f) {
	return FileOps(f).IsWriteSeekable();
}

template <typename Tfile>
inline IO::Offset TellRead(Tfile & f) {
	return FileOps(f).TellRead();
}

template <typename Tfile>
inline IO::Offset TellWrite(Tfile & f) {
	return FileOps(f).TellWrite();
}

template <typename Tfile>
inline bool SeekBegin(Tfile & f) {
	return FileOps(f).SeekBegin();
}

template <typename Tfile>
inline bool SeekEnd(Tfile & f) {
	return FileOps(f).SeekEnd();
}

template <typename Tfile>
inline bool SeekAbsolute(Tfile & f, IO::Offset pos) {
	return FileOps(f).SeekAbsolute(pos);
}

template <typename Tfile>
inline bool SeekRelative(Tfile & f, IO::Offset off) {
	return FileOps(f).SeekRelative(off);
}

template <typename Tfile>
inline mpt::byte_span ReadRawImpl(Tfile & f, mpt::byte_span data) {
	return FileOps(f).ReadRawImpl(data);
}

template <typename Tfile>
inline bool WriteRawImpl(Tfile & f, mpt::const_byte_span data) {
	return FileOps(f).WriteRawImpl(data);
}

template <typename Tfile>
inline bool IsEof(Tfile & f) {
	return FileOps(f).IsEof();
}

template <typename Tfile>
inline bool Flush(Tfile & f) {
	return FileOps(f).Flush();
}



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_BASE_HPP
