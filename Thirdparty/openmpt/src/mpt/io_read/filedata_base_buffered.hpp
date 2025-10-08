/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_READ_FILEDATA_BASE_BUFFERED_HPP
#define MPT_IO_READ_FILEDATA_BASE_BUFFERED_HPP



#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/numeric.hpp"
#include "mpt/io/base.hpp"
#include "mpt/io_read/filedata.hpp"
#include "mpt/io_read/filedata_base_seekable.hpp"

#include <algorithm>
#include <vector>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



class FileDataSeekableBuffered : public FileDataSeekable {

private:
	enum : std::size_t {
		CHUNK_SIZE = mpt::IO::BUFFERSIZE_SMALL,
		BUFFER_SIZE = mpt::IO::BUFFERSIZE_NORMAL
	};
	enum : std::size_t {
		NUM_CHUNKS = BUFFER_SIZE / CHUNK_SIZE
	};
	struct chunk_info {
		pos_type ChunkOffset = 0;
		std::size_t ChunkLength = 0;
		bool ChunkValid = false;
	};
	mutable std::vector<std::byte> m_Buffer = std::vector<std::byte>(BUFFER_SIZE);

	mpt::byte_span chunk_data(std::size_t chunkIndex) const {
		return mpt::byte_span(m_Buffer.data() + (chunkIndex * CHUNK_SIZE), CHUNK_SIZE);
	}

	mutable std::array<chunk_info, NUM_CHUNKS> m_ChunkInfo = {};
	mutable std::array<std::size_t, NUM_CHUNKS> m_ChunkIndexLRU = {};

	std::size_t InternalFillPageAndReturnIndex(pos_type pos) const {
		pos = mpt::align_down(pos, static_cast<pos_type>(CHUNK_SIZE));
		for (std::size_t chunkLRUIndex = 0; chunkLRUIndex < NUM_CHUNKS; ++chunkLRUIndex) {
			std::size_t chunkIndex = m_ChunkIndexLRU[chunkLRUIndex];
			if (m_ChunkInfo[chunkIndex].ChunkValid && (m_ChunkInfo[chunkIndex].ChunkOffset == pos)) {
				std::size_t chunk = std::move(m_ChunkIndexLRU[chunkLRUIndex]);
				std::move_backward(m_ChunkIndexLRU.begin(), m_ChunkIndexLRU.begin() + chunkLRUIndex, m_ChunkIndexLRU.begin() + (chunkLRUIndex + 1));
				m_ChunkIndexLRU[0] = std::move(chunk);
				return chunkIndex;
			}
		}
		{
			std::size_t chunk = std::move(m_ChunkIndexLRU[NUM_CHUNKS - 1]);
			std::move_backward(m_ChunkIndexLRU.begin(), m_ChunkIndexLRU.begin() + (NUM_CHUNKS - 1), m_ChunkIndexLRU.begin() + NUM_CHUNKS);
			m_ChunkIndexLRU[0] = std::move(chunk);
		}
		std::size_t chunkIndex = m_ChunkIndexLRU[0];
		chunk_info & chunk = m_ChunkInfo[chunkIndex];
		chunk.ChunkOffset = pos;
		chunk.ChunkLength = InternalReadBuffered(pos, chunk_data(chunkIndex)).size();
		chunk.ChunkValid = true;
		return chunkIndex;
	}

protected:
	FileDataSeekableBuffered(pos_type streamLength_)
		: FileDataSeekable(streamLength_) {
		return;
	}

private:
	mpt::byte_span InternalReadSeekable(pos_type pos, mpt::byte_span dst) const override {
		pos_type totalRead = 0;
		std::byte * pdst = dst.data();
		std::size_t count = dst.size();
		while (count > 0) {
			std::size_t chunkIndex = InternalFillPageAndReturnIndex(pos);
			pos_type pageSkip = pos - m_ChunkInfo[chunkIndex].ChunkOffset;
			pos_type chunkWanted = std::min(static_cast<pos_type>(CHUNK_SIZE) - pageSkip, static_cast<pos_type>(count));
			pos_type chunkGot = (static_cast<pos_type>(m_ChunkInfo[chunkIndex].ChunkLength) > pageSkip) ? (static_cast<pos_type>(m_ChunkInfo[chunkIndex].ChunkLength) - pageSkip) : 0;
			pos_type chunk = std::min(chunkWanted, chunkGot);
			std::copy(chunk_data(chunkIndex).data() + static_cast<std::size_t>(pageSkip), chunk_data(chunkIndex).data() + static_cast<std::size_t>(pageSkip + chunk), pdst);
			pos += chunk;
			pdst += chunk;
			totalRead += chunk;
			count -= static_cast<std::size_t>(chunk);
			if (chunkWanted > chunk) {
				return dst.first(static_cast<std::size_t>(totalRead));
			}
		}
		return dst.first(static_cast<std::size_t>(totalRead));
	}

	virtual mpt::byte_span InternalReadBuffered(pos_type pos, mpt::byte_span dst) const = 0;
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_READ_FILEDATA_BASE_BUFFERED_HPP
