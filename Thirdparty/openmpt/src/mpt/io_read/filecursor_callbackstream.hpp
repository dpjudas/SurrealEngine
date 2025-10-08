/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_READ_FILECURSOR_CALLBACKSTREAM_HPP
#define MPT_IO_READ_FILECURSOR_CALLBACKSTREAM_HPP



#include "mpt/base/namespace.hpp"
#include "mpt/io_read/callbackstream.hpp"
#include "mpt/io_read/filecursor.hpp"
#include "mpt/io_read/filecursor_filename_traits.hpp"
#include "mpt/io_read/filecursor_traits_filedata.hpp"
#include "mpt/io_read/filedata_callbackstream.hpp"

#include <memory>
#include <utility>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



// Initialize file reader object with a CallbackStream.
template <typename Tpath, typename Tstream>
inline FileCursor<FileCursorTraitsFileData, FileCursorFilenameTraits<Tpath>> make_FileCursor(CallbackStreamTemplate<Tstream> s, std::shared_ptr<Tpath> filename = nullptr) {
	if (FileDataCallbackStreamTemplate<Tstream>::IsSeekable(s)) {
		return FileCursor<FileCursorTraitsFileData, FileCursorFilenameTraits<Tpath>>(std::static_pointer_cast<IFileData>(std::make_shared<FileDataCallbackStreamSeekableTemplate<Tstream>>(s)), std::move(filename));
	} else {
		return FileCursor<FileCursorTraitsFileData, FileCursorFilenameTraits<Tpath>>(std::static_pointer_cast<IFileData>(std::make_shared<FileDataCallbackStreamUnseekableTemplate<Tstream>>(s)), std::move(filename));
	}
}



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_READ_FILECURSOR_CALLBACKSTREAM_HPP
