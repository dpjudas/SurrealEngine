/*
 * FileReader.h
 * ------------
 * Purpose: A basic class for transparent reading of memory-based files.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/io_read/filecursor.hpp"
#include "mpt/io_read/filecursor_filename_traits.hpp"
#include "mpt/io_read/filecursor_traits_filedata.hpp"
#include "mpt/io_read/filecursor_traits_memory.hpp"
#include "mpt/io_read/filereader.hpp"

#include "openmpt/base/Types.hpp"

#include "mptPathString.h"
#include "mptStringBuffer.h"

#include <algorithm>
#include <array>
#include <limits>
#include <optional>
#include <string>
#include <vector>

#include <cstddef>
#include <cstring>

#include "FileReaderFwd.h"


OPENMPT_NAMESPACE_BEGIN


namespace FileReaderExt
{

	// Read a string of length srcSize into fixed-length char array destBuffer using a given read mode.
	// The file cursor is advanced by "srcSize" bytes.
	// Returns true if at least one byte could be read or 0 bytes were requested.
	template<mpt::String::ReadWriteMode mode, std::size_t destSize, typename TFileCursor>
	bool ReadString(TFileCursor &f, char (&destBuffer)[destSize], const std::size_t srcSize)
	{
		typename TFileCursor::PinnedView source = f.ReadPinnedView(srcSize); // Make sure the string is cached properly.
		std::size_t realSrcSize = source.size();	// In case fewer bytes are available
		mpt::String::WriteAutoBuf(destBuffer) = mpt::String::ReadBuf(mode, mpt::byte_cast<const char*>(source.data()), realSrcSize);
		return (realSrcSize > 0 || srcSize == 0);
	}

	// Read a string of length srcSize into a std::string dest using a given read mode.
	// The file cursor is advanced by "srcSize" bytes.
	// Returns true if at least one character could be read or 0 characters were requested.
	template<mpt::String::ReadWriteMode mode, typename TFileCursor>
	bool ReadString(TFileCursor &f, std::string &dest, const std::size_t srcSize)
	{
		dest.clear();
		typename TFileCursor::PinnedView source = f.ReadPinnedView(srcSize);	// Make sure the string is cached properly.
		std::size_t realSrcSize = source.size();	// In case fewer bytes are available
		dest = mpt::String::ReadBuf(mode, mpt::byte_cast<const char*>(source.data()), realSrcSize);
		return (realSrcSize > 0 || srcSize == 0);
	}

	// Read a string of length srcSize into a mpt::charbuf dest using a given read mode.
	// The file cursor is advanced by "srcSize" bytes.
	// Returns true if at least one character could be read or 0 characters were requested.
	template<mpt::String::ReadWriteMode mode, std::size_t len, typename TFileCursor>
	bool ReadString(TFileCursor &f, mpt::charbuf<len> &dest, const std::size_t srcSize)
	{
		typename TFileCursor::PinnedView source = f.ReadPinnedView(srcSize);	// Make sure the string is cached properly.
		std::size_t realSrcSize = source.size();	// In case fewer bytes are available
		dest = mpt::String::ReadBuf(mode, mpt::byte_cast<const char*>(source.data()), realSrcSize);
		return (realSrcSize > 0 || srcSize == 0);
	}

	// Read a charset encoded string of length srcSize into a mpt::ustring dest using a given read mode.
	// The file cursor is advanced by "srcSize" bytes.
	// Returns true if at least one character could be read or 0 characters were requested.
	template<mpt::String::ReadWriteMode mode, typename TFileCursor>
	bool ReadString(TFileCursor &f, mpt::ustring &dest, mpt::Charset charset, const std::size_t srcSize)
	{
		dest.clear();
		typename TFileCursor::PinnedView source = f.ReadPinnedView(srcSize);	// Make sure the string is cached properly.
		std::size_t realSrcSize = source.size();	// In case fewer bytes are available
		dest = mpt::ToUnicode(charset, mpt::String::ReadBuf(mode, mpt::byte_cast<const char*>(source.data()), realSrcSize));
		return (realSrcSize > 0 || srcSize == 0);
	}

	// Read a string with a preprended length field of type Tsize (must be a packed<*,*> type) into a std::string dest using a given read mode.
	// The file cursor is advanced by the string length.
	// Returns true if the size field could be read and at least one character could be read or 0 characters were requested.
	template<typename Tsize, mpt::String::ReadWriteMode mode, std::size_t destSize, typename TFileCursor>
	bool ReadSizedString(TFileCursor &f, char (&destBuffer)[destSize], const std::size_t maxLength = std::numeric_limits<std::size_t>::max())
	{
		static_assert(mpt::is_binary_safe<Tsize>::value);
		Tsize srcSize;
		if(!mpt::IO::FileReader::Read(f, srcSize))
		{
			return false;
		}
		return FileReaderExt::ReadString<mode>(f, destBuffer, std::min(static_cast<std::size_t>(srcSize), maxLength));
	}

	// Read a string with a preprended length field of type Tsize (must be a packed<*,*> type) into a std::string dest using a given read mode.
	// The file cursor is advanced by the string length.
	// Returns true if the size field could be read and at least one character could be read or 0 characters were requested.
	template<typename Tsize, mpt::String::ReadWriteMode mode, typename TFileCursor>
	bool ReadSizedString(TFileCursor &f, std::string &dest, const std::size_t maxLength = std::numeric_limits<std::size_t>::max())
	{
		static_assert(mpt::is_binary_safe<Tsize>::value);
		Tsize srcSize;
		if(!mpt::IO::FileReader::Read(f, srcSize))
		{
			return false;
		}
		return FileReaderExt::ReadString<mode>(f, dest, std::min(static_cast<std::size_t>(srcSize), maxLength));
	}

	// Read a string with a preprended length field of type Tsize (must be a packed<*,*> type) into a mpt::charbuf dest using a given read mode.
	// The file cursor is advanced by the string length.
	// Returns true if the size field could be read and at least one character could be read or 0 characters were requested.
	template<typename Tsize, mpt::String::ReadWriteMode mode, std::size_t len, typename TFileCursor>
	bool ReadSizedString(TFileCursor &f, mpt::charbuf<len> &dest, const std::size_t maxLength = std::numeric_limits<std::size_t>::max())
	{
		static_assert(mpt::is_binary_safe<Tsize>::value);
		Tsize srcSize;
		if(!mpt::IO::FileReader::Read(f, srcSize))
		{
			return false;
		}
		return FileReaderExt::ReadString<mode>(f, dest, std::min(static_cast<std::size_t>(srcSize), maxLength));
	}

} // namespace FileReaderExt

namespace detail {

template <typename Ttraits, typename Tfilenametraits>
using FileCursor = mpt::IO::FileCursor<Ttraits, Tfilenametraits>;

template <typename Ttraits, typename Tfilenametraits>
class FileReader
	: public FileCursor<Ttraits, Tfilenametraits>
{

private:

	using traits_type = Ttraits;
	using filename_traits_type = Tfilenametraits;

public:

	using pos_type = typename traits_type::pos_type;

	using data_type = typename traits_type::data_type;
	using ref_data_type = typename traits_type::ref_data_type;
	using shared_data_type = typename traits_type::shared_data_type;
	using value_data_type = typename traits_type::value_data_type;

	using shared_filename_type = typename filename_traits_type::shared_filename_type;

public:

	// Initialize invalid file reader object.
	FileReader()
	{
		return;
	}

	FileReader(const FileCursor<Ttraits, Tfilenametraits> &other)
		: FileCursor<Ttraits, Tfilenametraits>(other)
	{
		return;
	}
	FileReader(FileCursor<Ttraits, Tfilenametraits> &&other)
		: FileCursor<Ttraits, Tfilenametraits>(std::move(other))
	{
		return;
	}

	// Initialize file reader object with pointer to data and data length.
	template <typename Tbyte>
	explicit FileReader(mpt::span<Tbyte> bytedata, shared_filename_type filename = shared_filename_type{})
		: FileCursor<Ttraits, Tfilenametraits>(bytedata, std::move(filename))
	{
		return;
	}

	// Initialize file reader object based on an existing file reader object window.
	explicit FileReader(value_data_type other, shared_filename_type filename = shared_filename_type{})
		: FileCursor<Ttraits, Tfilenametraits>(std::move(other), std::move(filename))
	{
		return;
	}

public:

	template <typename T>
	bool Read(T &target)
	{
		return mpt::IO::FileReader::Read(*this, target);
	}

	template <typename T>
	T ReadIntLE()
	{
		return mpt::IO::FileReader::ReadIntLE<T>(*this);
	}

	template <typename T>
	T ReadIntBE()
	{
		return mpt::IO::FileReader::ReadIntLE<T>(*this);
	}

	template <typename T>
	T ReadTruncatedIntLE(std::size_t size)
	{
		return mpt::IO::FileReader::ReadTruncatedIntLE<T>(*this, size);
	}

	template <typename T>
	T ReadSizedIntLE(std::size_t size)
	{
		return mpt::IO::FileReader::ReadSizedIntLE<T>(*this, size);
	}

	uint32 ReadUint32LE()
	{
		return mpt::IO::FileReader::ReadUint32LE(*this);
	}

	uint32 ReadUint32BE()
	{
		return mpt::IO::FileReader::ReadUint32BE(*this);
	}

	int32 ReadInt32LE()
	{
		return mpt::IO::FileReader::ReadInt32LE(*this);
	}

	int32 ReadInt32BE()
	{
		return mpt::IO::FileReader::ReadInt32BE(*this);
	}

	uint32 ReadUint24LE()
	{
		return mpt::IO::FileReader::ReadUint24LE(*this);
	}

	uint32 ReadUint24BE()
	{
		return mpt::IO::FileReader::ReadUint24BE(*this);
	}

	uint16 ReadUint16LE()
	{
		return mpt::IO::FileReader::ReadUint16LE(*this);
	}

	uint16 ReadUint16BE()
	{
		return mpt::IO::FileReader::ReadUint16BE(*this);
	}

	int16 ReadInt16LE()
	{
		return mpt::IO::FileReader::ReadInt16LE(*this);
	}

	int16 ReadInt16BE()
	{
		return mpt::IO::FileReader::ReadInt16BE(*this);
	}

	char ReadChar()
	{
		return mpt::IO::FileReader::ReadChar(*this);
	}

	uint8 ReadUint8()
	{
		return mpt::IO::FileReader::ReadUint8(*this);
	}

	int8 ReadInt8()
	{
		return mpt::IO::FileReader::ReadInt8(*this);
	}

	float ReadFloatLE()
	{
		return mpt::IO::FileReader::ReadFloatLE(*this);
	}

	float ReadFloatBE()
	{
		return mpt::IO::FileReader::ReadFloatBE(*this);
	}

	double ReadDoubleLE()
	{
		return mpt::IO::FileReader::ReadDoubleLE(*this);
	}

	double ReadDoubleBE()
	{
		return mpt::IO::FileReader::ReadDoubleBE(*this);
	}

	template <typename T>
	bool ReadStruct(T &target)
	{
		return mpt::IO::FileReader::ReadStruct(*this, target);
	}

	template <typename T>
	std::size_t ReadStructPartial(T &target, std::size_t partialSize = sizeof(T))
	{
		return mpt::IO::FileReader::ReadStructPartial(*this, target, partialSize);
	}

	bool ReadNullString(std::string &dest, const std::size_t maxLength = std::numeric_limits<std::size_t>::max())
	{
		return mpt::IO::FileReader::ReadNullString(*this, dest, maxLength);
	}

	bool ReadLine(std::string &dest, const std::size_t maxLength = std::numeric_limits<std::size_t>::max())
	{
		return mpt::IO::FileReader::ReadLine(*this, dest, maxLength);
	}

	template<typename T, std::size_t destSize>
	bool ReadArray(T (&destArray)[destSize])
	{
		return mpt::IO::FileReader::ReadArray(*this, destArray);
	}

	template<typename T, std::size_t destSize>
	bool ReadArray(std::array<T, destSize> &destArray)
	{
		return mpt::IO::FileReader::ReadArray(*this, destArray);
	}

	template <typename T, std::size_t destSize>
	std::array<T, destSize> ReadArray()
	{
		return mpt::IO::FileReader::ReadArray<T, destSize>(*this);
	}

	template<typename T>
	bool ReadVector(std::vector<T> &destVector, std::size_t destSize)
	{
		return mpt::IO::FileReader::ReadVector(*this, destVector, destSize);
	}

	template<std::size_t N>
	bool ReadMagic(const char (&magic)[N])
	{
		return mpt::IO::FileReader::ReadMagic(*this, magic);
	}

	template<typename T>
	bool ReadVarInt(T &target)
	{
		return mpt::IO::FileReader::ReadVarInt(*this, target);
	}

	template <typename T>
	using Item = mpt::IO::FileReader::Chunk<T, FileReader>;

	template <typename T>
	using ChunkList = mpt::IO::FileReader::ChunkList<T, FileReader>;

	template<typename T>
	Item<T> ReadNextChunk(pos_type alignment)
	{
		return mpt::IO::FileReader::ReadNextChunk<T, FileReader>(*this, alignment);
	}

	template<typename T>
	ChunkList<T> ReadChunks(pos_type alignment)
	{
		return mpt::IO::FileReader::ReadChunks<T, FileReader>(*this, alignment);
	}

	template<typename T>
	ChunkList<T> ReadChunksUntil(pos_type alignment, decltype(T().GetID()) stopAtID)
	{
		return mpt::IO::FileReader::ReadChunksUntil<T, FileReader>(*this, alignment, stopAtID);
	}

	template<mpt::String::ReadWriteMode mode, std::size_t destSize>
	bool ReadString(char (&destBuffer)[destSize], const std::size_t srcSize)
	{
		return FileReaderExt::ReadString<mode>(*this, destBuffer, srcSize);
	}

	template<mpt::String::ReadWriteMode mode>
	bool ReadString(std::string &dest, const std::size_t srcSize)
	{
		return FileReaderExt::ReadString<mode>(*this, dest, srcSize);
	}

	template<mpt::String::ReadWriteMode mode, std::size_t len>
	bool ReadString(mpt::charbuf<len> &dest, const std::size_t srcSize)
	{
		return FileReaderExt::ReadString<mode>(*this, dest, srcSize);
	}

	template<mpt::String::ReadWriteMode mode>
	bool ReadString(mpt::ustring &dest, mpt::Charset charset, const std::size_t srcSize)
	{
		return FileReaderExt::ReadString<mode>(*this, dest, charset, srcSize);
	}

	template<typename Tsize, mpt::String::ReadWriteMode mode, std::size_t destSize>
	bool ReadSizedString(char (&destBuffer)[destSize], const std::size_t maxLength = std::numeric_limits<std::size_t>::max())
	{
		return FileReaderExt::ReadSizedString<Tsize, mode>(*this, destBuffer, maxLength);
	}

	template<typename Tsize, mpt::String::ReadWriteMode mode>
	bool ReadSizedString(std::string &dest, const std::size_t maxLength = std::numeric_limits<std::size_t>::max())
	{
		return FileReaderExt::ReadSizedString<Tsize, mode>(*this, dest, maxLength);
	}

	template<typename Tsize, mpt::String::ReadWriteMode mode, std::size_t len>
	bool ReadSizedString(mpt::charbuf<len> &dest, const std::size_t maxLength = std::numeric_limits<std::size_t>::max())
	{
		return FileReaderExt::ReadSizedString<Tsize, mode, len>(*this, dest, maxLength);
	}

};

} // namespace detail

using FileCursor = detail::FileCursor<mpt::IO::FileCursorTraitsFileData, mpt::IO::FileCursorFilenameTraits<mpt::PathString>>;
using FileReader = detail::FileReader<mpt::IO::FileCursorTraitsFileData, mpt::IO::FileCursorFilenameTraits<mpt::PathString>>;

using ChunkReader = FileReader;

using MemoryFileCursor = detail::FileCursor<mpt::IO::FileCursorTraitsMemory, mpt::IO::FileCursorFilenameTraitsNone>;
using MemoryFileReader = detail::FileReader<mpt::IO::FileCursorTraitsMemory, mpt::IO::FileCursorFilenameTraitsNone>;


OPENMPT_NAMESPACE_END
