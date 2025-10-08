/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_READ_FILEDATA_HPP
#define MPT_IO_READ_FILEDATA_HPP



#if !defined(MPT_CONFIGURATION_IO_READ_FILEDATA_NO_64BIT)
#include "mpt/base/integer.hpp"
#endif // !MPT_CONFIGURATION_IO_READ_FILEDATA_NO_64BIT
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"

#include <algorithm>

#if defined(MPT_CONFIGURATION_IO_READ_FILEDATA_NO_64BIT)
#include <cstddef>
#endif // MPT_CONFIGURATION_IO_READ_FILEDATA_NO_64BIT



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



class IFileData {
public:
#if !defined(MPT_CONFIGURATION_IO_READ_FILEDATA_NO_64BIT)
	using pos_type = uint64;
#else  // MPT_CONFIGURATION_IO_READ_FILEDATA_NO_64BIT
	using pos_type = std::size_t;
#endif // MPT_CONFIGURATION_IO_READ_FILEDATA_NO_64BIT

protected:
	IFileData() = default;

public:
	IFileData(const IFileData &) = default;
	IFileData & operator=(const IFileData &) = default;
	virtual ~IFileData() = default;

public:
	virtual bool IsValid() const = 0;
	virtual bool HasFastGetLength() const = 0;
	virtual bool HasPinnedView() const = 0;
	virtual const std::byte * GetRawData() const = 0;
	virtual pos_type GetLength() const = 0;
	virtual mpt::byte_span Read(pos_type pos, mpt::byte_span dst) const = 0;

	virtual bool CanRead(pos_type pos, pos_type length) const {
		pos_type dataLength = GetLength();
		if ((pos == dataLength) && (length == 0)) {
			return true;
		}
		if (pos >= dataLength) {
			return false;
		}
		return length <= dataLength - pos;
	}

	virtual pos_type GetReadableLength(pos_type pos, pos_type length) const {
		pos_type dataLength = GetLength();
		if (pos >= dataLength) {
			return 0;
		}
		return std::min(length, dataLength - pos);
	}
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_READ_FILEDATA_HPP
