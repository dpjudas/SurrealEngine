/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_READ_FILEDATA_BASE_SEEKABLE_HPP
#define MPT_IO_READ_FILEDATA_BASE_SEEKABLE_HPP



#include "mpt/base/alloc.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/span.hpp"
#include "mpt/io_read/filedata.hpp"

#include <algorithm>
#include <vector>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



class FileDataSeekable : public IFileData {

private:
	pos_type streamLength;

protected:
	FileDataSeekable(pos_type streamLength_)
		: streamLength(streamLength_) {
		return;
	}

public:
	bool IsValid() const override {
		return true;
	}

	bool HasFastGetLength() const override {
		return true;
	}

	bool HasPinnedView() const override {
		return false;
	}

	const std::byte * GetRawData() const override {
		return nullptr;
	}

	pos_type GetLength() const override {
		return streamLength;
	}

	mpt::byte_span Read(pos_type pos, mpt::byte_span dst) const override {
		return InternalReadSeekable(pos, dst);
	}

private:
	virtual mpt::byte_span InternalReadSeekable(pos_type pos, mpt::byte_span dst) const = 0;
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_READ_FILEDATA_BASE_SEEKABLE_HPP
