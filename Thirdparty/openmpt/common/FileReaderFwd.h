/*
 * FileReaderFwd.h
 * ---------------
 * Purpose: Forward declaration for class FileReader.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/namespace.hpp"

namespace mpt {
inline namespace MPT_INLINE_NS {

template <typename Traits, bool allow_transcode_locale>
class BasicPathString;
struct NativePathTraits;
struct Utf8PathTraits;
using native_path = BasicPathString<NativePathTraits, true>;

namespace IO {

class FileCursorTraitsMemory;

class FileCursorTraitsFileData;

class FileCursorFilenameTraitsNone;

template <typename Tpath>
class FileCursorFilenameTraits;

template <typename Ttraits, typename Tfilenametraits>
class FileCursor;

} // namespace IO

} // inline namespace MPT_INLINE_NS
} // namespace mpt

OPENMPT_NAMESPACE_BEGIN

namespace mpt {

} // namespace mpt

namespace detail {

template <typename Ttraits, typename Tfilenametraits>
using FileCursor = mpt::IO::FileCursor<Ttraits, Tfilenametraits>;

template <typename Ttraits, typename Tfilenametraits>
class FileReader;

} // namespace detail

namespace mpt {
#if defined(MPT_ENABLE_CHARSET_LOCALE)
using PathString = mpt::native_path;
#else
using PathString = mpt::BasicPathString<mpt::Utf8PathTraits, false>;
#endif
} // namespace mpt

using FileCursor = detail::FileCursor<mpt::IO::FileCursorTraitsFileData, mpt::IO::FileCursorFilenameTraits<mpt::PathString>>;
using FileReader = detail::FileReader<mpt::IO::FileCursorTraitsFileData, mpt::IO::FileCursorFilenameTraits<mpt::PathString>>;

using MemoryFileCursor = detail::FileCursor<mpt::IO::FileCursorTraitsMemory, mpt::IO::FileCursorFilenameTraitsNone>;
using MemoryFileReader = detail::FileReader<mpt::IO::FileCursorTraitsMemory, mpt::IO::FileCursorFilenameTraitsNone>;

OPENMPT_NAMESPACE_END

