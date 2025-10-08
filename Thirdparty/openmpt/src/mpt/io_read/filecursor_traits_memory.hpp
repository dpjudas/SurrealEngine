/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_READ_FILECURSOR_TRAITS_MEMORY_HPP
#define MPT_IO_READ_FILECURSOR_TRAITS_MEMORY_HPP



#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/span.hpp"
#include "mpt/io_read/filedata.hpp"
#include "mpt/io_read/filedata_memory.hpp"

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



class FileCursorTraitsMemory {

public:
	using pos_type = FileDataMemory::pos_type;

	using data_type = FileDataMemory;
	using ref_data_type = const FileDataMemory &;
	using shared_data_type = const FileDataMemory &;
	using value_data_type = FileDataMemory;

	static shared_data_type get_shared(const data_type & data) {
		return data;
	}
	static ref_data_type get_ref(const data_type & data) {
		return data;
	}

	static value_data_type make_data() {
		return mpt::const_byte_span();
	}
	static value_data_type make_data(mpt::const_byte_span data) {
		return data;
	}

	static value_data_type make_chunk(shared_data_type data, pos_type position, pos_type size) {
		return mpt::as_span(data.GetRawData() + static_cast<std::size_t>(position), static_cast<std::size_t>(size));
	}
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_READ_FILECURSOR_TRAITS_MEMORY_HPP
