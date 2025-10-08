/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_READ_FILECURSOR_STDSTREAM_HPP
#define MPT_IO_READ_FILECURSOR_STDSTREAM_HPP



#include "mpt/base/namespace.hpp"
#include "mpt/io_read/filecursor.hpp"
#include "mpt/io_read/filecursor_filename_traits.hpp"
#include "mpt/io_read/filecursor_traits_filedata.hpp"
#include "mpt/io_read/filedata_stdstream.hpp"

#include <istream>
#include <memory>
#include <utility>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



// Initialize file reader object with a std::istream.
template <typename Tpath>
inline FileCursor<FileCursorTraitsFileData, FileCursorFilenameTraits<Tpath>> make_FileCursor(std::istream & s, std::shared_ptr<Tpath> filename = nullptr) {
	if (FileDataStdStream::IsSeekable(s)) {
		return FileCursor<FileCursorTraitsFileData, FileCursorFilenameTraits<Tpath>>(std::static_pointer_cast<IFileData>(std::make_shared<FileDataStdStreamSeekable>(s)), std::move(filename));
	} else {
		return FileCursor<FileCursorTraitsFileData, FileCursorFilenameTraits<Tpath>>(std::static_pointer_cast<IFileData>(std::make_shared<FileDataStdStreamUnseekable>(s)), std::move(filename));
	}
}



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_READ_FILECURSOR_STDSTREAM_HPP
