/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_FILE_READ_INPUTFILE_FILECURSOR_HPP
#define MPT_IO_FILE_READ_INPUTFILE_FILECURSOR_HPP



#include "mpt/base/namespace.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "mpt/io_file/inputfile.hpp"
#include "mpt/io_read/filecursor_filename_traits.hpp"
#include "mpt/io_read/filecursor_memory.hpp"
#include "mpt/io_read/filecursor_stdstream.hpp"
#include "mpt/io_read/filecursor_traits_filedata.hpp"
#include "mpt/io_read/filedata_memory.hpp"
#include "mpt/io_read/filedata_stdstream.hpp"



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



// templated in order to reduce header inter-dependencies
template <typename Tpath>
inline FileCursor<FileCursorTraitsFileData, FileCursorFilenameTraits<Tpath>> make_FileCursor(UncachedInputFile & file) {
	if (!file.IsValid()) {
		return FileCursor<FileCursorTraitsFileData, FileCursorFilenameTraits<Tpath>>();
	}
	return mpt::IO::make_FileCursor<Tpath>(file.GetStream(), std::make_shared<Tpath>(file.GetFilename()));
}



// templated in order to reduce header inter-dependencies
template <typename Tpath>
inline FileCursor<FileCursorTraitsFileData, FileCursorFilenameTraits<Tpath>> make_FileCursor(CachedInputFile & file) {
	if (!file.IsValid()) {
		return FileCursor<FileCursorTraitsFileData, FileCursorFilenameTraits<Tpath>>();
	}
	if (file.IsCached()) {
		return mpt::IO::make_FileCursor<Tpath>(file.GetCache(), std::make_shared<Tpath>(file.GetFilename()));
	} else {
		return mpt::IO::make_FileCursor<Tpath>(file.GetStream(), std::make_shared<Tpath>(file.GetFilename()));
	}
}



// templated in order to reduce header inter-dependencies
template <typename Tpath>
inline FileCursor<FileCursorTraitsFileData, FileCursorFilenameTraits<Tpath>> make_FileCursor(InputFile & file) {
	if (!file.IsValid()) {
		return FileCursor<FileCursorTraitsFileData, FileCursorFilenameTraits<Tpath>>();
	}
	if (file.IsCached()) {
		return mpt::IO::make_FileCursor<Tpath>(file.GetCache(), std::make_shared<Tpath>(file.GetFilename()));
	} else {
		return mpt::IO::make_FileCursor<Tpath>(file.GetStream(), std::make_shared<Tpath>(file.GetFilename()));
	}
}



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_FILE_READ_INPUTFILE_FILECURSOR_HPP
