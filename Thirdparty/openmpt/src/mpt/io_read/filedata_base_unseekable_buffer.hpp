/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_READ_FILEDATA_BASE_UNSEEKABLE_BUFFER_HPP
#define MPT_IO_READ_FILEDATA_BASE_UNSEEKABLE_BUFFER_HPP



#include "mpt/base/algorithm.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/numeric.hpp"
#include "mpt/io/base.hpp"
#include "mpt/io_read/filedata.hpp"

#include <algorithm>
#include <limits>
#include <vector>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



class FileDataUnseekableBuffer : public IFileData {

private:
	mutable std::vector<std::byte> cache;
	mutable bool streamFullyCached;

protected:
	FileDataUnseekableBuffer()
		: streamFullyCached(false) {
		return;
	}

private:
	enum : std::size_t {
		QUANTUM_SIZE = mpt::IO::BUFFERSIZE_SMALL,
		BUFFER_SIZE = mpt::IO::BUFFERSIZE_NORMAL
	};

	void CacheStream() const {
		if (streamFullyCached) {
			return;
		}
		while (!InternalEof()) {
			InternalReadContinue(cache, BUFFER_SIZE);
		}
		streamFullyCached = true;
	}

	void CacheStreamUpTo(pos_type pos, pos_type length) const {
		if (streamFullyCached) {
			return;
		}
		if (length > std::numeric_limits<pos_type>::max() - pos) {
			length = std::numeric_limits<pos_type>::max() - pos;
		}
		std::size_t target = mpt::saturate_cast<std::size_t>(pos + length);
		if (target <= cache.size()) {
			return;
		}
		while (!InternalEof() && (cache.size() < target)) {
			std::size_t readsize = ((target - cache.size()) > QUANTUM_SIZE) ? BUFFER_SIZE : QUANTUM_SIZE;
			InternalReadContinue(cache, readsize);
		}
	}

private:
	void ReadCached(pos_type pos, mpt::byte_span dst) const {
		std::copy(cache.begin() + static_cast<std::size_t>(pos), cache.begin() + static_cast<std::size_t>(pos + dst.size()), dst.data());
	}

public:
	bool IsValid() const override {
		return true;
	}

	bool HasFastGetLength() const override {
		return false;
	}

	bool HasPinnedView() const override {
		return true; // we have the cache which is required for seeking anyway
	}

	const std::byte * GetRawData() const override {
		CacheStream();
		return cache.data();
	}

	pos_type GetLength() const override {
		CacheStream();
		return cache.size();
	}

	mpt::byte_span Read(pos_type pos, mpt::byte_span dst) const override {
		CacheStreamUpTo(pos, dst.size());
		if (pos >= static_cast<pos_type>(cache.size())) {
			return dst.first(0);
		}
		pos_type cache_avail = std::min(static_cast<pos_type>(cache.size()) - pos, static_cast<pos_type>(dst.size()));
		ReadCached(pos, dst.subspan(0, static_cast<std::size_t>(cache_avail)));
		return dst.subspan(0, static_cast<std::size_t>(cache_avail));
	}

	bool CanRead(pos_type pos, pos_type length) const override {
		CacheStreamUpTo(pos, length);
		if ((pos == static_cast<pos_type>(cache.size())) && (length == 0)) {
			return true;
		}
		if (pos >= static_cast<pos_type>(cache.size())) {
			return false;
		}
		return length <= static_cast<pos_type>(cache.size()) - pos;
	}

	pos_type GetReadableLength(pos_type pos, pos_type length) const override {
		CacheStreamUpTo(pos, length);
		if (pos >= cache.size()) {
			return 0;
		}
		return std::min(static_cast<pos_type>(cache.size()) - pos, length);
	}

private:
	virtual bool InternalEof() const = 0;
	virtual void InternalReadContinue(std::vector<std::byte> & streamCache, std::size_t suggestedCount) const = 0;
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_READ_FILEDATA_BASE_UNSEEKABLE_BUFFER_HPP
