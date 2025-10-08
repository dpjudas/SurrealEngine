/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_READ_FILEDATA_MEMORY_HPP
#define MPT_IO_READ_FILEDATA_MEMORY_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/io_read/filedata.hpp"

#include <algorithm>

#include <cstddef>
#if MPT_GCC_AT_LEAST(12, 0, 0)
#include <cstring>
#endif



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



class FileDataMemory
	: public IFileData {

private:
	const std::byte * streamData; // Pointer to memory-mapped file
	pos_type streamLength;        // Size of memory-mapped file in bytes

public:
	FileDataMemory()
		: streamData(nullptr)
		, streamLength(0) { }
	FileDataMemory(mpt::const_byte_span data)
		: streamData(data.data())
		, streamLength(data.size()) { }

public:
	bool IsValid() const override {
		return streamData != nullptr;
	}

	bool HasFastGetLength() const override {
		return true;
	}

	bool HasPinnedView() const override {
		return true;
	}

	const std::byte * GetRawData() const override {
		return streamData;
	}

	pos_type GetLength() const override {
		return streamLength;
	}

	mpt::byte_span Read(pos_type pos, mpt::byte_span dst) const override {
		if (pos >= streamLength) {
			return dst.first(0);
		}
		pos_type avail = std::min(streamLength - pos, static_cast<pos_type>(dst.size()));
		const std::byte * src = streamData + pos;
#if MPT_GCC_AT_LEAST(12, 0, 0)
		// work-around bogus -Warray-bounds
		// work-around bogus -Wstringop-overflow
		// See <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=121165>.
		std::memcpy(dst.data(), src, avail);
#else
		std::copy(src, src + avail, dst.data());
#endif
		return dst.first(static_cast<std::size_t>(avail));
	}

	bool CanRead(pos_type pos, pos_type length) const override {
		if ((pos == streamLength) && (length == 0)) {
			return true;
		}
		if (pos >= streamLength) {
			return false;
		}
		return (length <= streamLength - pos);
	}

	pos_type GetReadableLength(pos_type pos, pos_type length) const override {
		if (pos >= streamLength) {
			return 0;
		}
		return std::min(length, streamLength - pos);
	}
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_READ_FILEDATA_MEMORY_HPP
