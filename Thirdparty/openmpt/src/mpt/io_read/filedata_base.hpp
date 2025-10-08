/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_READ_FILEDATA_BASE_HPP
#define MPT_IO_READ_FILEDATA_BASE_HPP



#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/io_read/filedata.hpp"

#include <algorithm>
#include <memory>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



class FileDataDummy : public IFileData {
public:
	FileDataDummy() { }

public:
	bool IsValid() const override {
		return false;
	}

	bool HasFastGetLength() const override {
		return true;
	}

	bool HasPinnedView() const override {
		return true;
	}

	const std::byte * GetRawData() const override {
		return nullptr;
	}

	pos_type GetLength() const override {
		return 0;
	}
	mpt::byte_span Read(pos_type /* pos */, mpt::byte_span dst) const override {
		return dst.first(0);
	}
};


class FileDataWindow : public IFileData {
private:
	std::shared_ptr<const IFileData> data;
	const pos_type dataOffset;
	const pos_type dataLength;

public:
	FileDataWindow(std::shared_ptr<const IFileData> src, pos_type off, pos_type len)
		: data(src)
		, dataOffset(off)
		, dataLength(len) { }

	bool IsValid() const override {
		return data->IsValid();
	}
	bool HasFastGetLength() const override {
		return true;
	}
	bool HasPinnedView() const override {
		return data->HasPinnedView();
	}
	const std::byte * GetRawData() const override {
		return data->GetRawData() + dataOffset;
	}
	pos_type GetLength() const override {
		return dataLength;
	}
	mpt::byte_span Read(pos_type pos, mpt::byte_span dst) const override {
		if (pos >= dataLength) {
			return dst.first(0);
		}
		return data->Read(dataOffset + pos, dst.first(static_cast<std::size_t>(std::min(static_cast<pos_type>(dst.size()), dataLength - pos))));
	}
	bool CanRead(pos_type pos, pos_type length) const override {
		if ((pos == dataLength) && (length == 0)) {
			return true;
		}
		if (pos >= dataLength) {
			return false;
		}
		return (length <= dataLength - pos);
	}
	pos_type GetReadableLength(pos_type pos, pos_type length) const override {
		if (pos >= dataLength) {
			return 0;
		}
		return std::min(length, dataLength - pos);
	}
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_READ_FILEDATA_BASE_HPP
