/*
 * serialization_utils.h
 * ---------------------
 * Purpose: Serializing data to and from MPTM files.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/alloc.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "openmpt/base/Endian.hpp"

#include "../common/mptBaseTypes.h"

#include <algorithm>
#include <bitset>
#include <ios>
#include <iosfwd>
#include <limits>
#include <string>
#include <vector>

#include <istream>
#include <ostream>

#include <cstddef>
#include <cstring>

OPENMPT_NAMESPACE_BEGIN


#ifdef MPT_ALL_LOGGING
#define SSB_LOGGING
#endif


#ifdef SSB_LOGGING
#define SSB_LOG(x) MPT_LOG_GLOBAL(LogDebug, "serialization", (x))
#else
#define SSB_LOG(x) do { } while(0)
#endif


namespace srlztn //SeRiaLiZaTioN
{


constexpr inline std::size_t invalidDatasize = static_cast<std::size_t>(0) - 1;


enum class StatusLevel : uint8
{
	Failure = 0x2,
	Note    = 0x1,
	None    = 0x0,
};

enum class StatusMessages : uint32
{
	None = 0,

	// Read notes and warnings.
	SNR_ZEROENTRYCOUNT                           = 0x00'00'00'01,
	SNR_NO_ENTRYIDS_WITH_CUSTOMID_DEFINED        = 0x00'00'00'02,
	SNR_LOADING_OBJECT_WITH_LARGER_VERSION       = 0x00'00'00'04,

	// Read failures.
	SNR_BADSTREAM_AFTER_MAPHEADERSEEK            = 0x00'00'01'00,
	SNR_STARTBYTE_MISMATCH                       = 0x00'00'02'00,
	SNR_BADSTREAM_AT_MAP_READ                    = 0x00'00'04'00,
	SNR_INSUFFICIENT_STREAM_OFFTYPE              = 0x00'00'08'00,
	SNR_OBJECTCLASS_IDMISMATCH                   = 0x00'00'10'00,
	SNR_TOO_MANY_ENTRIES_TO_READ                 = 0x00'00'20'00,
	SNR_INSUFFICIENT_RPOSTYPE                    = 0x00'00'40'00,

	// Write failures.
	SNW_INSUFFICIENT_FIXEDSIZE                   = 0x00'01'00'00,
	SNW_CHANGING_IDSIZE_WITH_FIXED_IDSIZESETTING = 0x00'02'00'00,
	SNW_DATASIZETYPE_OVERFLOW                    = 0x00'04'00'00,
	SNW_MAX_WRITE_COUNT_REACHED                  = 0x00'08'00'00,
	SNW_INSUFFICIENT_DATASIZETYPE                = 0x00'10'00'00,

	SNRW_BADGIVEN_STREAM                         = 0x01'00'00'00,
};

struct Status
{
	StatusLevel level = StatusLevel::None;
	StatusMessages messages = StatusMessages::None;
};

constexpr inline Status SNR_ZEROENTRYCOUNT = {StatusLevel::Note, StatusMessages::SNR_ZEROENTRYCOUNT};
constexpr inline Status SNR_NO_ENTRYIDS_WITH_CUSTOMID_DEFINED = {StatusLevel::Note, StatusMessages::SNR_NO_ENTRYIDS_WITH_CUSTOMID_DEFINED};
constexpr inline Status SNR_LOADING_OBJECT_WITH_LARGER_VERSION = {StatusLevel::Note, StatusMessages::SNR_LOADING_OBJECT_WITH_LARGER_VERSION};

constexpr inline Status SNR_BADSTREAM_AFTER_MAPHEADERSEEK = {StatusLevel::Failure, StatusMessages::SNR_BADSTREAM_AFTER_MAPHEADERSEEK};
constexpr inline Status SNR_STARTBYTE_MISMATCH = {StatusLevel::Failure, StatusMessages::SNR_STARTBYTE_MISMATCH};
constexpr inline Status SNR_BADSTREAM_AT_MAP_READ = {StatusLevel::Failure, StatusMessages::SNR_BADSTREAM_AT_MAP_READ};
constexpr inline Status SNR_INSUFFICIENT_STREAM_OFFTYPE = {StatusLevel::Failure, StatusMessages::SNR_INSUFFICIENT_STREAM_OFFTYPE};
constexpr inline Status SNR_OBJECTCLASS_IDMISMATCH = {StatusLevel::Failure, StatusMessages::SNR_OBJECTCLASS_IDMISMATCH};
constexpr inline Status SNR_TOO_MANY_ENTRIES_TO_READ = {StatusLevel::Failure, StatusMessages::SNR_TOO_MANY_ENTRIES_TO_READ};
constexpr inline Status SNR_INSUFFICIENT_RPOSTYPE = {StatusLevel::Failure, StatusMessages::SNR_INSUFFICIENT_RPOSTYPE};

constexpr inline Status SNW_INSUFFICIENT_FIXEDSIZE = {StatusLevel::Failure, StatusMessages::SNW_INSUFFICIENT_FIXEDSIZE};
constexpr inline Status SNW_CHANGING_IDSIZE_WITH_FIXED_IDSIZESETTING = {StatusLevel::Failure, StatusMessages::SNW_CHANGING_IDSIZE_WITH_FIXED_IDSIZESETTING};
constexpr inline Status SNW_DATASIZETYPE_OVERFLOW = {StatusLevel::Failure, StatusMessages::SNW_DATASIZETYPE_OVERFLOW};
constexpr inline Status SNW_MAX_WRITE_COUNT_REACHED = {StatusLevel::Failure, StatusMessages::SNW_MAX_WRITE_COUNT_REACHED};
constexpr inline Status SNW_INSUFFICIENT_DATASIZETYPE = {StatusLevel::Failure, StatusMessages::SNW_INSUFFICIENT_DATASIZETYPE};

constexpr inline Status SNRW_BADGIVEN_STREAM = {StatusLevel::Failure, StatusMessages::SNRW_BADGIVEN_STREAM};


enum : uint16
{
	IdSizeVariable = std::numeric_limits<uint16>::max(),
	IdSizeMaxFixedSize = (std::numeric_limits<uint8>::max() >> 1)
};

struct ReadEntry
{
	std::size_t nIdpos = 0;               // Index of id start in ID array.
	std::streamoff rposStart = 0;         // Entry start position.
	std::size_t nSize = invalidDatasize;  // Entry size.
	uint16 nIdLength = 0;                 // Length of id.
};


enum Rwf
{
	RwfWMapStartPosEntry,	// Write. True to include data start pos entry to map.
	RwfWMapSizeEntry,		// Write. True to include data size entry to map.
	RwfWMapDescEntry,		// Write. True to include description entry to map.
	RwfWVersionNum,			// Write. True to include version numeric.
	RwfRMapCached,			// Read. True if map has been cached.
	RwfRMapHasId,			// Read. True if map has IDs
	RwfRMapHasStartpos,		// Read. True if map data start pos.
	RwfRMapHasSize,			// Read. True if map has entry size.
	RwfRMapHasDesc,			// Read. True if map has entry description.
	RwfRTwoBytesDescChar,	// Read. True if map description characters are two bytes.
	RwfRHeaderIsRead,		// Read. True when header is read.
	RwfRwHasMap,			// Read/write. True if map exists.

	RwfNumFlags
};


template<class T>
inline void Binarywrite(std::ostream& oStrm, const T& data)
{
	mpt::IO::WriteIntLE(oStrm, data);
}

template<>
inline void Binarywrite(std::ostream& oStrm, const float& data)
{
	IEEE754binary32LE tmp = IEEE754binary32LE(data);
	mpt::IO::Write(oStrm, tmp);
}

template<>
inline void Binarywrite(std::ostream& oStrm, const double& data)
{
	IEEE754binary64LE tmp = IEEE754binary64LE(data);
	mpt::IO::Write(oStrm, tmp);
}

template <class T>
inline void WriteItem(std::ostream& oStrm, const T& data)
{
	static_assert(std::is_trivial<T>::value == true, "");
	Binarywrite(oStrm, data);
}

void WriteItemString(std::ostream& oStrm, const std::string &str);

template <>
inline void WriteItem<std::string>(std::ostream& oStrm, const std::string& str) {WriteItemString(oStrm, str);}


template<class T>
inline void Binaryread(std::istream& iStrm, T& data)
{
	mpt::IO::ReadIntLE(iStrm, data);
}

template<>
inline void Binaryread(std::istream& iStrm, float& data)
{
	IEEE754binary32LE tmp = IEEE754binary32LE(0.0f);
	mpt::IO::Read(iStrm, tmp);
	data = tmp;
}

template<>
inline void Binaryread(std::istream& iStrm, double& data)
{
	IEEE754binary64LE tmp = IEEE754binary64LE(0.0);
	mpt::IO::Read(iStrm, tmp);
	data = tmp;
}

//Read only given number of bytes to the beginning of data; data bytes are memset to 0 before reading.
template <class T>
inline void Binaryread(std::istream& iStrm, T& data, const std::size_t bytecount)
{
	mpt::IO::ReadBinaryTruncatedLE(iStrm, data, bytecount);
}

template <>
inline void Binaryread<float>(std::istream& iStrm, float& data, const std::size_t bytecount)
{
	using T = IEEE754binary32LE;
	mpt::IO::SeekRelative(iStrm, std::min(bytecount, sizeof(T)));
	// There is not much we can sanely do for truncated floats,
	// thus we ignore what we could read and return 0.
	data = 0.0f;
}

template <>
inline void Binaryread<double>(std::istream& iStrm, double& data, const std::size_t bytecount)
{
	using T = IEEE754binary64LE;
	mpt::IO::SeekRelative(iStrm, std::min(bytecount, sizeof(T)));
	// There is not much we can sanely do for truncated floats,
	// thus we ignore what we could read and return 0.
	data = 0.0;
}


template <class T>
inline void ReadItem(std::istream& iStrm, T& data, const std::size_t nSize)
{
	static_assert(std::is_trivial<T>::value == true, "");
	if (nSize == sizeof(T) || nSize == invalidDatasize)
		Binaryread(iStrm, data);
	else
		Binaryread(iStrm, data, nSize);
}

void ReadItemString(std::istream& iStrm, std::string& str, const std::size_t);

template <>
inline void ReadItem<std::string>(std::istream& iStrm, std::string& str, const std::size_t nSize)
{
	ReadItemString(iStrm, str, nSize);
}



class ID
{
private:
	std::string m_ID; // NOTE: can contain null characters ('\0')
public:
	ID() = default;
	ID(const std::string &id) : m_ID(id) { }
	ID(const char *id) : m_ID(id ? id : "") { }
	ID(const char * str, std::size_t len) : m_ID(str, len) { }
	template <typename T>
	static ID FromInt(const T &val)
	{
		static_assert(std::numeric_limits<T>::is_integer);
		typename mpt::make_le<T>::type valle;
		valle = val;
		return ID(mpt::byte_cast<const char*>(mpt::as_raw_memory(valle).data()), mpt::as_raw_memory(valle).size());
	}
#ifdef SSB_LOGGING
	bool IsPrintable() const;
	mpt::ustring AsString() const;
#endif
	std::size_t GetSize() const
	{
		return m_ID.size();
	}
	mpt::span<const char> AsSpan() const
	{
		return mpt::as_span(m_ID);
	}
	bool operator == (const ID &other) const { return m_ID == other.m_ID; }
	bool operator != (const ID &other) const { return m_ID != other.m_ID; }
};



class Ssb
{

protected:

	Ssb();

public:

	bool HasFailed() const
	{
		return (m_Status.level >= StatusLevel::Failure);
	}

protected:

	Status m_Status;

	uint32 m_nFixedEntrySize;			// Read/write: If > 0, data entries have given fixed size.

	std::streamoff m_posStart;					// Read/write: Stream position at the beginning of object.

	uint16 m_nIdbytes;					// Read/Write: Tells map ID entry size in bytes. If size is variable, value is IdSizeVariable.
	std::size_t m_nCounter;					// Read/write: Keeps count of entries written/read.

	std::bitset<RwfNumFlags> m_Flags;	// Read/write: Various flags.

protected:

	enum : uint8 { s_DefaultFlagbyte = 0 };
	static const char s_EntryID[3];

};



class SsbRead
	: public Ssb
{

public:

	using ReadIterator = std::vector<ReadEntry>::const_iterator;

	SsbRead(std::istream& iStrm);

	// Call this to begin reading: must be called before other read functions.
	void BeginRead(const ID &id, const uint64& nVersion);

	// After calling BeginRead(), this returns number of entries in the file.
	std::size_t GetNumEntries() const {return m_nReadEntrycount;}

	// Returns read iterator to the beginning of entries.
	// The behaviour of read iterators is undefined if map doesn't
	// contain entry ids or data begin positions.
	ReadIterator GetReadBegin();

	// Returns read iterator to the end(one past last) of entries.
	ReadIterator GetReadEnd();

	// Compares given id with read entry id 
	bool MatchesId(const ReadIterator& iter, const ID &id);

	uint64 GetReadVersion() {return m_nReadVersion;}

	// Read item using default read implementation.
	template <class T>
	bool ReadItem(T& obj, const ID &id) {return ReadItem(obj, id, srlztn::ReadItem<T>);}

	// Read item using given function.
	template <class T, class FuncObj>
	bool ReadItem(T& obj, const ID &id, FuncObj);

	// Read item using read iterator.
	template <class T>
	bool ReadIterItem(const ReadIterator& iter, T& obj) {return ReadIterItem(iter, obj, srlztn::ReadItem<T>);}
	template <class T, class FuncObj>
	bool ReadIterItem(const ReadIterator& iter, T& obj, FuncObj func);

private:

	// Reads map to cache.
	void CacheMap();

	// Searches for entry with given ID. If found, returns pointer to corresponding entry, else
	// returns nullptr.
	const ReadEntry* Find(const ID &id);

	// Called after reading an object.
	void OnReadEntry(const ReadEntry* pE, const ID &id, const std::streamoff& posReadBegin);

	void AddReadNote(const Status s);

#ifdef SSB_LOGGING
	// Called after reading entry. pRe is a pointer to associated map entry if exists.
	void LogReadEntry(const ReadEntry &pRe, const std::size_t nNum);
#endif

	void ResetReadstatus();

private:

	//  mapData is a cache that facilitates faster access to the stored data
	// without having to reparse on every access.
	//  Iterator invalidation in CacheMap() is not a problem because every code
	// path that ever returns an iterator into mapData does CacheMap exactly once
	// beforehand. Following calls use this already cached map. As the data is
	// immutable when reading, there is no need to ever invalidate the cache and
	// redo CacheMap().

	std::istream& iStrm;

	std::vector<char> m_Idarray;		// Read: Holds entry ids.

	std::vector<ReadEntry> mapData;		// Read: Contains map information.
	uint64 m_nReadVersion;				// Read: Version is placed here when reading.
	std::streamoff m_rposMapBegin;			// Read: If map exists, rpos of map begin, else m_rposEndofHdrData.
	std::streamoff m_posMapEnd;				// Read: If map exists, map end position, else pos of end of hdrData.
	std::streamoff m_posDataBegin;				// Read: Data begin position.
	std::streamoff m_rposEndofHdrData;		// Read: rpos of end of header data.
	std::size_t m_nReadEntrycount;			// Read: Number of entries.

	std::size_t m_nNextReadHint;			// Read: Hint where to start looking for the next read entry.

};



class SsbWrite
	: public Ssb
{

public:

	SsbWrite(std::ostream& oStrm);

	// Write header
	void BeginWrite(const ID &id, const uint64& nVersion);

	// Write item using default write implementation.
	template <class T>
	void WriteItem(const T& obj, const ID &id) {WriteItem(obj, id, &srlztn::WriteItem<T>);}

	// Write item using given function.
	template <class T, class FuncObj>
	void WriteItem(const T& obj, const ID &id, FuncObj);

	// Writes mapping.
	void FinishWrite();

private:

	// Called after writing an item.
	void OnWroteItem(const ID &id, const std::streamoff& posBeforeWrite);

	void AddWriteNote(const Status s);

#ifdef SSB_LOGGING
	void LogWriteEntry(const ID &id,
		const std::size_t nEntryNum,
		const std::size_t nBytecount,
		const std::streamoff rposStart);
#endif

	// Writes mapping item to mapstream.
	void WriteMapItem(const ID &id,
		const std::streamoff& rposDataStart,
		const std::size_t& nDatasize,
		const std::string &pszDesc);

	void ResetWritestatus()
	{
		m_Status = Status{};
	}

private:

	std::ostream& oStrm;

	std::streamoff m_posEntrycount;			// Write: Pos of entrycount field. 
	std::streamoff m_posMapPosField;			// Write: Pos of map position field.
	std::string m_MapStreamString;				// Write: Map stream string.

};


template <class T, class FuncObj>
void SsbWrite::WriteItem(const T& obj, const ID &id, FuncObj Func)
{
	const std::streamoff pos = static_cast<std::streamoff>(oStrm.tellp());
	Func(oStrm, obj);
	OnWroteItem(id, pos);
}

template <class T, class FuncObj>
bool SsbRead::ReadItem(T& obj, const ID &id, FuncObj Func)
{
	const ReadEntry* pE = Find(id);
	const std::streamoff pos = static_cast<std::streamoff>(iStrm.tellg());
	const bool entryFound = (pE || !m_Flags[RwfRMapHasId]);
	if(entryFound)
	{
		Func(iStrm, obj, (pE) ? (pE->nSize) : invalidDatasize);
	}
	OnReadEntry(pE, id, pos);
	return entryFound;
}


template <class T, class FuncObj>
bool SsbRead::ReadIterItem(const ReadIterator& iter, T& obj, FuncObj func)
{
	iStrm.clear();
	if (iter->rposStart != 0)
		iStrm.seekg(m_posStart + iter->rposStart);
	const std::streamoff pos = static_cast<std::streamoff>(iStrm.tellg());
	func(iStrm, obj, iter->nSize);
	OnReadEntry(&(*iter), ID(&m_Idarray[iter->nIdpos], iter->nIdLength), pos);
	return true;
}


inline bool SsbRead::MatchesId(const ReadIterator& iter, const ID &id)
{
	if(iter->nIdpos >= m_Idarray.size())
	{
		return false;
	}
	return (id == ID(&m_Idarray[iter->nIdpos], iter->nIdLength));
}


inline SsbRead::ReadIterator SsbRead::GetReadBegin()
{
	MPT_ASSERT(m_Flags[RwfRMapHasId] && (m_Flags[RwfRMapHasStartpos] || m_Flags[RwfRMapHasSize] || m_nFixedEntrySize > 0));
	if(!m_Flags[RwfRMapCached])
		CacheMap();
	return mapData.begin();
}


inline SsbRead::ReadIterator SsbRead::GetReadEnd()
{
	if(!m_Flags[RwfRMapCached])
		CacheMap();
	return mapData.end();
}


template <class T>
struct VectorWriter
{
	VectorWriter(size_t nCount) : m_nCount(nCount) {}
	void operator()(std::ostream &oStrm, const std::vector<T> &vec)
	{
		for(size_t i = 0; i < m_nCount; i++)
		{
			Binarywrite(oStrm, vec[i]);
		}
	}
	size_t m_nCount;
};

template <class T>
struct VectorReader
{
	VectorReader(size_t nCount) : m_nCount(nCount) {}
	void operator()(std::istream& iStrm, std::vector<T> &vec, const size_t)
	{
		vec.resize(m_nCount);
		for(std::size_t i = 0; i < m_nCount; ++i)
		{
			Binaryread(iStrm, vec[i]);
		}
	}
	size_t m_nCount;
};

template <class T>
struct ArrayReader
{
	ArrayReader(size_t nCount) : m_nCount(nCount) {}
	void operator()(std::istream& iStrm, T* pData, const size_t)
	{
		for(std::size_t i=0; i<m_nCount; ++i)
		{
			Binaryread(iStrm, pData[i]);
		}
	} 
	size_t m_nCount;
};



} //namespace srlztn.


OPENMPT_NAMESPACE_END
