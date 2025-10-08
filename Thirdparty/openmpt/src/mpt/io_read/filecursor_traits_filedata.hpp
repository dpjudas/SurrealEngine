/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_READ_FILECURSOR_TRAITS_FILEDATA_HPP
#define MPT_IO_READ_FILECURSOR_TRAITS_FILEDATA_HPP



#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/io_read/filedata.hpp"
#include "mpt/io_read/filedata_base.hpp"
#include "mpt/io_read/filedata_memory.hpp"

#include <memory>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



class FileCursorTraitsFileData {

public:
	using pos_type = IFileData::pos_type;

	using data_type = std::shared_ptr<const IFileData>;
	using ref_data_type = const IFileData &;
	using shared_data_type = std::shared_ptr<const IFileData>;
	using value_data_type = std::shared_ptr<const IFileData>;

	static shared_data_type get_shared(const data_type & data) {
		return data;
	}
	static ref_data_type get_ref(const data_type & data) {
		return *data;
	}

	static value_data_type make_data() {
		return std::make_shared<FileDataDummy>();
	}
	static value_data_type make_data(mpt::const_byte_span data) {
		return std::make_shared<FileDataMemory>(data);
	}

	static value_data_type make_chunk(shared_data_type data, pos_type position, pos_type size) {
		return std::static_pointer_cast<IFileData>(std::make_shared<FileDataWindow>(data, position, size));
	}
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_READ_FILECURSOR_TRAITS_FILEDATA_HPP
