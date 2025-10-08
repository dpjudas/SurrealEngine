/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_READ_FILECURSOR_MEMORY_HPP
#define MPT_IO_READ_FILECURSOR_MEMORY_HPP



#include "mpt/base/namespace.hpp"
#include "mpt/base/span.hpp"
#include "mpt/io_read/filecursor.hpp"
#include "mpt/io_read/filecursor_filename_traits.hpp"
#include "mpt/io_read/filecursor_traits_filedata.hpp"

#include <memory>
#include <utility>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



// Initialize file reader object with pointer to data and data length.
template <typename Tpath, typename Tbyte>
inline FileCursor<FileCursorTraitsFileData, FileCursorFilenameTraits<Tpath>> make_FileCursor(mpt::span<Tbyte> bytedata, std::shared_ptr<Tpath> filename = nullptr) {
	return FileCursor<FileCursorTraitsFileData, FileCursorFilenameTraits<Tpath>>(mpt::byte_cast<mpt::const_byte_span>(bytedata), std::move(filename));
}



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_READ_FILECURSOR_STDSTREAM_HPP
