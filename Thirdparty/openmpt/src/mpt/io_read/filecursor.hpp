/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_READ_FILECURSOR_HPP
#define MPT_IO_READ_FILECURSOR_HPP



#include "mpt/base/alloc.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/span.hpp"
#include "mpt/base/utility.hpp"
#include "mpt/out_of_memory/out_of_memory.hpp"

#include <algorithm>
#include <optional>
#include <vector>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



// change to show warnings for functions which trigger pre-caching the whole file for unseekable streams
//#define MPT_FILECURSOR_DEPRECATED [[deprecated]]
#define MPT_FILECURSOR_DEPRECATED



template <typename Ttraits, typename Tfilenametraits>
class FileCursor {

private:
	using traits_type = Ttraits;
	using filename_traits_type = Tfilenametraits;

public:
	using pos_type = typename traits_type::pos_type;

	using data_type = typename traits_type::data_type;
	using ref_data_type = typename traits_type::ref_data_type;
	using shared_data_type = typename traits_type::shared_data_type;
	using value_data_type = typename traits_type::value_data_type;

	using filename_type = typename filename_traits_type::filename_type;
	using shared_filename_type = typename filename_traits_type::shared_filename_type;

protected:
	shared_data_type SharedDataContainer() const {
		return traits_type::get_shared(m_data);
	}
	ref_data_type DataContainer() const {
		return traits_type::get_ref(m_data);
	}

	static value_data_type DataInitializer() {
		return traits_type::make_data();
	}
	static value_data_type DataInitializer(mpt::const_byte_span data) {
		return traits_type::make_data(data);
	}

	static value_data_type CreateChunkImpl(shared_data_type data, pos_type position, pos_type size) {
		return traits_type::make_chunk(data, position, size);
	}

private:
	data_type m_data;

	pos_type streamPos; // Cursor location in the file

	shared_filename_type m_fileName; // Filename that corresponds to this FileCursor. It is only set if this FileCursor represents the whole contents of fileName. May be nullopt.

public:
	// Initialize invalid file reader object.
	FileCursor()
		: m_data(DataInitializer())
		, streamPos(0)
		, m_fileName(nullptr) {
		return;
	}

	// Initialize file reader object with pointer to data and data length.
	template <typename Tbyte>
	explicit FileCursor(mpt::span<Tbyte> bytedata, shared_filename_type filename = shared_filename_type{})
		: m_data(DataInitializer(mpt::byte_cast<mpt::const_byte_span>(bytedata)))
		, streamPos(0)
		, m_fileName(std::move(filename)) {
		return;
	}

	// Initialize file reader object based on an existing file reader object window.
	explicit FileCursor(value_data_type other, shared_filename_type filename = shared_filename_type{})
		: m_data(std::move(other))
		, streamPos(0)
		, m_fileName(std::move(filename)) {
		return;
	}

public:
	std::optional<filename_type> GetOptionalFileName() const {
		return filename_traits_type::get_optional_filename(m_fileName);
	}

	// Returns true if the object points to a valid (non-empty) stream.
	operator bool() const {
		return IsValid();
	}

	// Returns true if the object points to a valid (non-empty) stream.
	bool IsValid() const {
		return DataContainer().IsValid();
	}

	// Reset cursor to first byte in file.
	void Rewind() {
		streamPos = 0;
	}

	// Seek to a position in the mapped file.
	// Returns false if position is invalid.
	bool Seek(pos_type position) {
		if (position <= streamPos) {
			streamPos = position;
			return true;
		}
		if (DataContainer().CanRead(0, position)) {
			streamPos = position;
			return true;
		} else {
			return false;
		}
	}

	// Increases position by skipBytes.
	// Returns true if skipBytes could be skipped or false if the file end was reached earlier.
	bool Skip(pos_type skipBytes) {
		if (CanRead(skipBytes)) {
			streamPos += skipBytes;
			return true;
		} else {
			streamPos = DataContainer().GetLength();
			return false;
		}
	}

	// Decreases position by skipBytes.
	// Returns true if skipBytes could be skipped or false if the file start was reached earlier.
	bool SkipBack(pos_type skipBytes) {
		if (streamPos >= skipBytes) {
			streamPos -= skipBytes;
			return true;
		} else {
			streamPos = 0;
			return false;
		}
	}

	// Returns cursor position in the mapped file.
	pos_type GetPosition() const {
		return streamPos;
	}

	// Return true IFF seeking and GetLength() is fast.
	// In particular, it returns false for unseekable stream where GetLength()
	// requires pre-caching.
	bool HasFastGetLength() const {
		return DataContainer().HasFastGetLength();
	}

	// Returns size of the mapped file in bytes.
	MPT_FILECURSOR_DEPRECATED pos_type GetLength() const {
		// deprecated because in case of an unseekable std::istream, this triggers caching of the whole file
		return DataContainer().GetLength();
	}

	// Return byte count between cursor position and end of file, i.e. how many bytes can still be read.
	MPT_FILECURSOR_DEPRECATED pos_type BytesLeft() const {
		// deprecated because in case of an unseekable std::istream, this triggers caching of the whole file
		return DataContainer().GetLength() - streamPos;
	}

	bool EndOfFile() const {
		return !CanRead(1);
	}

	bool NoBytesLeft() const {
		return !CanRead(1);
	}

	// Check if "amount" bytes can be read from the current position in the stream.
	bool CanRead(pos_type amount) const {
		return DataContainer().CanRead(streamPos, amount);
	}

	// Check if file size is at least size, without potentially caching the whole file to query the exact file length.
	bool LengthIsAtLeast(pos_type size) const {
		return DataContainer().CanRead(0, size);
	}

	// Check if file size is exactly size, without potentially caching the whole file if it is larger.
	bool LengthIs(pos_type size) const {
		return DataContainer().CanRead(0, size) && !DataContainer().CanRead(size, 1);
	}

protected:
	FileCursor CreateChunk(pos_type position, pos_type length) const {
		pos_type readableLength = DataContainer().GetReadableLength(position, length);
		if (readableLength == 0) {
			return FileCursor();
		}
		return FileCursor(CreateChunkImpl(SharedDataContainer(), position, readableLength));
	}

public:
	// Create a new FileCursor object for parsing a sub chunk at a given position with a given length.
	// The file cursor is not modified.
	FileCursor GetChunkAt(pos_type position, pos_type length) const {
		return CreateChunk(position, length);
	}

	// Create a new FileCursor object for parsing a sub chunk at the current position with a given length.
	// The file cursor is not advanced.
	FileCursor GetChunk(pos_type length) {
		return CreateChunk(streamPos, length);
	}
	// Create a new FileCursor object for parsing a sub chunk at the current position with a given length.
	// The file cursor is advanced by "length" bytes.
	FileCursor ReadChunk(pos_type length) {
		pos_type position = streamPos;
		Skip(length);
		return CreateChunk(position, length);
	}

	class PinnedView {
	private:
		std::size_t size_;
		const std::byte * pinnedData;
		std::vector<std::byte> cache;

	private:
		void Init(const FileCursor & file, std::size_t size) {
			size_ = 0;
			pinnedData = nullptr;
			if (!file.CanRead(size)) {
				size = static_cast<std::size_t>(file.BytesLeft());
			}
			size_ = size;
			if (file.DataContainer().HasPinnedView()) {
				pinnedData = file.DataContainer().GetRawData() + file.GetPosition();
			} else {
				cache.resize(size_);
				if (!cache.empty()) {
					file.GetRaw(mpt::as_span(cache));
				}
			}
		}

	public:
		PinnedView()
			: size_(0)
			, pinnedData(nullptr) {
		}
		PinnedView(const FileCursor & file) {
			MPT_MAYBE_CONSTANT_IF (!mpt::in_range<std::size_t>(file.BytesLeft())) {
				mpt::throw_out_of_memory();
			}
			Init(file, static_cast<std::size_t>(file.BytesLeft()));
		}
		PinnedView(const FileCursor & file, std::size_t size) {
			Init(file, size);
		}
		PinnedView(FileCursor & file, bool advance) {
			MPT_MAYBE_CONSTANT_IF (!mpt::in_range<std::size_t>(file.BytesLeft())) {
				mpt::throw_out_of_memory();
			}
			Init(file, static_cast<std::size_t>(file.BytesLeft()));
			if (advance) {
				file.Skip(size_);
			}
		}
		PinnedView(FileCursor & file, std::size_t size, bool advance) {
			Init(file, size);
			if (advance) {
				file.Skip(size_);
			}
		}

	public:
		mpt::const_byte_span GetSpan() const {
			if (pinnedData) {
				return mpt::as_span(pinnedData, size_);
			} else if (!cache.empty()) {
				return mpt::as_span(cache);
			} else {
				return mpt::const_byte_span();
			}
		}
		mpt::const_byte_span span() const {
			return GetSpan();
		}
		void invalidate() {
			size_ = 0;
			pinnedData = nullptr;
			cache = std::vector<std::byte>();
		}
		const std::byte * data() const {
			return span().data();
		}
		std::size_t size() const {
			return size_;
		}
		mpt::const_byte_span::pointer begin() const {
			return span().data();
		}
		mpt::const_byte_span::pointer end() const {
			return span().data() + span().size();
		}
		mpt::const_byte_span::const_pointer cbegin() const {
			return span().data();
		}
		mpt::const_byte_span::const_pointer cend() const {
			return span().data() + span().size();
		}
	};

	// Returns a pinned view into the remaining raw data from cursor position.
	PinnedView GetPinnedView() const {
		return PinnedView(*this);
	}
	// Returns a pinned view into the remeining raw data from cursor position, clamped at size.
	PinnedView GetPinnedView(std::size_t size) const {
		return PinnedView(*this, size);
	}

	// Returns a pinned view into the remeining raw data from cursor position.
	// File cursor is advaned by the size of the returned pinned view.
	PinnedView ReadPinnedView() {
		return PinnedView(*this, true);
	}
	// Returns a pinned view into the remeining raw data from cursor position, clamped at size.
	// File cursor is advaned by the size of the returned pinned view.
	PinnedView ReadPinnedView(std::size_t size) {
		return PinnedView(*this, size, true);
	}

	template <typename Tspan>
	Tspan GetRawWithOffset(std::size_t offset, Tspan dst) const {
		return mpt::byte_cast<Tspan>(DataContainer().Read(streamPos + offset, mpt::byte_cast<mpt::byte_span>(dst)));
	}

	template <typename Tspan>
	Tspan GetRaw(Tspan dst) const {
		return mpt::byte_cast<Tspan>(DataContainer().Read(streamPos, mpt::byte_cast<mpt::byte_span>(dst)));
	}

	template <typename Tspan>
	Tspan ReadRaw(Tspan dst) {
		Tspan result = mpt::byte_cast<Tspan>(DataContainer().Read(streamPos, mpt::byte_cast<mpt::byte_span>(dst)));
		streamPos += result.size();
		return result;
	}

	std::vector<std::byte> GetRawDataAsByteVector() const {
		PinnedView view = GetPinnedView();
		return mpt::make_vector(view.span());
	}
	std::vector<std::byte> ReadRawDataAsByteVector() {
		PinnedView view = ReadPinnedView();
		return mpt::make_vector(view.span());
	}
	std::vector<std::byte> GetRawDataAsByteVector(std::size_t size) const {
		PinnedView view = GetPinnedView(size);
		return mpt::make_vector(view.span());
	}
	std::vector<std::byte> ReadRawDataAsByteVector(std::size_t size) {
		PinnedView view = ReadPinnedView(size);
		return mpt::make_vector(view.span());
	}
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_READ_FILECURSOR_HPP
