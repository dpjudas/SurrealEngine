/*
 * serialization_utils.cpp
 * -----------------------
 * Purpose: Serializing data to and from MPTM files.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"

#include "serialization_utils.h"

#include "mpt/base/utility.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"

#include <array>
#include <istream>
#include <ostream>
#include <sstream>

#include "misc_util.h"


OPENMPT_NAMESPACE_BEGIN


namespace srlztn
{


static const uint8 HeaderId_FlagByte = 0;

// Indexing starts from 0.
static inline bool Testbit(uint8 val, uint8 bitindex) {return ((val & (1 << bitindex)) != 0);}

static inline void Setbit(uint8& val, uint8 bitindex, bool newval)
{
	if(newval)
	{
		val |= static_cast<uint8>(1u << bitindex);
	} else
	{
		val &= static_cast<uint8>(~(1u << bitindex));
	}
}


#ifdef SSB_LOGGING
bool ID::IsPrintable() const
{
	for(std::size_t i = 0; i < m_ID.length(); ++i)
	{
		if(m_ID[i] <= 0 || isprint(static_cast<unsigned char>(m_ID[i])) == 0)
		{
			return false;
		}
	}
	return true;
}
#endif


//Format: First bit tells whether the size indicator is 1 or 2 bytes.
static void WriteAdaptive12String(std::ostream& oStrm, const std::string& str)
{
	uint16 s = static_cast<uint16>(str.size());
	LimitMax(s, uint16(std::numeric_limits<uint16>::max() / 2));
	mpt::IO::WriteAdaptiveInt16LE(oStrm, s);
	oStrm.write(str.c_str(), s);
}


void WriteItemString(std::ostream& oStrm, const std::string &str)
{
	uint32 id = static_cast<uint32>(std::min(str.size(), static_cast<std::size_t>((std::numeric_limits<uint32>::max() >> 4)))) << 4;
	id |= 12; // 12 == 1100b
	Binarywrite<uint32>(oStrm, id);
	id >>= 4;
	if(id > 0)
		oStrm.write(str.data(), id);
}


void ReadItemString(std::istream& iStrm, std::string& str, const std::size_t)
{
	// bits 0,1: Bytes per char type: 1,2,3,4.
	// bits 2,3: Bytes in size indicator, 1,2,3,4
	uint32 id = 0;
	Binaryread(iStrm, id, 1);
	const uint8 nSizeBytes = (id & 12) >> 2; // 12 == 1100b
	if (nSizeBytes > 0)
	{
		uint8 bytes = std::min(uint8(3), nSizeBytes);
		uint8 v2 = 0;
		uint8 v3 = 0;
		uint8 v4 = 0;
		if(bytes >= 1) Binaryread(iStrm, v2);
		if(bytes >= 2) Binaryread(iStrm, v3);
		if(bytes >= 3) Binaryread(iStrm, v4);
		id &= 0xff;
		id |= (v2 << 8) | (v3 << 16) | (v4 << 24);
	}
	// Limit to 1 MB.
	str.resize(std::min(id >> 4, uint32(1000000)));
	for(size_t i = 0; i < str.size(); i++)
		iStrm.read(&str[i], 1);

	id = (id >> 4) - static_cast<uint32>(str.size());
	if(id > 0)
		iStrm.ignore(id);
}


#ifdef SSB_LOGGING
mpt::ustring ID::AsString() const
{
	if(IsPrintable())
	{
		return mpt::ToUnicode(mpt::Charset::ISO8859_1, m_ID);
	}
	if(m_ID.length() > 8)
	{
		return mpt::ustring();
	}
	uint64le val;
	val.set(0);
	std::memcpy(&val, m_ID.data(), m_ID.length());
	return mpt::ufmt::val(val);
}
#endif


const char Ssb::s_EntryID[3] = {'2','2','8'};


Ssb::Ssb()
	: m_Status(Status{})
	, m_nFixedEntrySize(0)
	, m_posStart(0)
	, m_nIdbytes(IdSizeVariable)
	, m_nCounter(0)
	, m_Flags((1 << RwfWMapStartPosEntry) + (1 << RwfWMapSizeEntry) + (1 << RwfWVersionNum))
{
	return;
}


SsbWrite::SsbWrite(std::ostream& os)
	: oStrm(os)
	, m_posEntrycount(0)
	, m_posMapPosField(0)
{
	return;
}


SsbRead::SsbRead(std::istream& is)
	: iStrm(is)
	, m_nReadVersion(0)
	, m_rposMapBegin(0)
	, m_posMapEnd(0)
	, m_posDataBegin(0)
	, m_rposEndofHdrData(0)
	, m_nReadEntrycount(0)
	, m_nNextReadHint(0)
{
	return;
}


void SsbWrite::AddWriteNote(const Status s)
{
	m_Status.level = static_cast<StatusLevel>(mpt::to_underlying(m_Status.level) | mpt::to_underlying(s.level));
	m_Status.messages = static_cast<StatusMessages>(mpt::to_underlying(m_Status.messages) | mpt::to_underlying(s.messages));
	SSB_LOG(MPT_UFORMAT("{}: 0x{} 0x{}")(U_("Write note: "), mpt::ufmt::hex(mpt::to_underlying(s.level)), mpt::ufmt::hex(mpt::to_underlying(s.messages))));
}

void SsbRead::AddReadNote(const Status s)
{
	m_Status.level = static_cast<StatusLevel>(mpt::to_underlying(m_Status.level) | mpt::to_underlying(s.level));
	m_Status.messages = static_cast<StatusMessages>(mpt::to_underlying(m_Status.messages) | mpt::to_underlying(s.messages));
	SSB_LOG(MPT_UFORMAT("{}: 0x{} 0x{}")(U_("Read note: "), mpt::ufmt::hex(mpt::to_underlying(s.level)), mpt::ufmt::hex(mpt::to_underlying(s.messages))));
}

#ifdef SSB_LOGGING
void SsbRead::LogReadEntry(const ReadEntry &pRe, const std::size_t nNum)
{
	SSB_LOG(MPT_UFORMAT("Read entry: {{num, id, rpos, size, desc}} = {{{}, {}, {}, {}, {}}}")(
				 nNum,
				 (pRe.nIdLength < 30 && m_Idarray.size() > 0) ?  ID(&m_Idarray[pRe.nIdpos], pRe.nIdLength).AsString() : U_(""),
				 pRe.rposStart,
				 (pRe.nSize != invalidDatasize) ? mpt::ufmt::val(pRe.nSize) : U_(""),
				 U_("")));
}
#endif

#ifdef SSB_LOGGING
// Called after writing an entry.
void SsbWrite::LogWriteEntry(const ID &id, const std::size_t nEntryNum, const std::size_t nBytecount, const std::streamoff rposStart)
{
	SSB_LOG(MPT_UFORMAT("Wrote entry: {{num, id, rpos, size}} = {{{}, {}, {}, {}}}")(nEntryNum, id.AsString(), rposStart, nBytecount));
}
#endif


void SsbRead::ResetReadstatus()
{
	m_Status = Status{};
	m_Idarray.reserve(32);
	m_Idarray.push_back(0);
}


void SsbWrite::WriteMapItem(const ID &id,
						const std::streamoff& rposDataStart,
						const std::size_t& nDatasize,
						const std::string &pszDesc)
{
	SSB_LOG(MPT_UFORMAT("Writing map entry: id={}, rpos={}, size={}")(
					(id.GetSize() > 0) ? id.AsString() : U_(""),
					rposDataStart,
					nDatasize));

	std::ostringstream mapStream;

	if(m_nIdbytes > 0)
	{
		if (m_nIdbytes != IdSizeVariable && id.GetSize() != m_nIdbytes)
			{ AddWriteNote(SNW_CHANGING_IDSIZE_WITH_FIXED_IDSIZESETTING); return; }

		if (m_nIdbytes == IdSizeVariable) //Variablesize ID?
			mpt::IO::WriteAdaptiveInt16LE(mapStream, static_cast<uint16>(id.GetSize()));

		if(id.GetSize() > 0)
			mapStream.write(id.AsSpan().data(), id.AsSpan().size());
	}

	if(m_Flags[RwfWMapStartPosEntry]) // Startpos
		mpt::IO::WriteAdaptiveInt64LE(mapStream, rposDataStart);
	if(m_Flags[RwfWMapSizeEntry]) // Entrysize
		mpt::IO::WriteAdaptiveInt64LE(mapStream, nDatasize);
	if(m_Flags[RwfWMapDescEntry]) // Entry descriptions
		WriteAdaptive12String(mapStream, pszDesc);

	m_MapStreamString.append(mapStream.str());

}


void SsbWrite::BeginWrite(const ID &id, const uint64& nVersion)
{
	SSB_LOG(MPT_UFORMAT("Write header with ID = {}")(id.AsString()));

	ResetWritestatus();

	if(!oStrm.good())
		{ AddWriteNote(SNRW_BADGIVEN_STREAM); return; }

	// Start bytes.
	oStrm.write(s_EntryID, sizeof(s_EntryID));

	m_posStart = static_cast<std::streamoff>(oStrm.tellp()) - static_cast<std::streamoff>(sizeof(s_EntryID));
	
	// Object ID.
	{
		uint8 idsize = static_cast<uint8>(id.GetSize());
		Binarywrite<uint8>(oStrm, idsize);
		if(idsize > 0) oStrm.write(id.AsSpan().data(), id.AsSpan().size());
	}

	// Form header.
	uint8 header = 0;

	m_Flags[RwfWMapStartPosEntry] = m_Flags[RwfWMapStartPosEntry] && m_nFixedEntrySize == 0;
	m_Flags[RwfWMapSizeEntry] = m_Flags[RwfWMapSizeEntry] && m_nFixedEntrySize == 0;

	header = (m_nIdbytes != 4) ? (m_nIdbytes & 3) : 3;	//0,1 : Bytes per IDtype, 0,1,2,4
	Setbit(header, 2, m_Flags[RwfWMapStartPosEntry]);	//2   : Startpos in map?
	Setbit(header, 3, m_Flags[RwfWMapSizeEntry]);		//3   : Datasize in map?
	Setbit(header, 4, m_Flags[RwfWVersionNum]);			//4   : Version numeric field?
	Setbit(header, 7, m_Flags[RwfWMapDescEntry]);		//7   : Entrydescriptions in map?

	// Write header
	Binarywrite<uint8>(oStrm, header);

	// Additional options.
	uint8 tempU8 = 0;
	Setbit(tempU8, 0, (m_nIdbytes == IdSizeVariable) || (m_nIdbytes == 3) || (m_nIdbytes > 4));
	Setbit(tempU8, 1, m_nFixedEntrySize != 0);
	
	const uint8 flags = tempU8;
	if(flags != s_DefaultFlagbyte)
	{
		mpt::IO::WriteAdaptiveInt32LE(oStrm, 2); //Headersize - now it is 2.
		Binarywrite<uint8>(oStrm, HeaderId_FlagByte);
		Binarywrite<uint8>(oStrm, flags);
	}
	else
		mpt::IO::WriteAdaptiveInt32LE(oStrm, 0);

	if(Testbit(header, 4)) // Version(numeric)?
		mpt::IO::WriteAdaptiveInt64LE(oStrm, nVersion);

	if(Testbit(flags, 0)) // Custom IDbytecount?
	{
		uint8 n = (m_nIdbytes == IdSizeVariable) ? 1 : static_cast<uint8>((m_nIdbytes << 1));
		Binarywrite<uint8>(oStrm, n);
	}

	if(Testbit(flags, 1)) // Fixedsize entries?
		mpt::IO::WriteAdaptiveInt32LE(oStrm, m_nFixedEntrySize);

	//Entrycount. Reserve two bytes(max uint16_max / 4 entries), actual value is written after writing data.
	m_posEntrycount = static_cast<std::streamoff>(oStrm.tellp());
	Binarywrite<uint16>(oStrm, 0);

	m_Flags[RwfRwHasMap] = (m_nIdbytes != 0 || m_Flags[RwfWMapStartPosEntry] || m_Flags[RwfWMapSizeEntry] || m_Flags[RwfWMapDescEntry]);

	m_posMapPosField = static_cast<std::streamoff>(oStrm.tellp());
	if(m_Flags[RwfRwHasMap]) // Mapping begin pos(reserve space - actual value is written after writing data)
		Binarywrite<uint64>(oStrm, 0);
}


void SsbRead::OnReadEntry(const ReadEntry* pE, const ID &id, const std::streamoff& posReadBegin)
{
#ifdef SSB_LOGGING
	if(pE)
	{
		LogReadEntry(*pE, m_nCounter);
	} else if(!m_Flags[RwfRMapHasId]) // Not ID's in map.
	{
		ReadEntry e;
		e.rposStart = posReadBegin - m_posStart;
		e.nSize = mpt::saturate_cast<std::size_t>(static_cast<std::streamoff>(static_cast<std::streamoff>(iStrm.tellg()) - posReadBegin));
		LogReadEntry(e, m_nCounter);
	} else // Entry not found.
	{
		SSB_LOG(MPT_UFORMAT("No entry with id {} found.")(id.AsString()));
	}
#else
	MPT_UNREFERENCED_PARAMETER(id);
	MPT_UNREFERENCED_PARAMETER(posReadBegin);
#endif
	const bool entryFound = (pE || !m_Flags[RwfRMapHasId]);
	if(entryFound)
	{
		m_nCounter++;
	}
}


void SsbWrite::OnWroteItem(const ID &id, const std::streamoff& posBeforeWrite)
{
	const std::streamoff nRawEntrySize = static_cast<std::streamoff>(oStrm.tellp()) - posBeforeWrite;

	MPT_MAYBE_CONSTANT_IF(!mpt::in_range<std::size_t>(nRawEntrySize))
	{
		AddWriteNote(SNW_INSUFFICIENT_DATASIZETYPE);
		return;
	}

	if(m_Flags[RwfRMapHasSize] && (static_cast<uint64>(nRawEntrySize) > (std::numeric_limits<std::size_t>::max() >> 2)))
		{ AddWriteNote(SNW_DATASIZETYPE_OVERFLOW); return; }

	std::size_t nEntrySize = static_cast<std::size_t>(nRawEntrySize);

	// Handle fixed size entries:
	if (m_nFixedEntrySize > 0)
	{
		if(nEntrySize <= m_nFixedEntrySize)
		{
			for(uint32 i = 0; i<m_nFixedEntrySize-nEntrySize; i++)
				oStrm.put(0);
			nEntrySize = m_nFixedEntrySize;
		}
		else
			{ AddWriteNote(SNW_INSUFFICIENT_FIXEDSIZE); return; }
	}
	if(m_Flags[RwfRwHasMap])
		WriteMapItem(id, posBeforeWrite - m_posStart, nEntrySize, "");

#ifdef SSB_LOGGING
	LogWriteEntry(id, m_nCounter, nEntrySize, posBeforeWrite - m_posStart);
#endif

	m_nCounter++;
	if(m_nCounter >= static_cast<uint16>(std::numeric_limits<uint16>::max() >> 2))
	{
		FinishWrite();
		AddWriteNote(SNW_MAX_WRITE_COUNT_REACHED);
	}
}


void SsbRead::BeginRead(const ID &id, const uint64& nVersion)
{
	SSB_LOG(MPT_UFORMAT("Read header with expected ID = {}")(id.AsString()));

	ResetReadstatus();

	if (!iStrm.good())
		{ AddReadNote(SNRW_BADGIVEN_STREAM); return; }

	m_posStart = static_cast<std::streamoff>(iStrm.tellg());

	// Start bytes.
	{
		char temp[sizeof(s_EntryID)];
		ArrayReader<char>(sizeof(s_EntryID))(iStrm, temp, sizeof(s_EntryID));
		if(std::memcmp(temp, s_EntryID, sizeof(s_EntryID)))
		{
			AddReadNote(SNR_STARTBYTE_MISMATCH);
			return;
		}
	}
	
	// Compare IDs.
	uint8 storedIdLen = 0;
	Binaryread<uint8>(iStrm, storedIdLen);
	std::array<char, 256> storedIdBuf;
	storedIdBuf = {};
	if(storedIdLen > 0)
	{
		iStrm.read(storedIdBuf.data(), storedIdLen);
	}
	if(!(id == ID(storedIdBuf.data(), storedIdLen)))
	{
		AddReadNote(SNR_OBJECTCLASS_IDMISMATCH);
	}
	if(HasFailed())
	{
		SSB_LOG(U_("ID mismatch, terminating read."));
		return;
	}

	SSB_LOG(U_("ID match, continuing reading."));
	
	// Header
	uint8 tempU8;
	Binaryread<uint8>(iStrm, tempU8);
	const uint8 header = tempU8;
	m_nIdbytes = ((header & 3) == 3) ? 4 : (header & 3);
	if (Testbit(header, 6))
		m_Flags[RwfRTwoBytesDescChar] = true;

	// Read headerdata size
	uint32 tempU32 = 0;
	mpt::IO::ReadAdaptiveInt32LE(iStrm, tempU32);
	const uint32 headerdatasize = tempU32;

	// If headerdatasize != 0, read known headerdata and ignore rest.
	uint8 flagbyte = s_DefaultFlagbyte;
	if(headerdatasize >= 2)
	{
		Binaryread<uint8>(iStrm, tempU8);
		if(tempU8 == HeaderId_FlagByte)
			Binaryread<uint8>(iStrm, flagbyte);

		iStrm.ignore( (tempU8 == HeaderId_FlagByte) ? headerdatasize - 2 : headerdatasize - 1);
	}

	uint64 tempU64 = 0;

	// Read version numeric if available.
	if (Testbit(header, 4))
	{
		mpt::IO::ReadAdaptiveInt64LE(iStrm, tempU64);
		m_nReadVersion = tempU64;
		if(tempU64 > nVersion)
			AddReadNote(SNR_LOADING_OBJECT_WITH_LARGER_VERSION);
	}

	if (Testbit(header, 5))
	{
		Binaryread<uint8>(iStrm, tempU8);
		iStrm.ignore(tempU8);
	}

	if(Testbit(flagbyte, 0)) // Custom ID?
	{
		Binaryread<uint8>(iStrm, tempU8);
		if ((tempU8 & 1) != 0)
			m_nIdbytes = IdSizeVariable;
		else
			m_nIdbytes = (tempU8 >> 1);
		if(m_nIdbytes == 0)
			AddReadNote(SNR_NO_ENTRYIDS_WITH_CUSTOMID_DEFINED);
	}

	m_nFixedEntrySize = 0;
	if(Testbit(flagbyte, 1)) // Fixedsize entries?
		mpt::IO::ReadAdaptiveInt32LE(iStrm, m_nFixedEntrySize);

	m_Flags[RwfRMapHasStartpos] = Testbit(header, 2);
	m_Flags[RwfRMapHasSize] = Testbit(header, 3);
	m_Flags[RwfRMapHasId] = (m_nIdbytes > 0);
	m_Flags[RwfRMapHasDesc] = Testbit(header, 7);
	m_Flags[RwfRwHasMap] = m_Flags[RwfRMapHasId] || m_Flags[RwfRMapHasStartpos] || m_Flags[RwfRMapHasSize] || m_Flags[RwfRMapHasDesc];
	
	if(!m_Flags[RwfRwHasMap])
	{
		SSB_LOG(U_("No map in the file."));
	}

	if (Testbit(flagbyte, 2)) // Object description?
	{
		uint16 size = 0;
		mpt::IO::ReadAdaptiveInt16LE(iStrm, size);
		iStrm.ignore(size * (m_Flags[RwfRTwoBytesDescChar] ? 2 : 1));
	}

	if(Testbit(flagbyte, 3))
		iStrm.ignore(5);

	// Read entrycount
	mpt::IO::ReadAdaptiveInt64LE(iStrm, tempU64);
	if(tempU64 > 16000)
		// The current code can only write 16383 entries because it uses a Adaptive64LE with a fixed size=2
		// Additionally, 16000 is an arbitrary limit to avoid an out-of-memory DoS when caching the map.
		{ AddReadNote(SNR_TOO_MANY_ENTRIES_TO_READ); return; }

	m_nReadEntrycount = static_cast<std::size_t>(tempU64);
	if(m_nReadEntrycount == 0)
		AddReadNote(SNR_ZEROENTRYCOUNT);

	// Read map rpos if map exists.
	if(m_Flags[RwfRwHasMap])
	{
		mpt::IO::ReadAdaptiveInt64LE(iStrm, tempU64);
		if(!mpt::in_range<std::streamoff>(tempU64))
			{ AddReadNote(SNR_INSUFFICIENT_STREAM_OFFTYPE); return; }
	}

	const std::streamoff rawEndOfHdrData = static_cast<std::streamoff>(iStrm.tellg()) - m_posStart;

	if(rawEndOfHdrData < 0)
	{
		AddReadNote(SNR_INSUFFICIENT_RPOSTYPE);
		return;
	}

	m_rposEndofHdrData = rawEndOfHdrData;
	m_rposMapBegin = (m_Flags[RwfRwHasMap]) ? static_cast<std::streamoff>(tempU64) : m_rposEndofHdrData;

	if(!m_Flags[RwfRwHasMap])
		m_posMapEnd = m_posStart + m_rposEndofHdrData;

	m_Flags[RwfRHeaderIsRead] = true;
}


void SsbRead::CacheMap()
{
	if(m_Flags[RwfRwHasMap] || m_nFixedEntrySize > 0)
	{
		iStrm.seekg(m_posStart + m_rposMapBegin, std::ios::beg);

		if(iStrm.fail())
			{ AddReadNote(SNR_BADSTREAM_AFTER_MAPHEADERSEEK); return; }

		SSB_LOG(MPT_UFORMAT("Reading map from rpos: {}")(m_rposMapBegin));

		mapData.resize(m_nReadEntrycount);
		m_Idarray.reserve(m_nReadEntrycount * 4);

		//Read map
		for(std::size_t i = 0; i<m_nReadEntrycount; i++)
		{
			if(iStrm.fail())
				{ AddReadNote(SNR_BADSTREAM_AT_MAP_READ); return; }

			// Read ID.
			uint16 nIdsize = m_nIdbytes;
			if(nIdsize == IdSizeVariable) //Variablesize ID
				mpt::IO::ReadAdaptiveInt16LE(iStrm, nIdsize);
			const size_t nOldEnd = m_Idarray.size();
			if (nIdsize > 0 && (Util::MaxValueOfType(nOldEnd) - nOldEnd >= nIdsize))
			{
				m_Idarray.resize(nOldEnd + nIdsize);
				iStrm.read(&m_Idarray[nOldEnd], nIdsize);
			}
			mapData[i].nIdLength = nIdsize;
			mapData[i].nIdpos = nOldEnd;

			// Read position.
			if(m_Flags[RwfRMapHasStartpos])
			{
				uint64 tempU64;
				mpt::IO::ReadAdaptiveInt64LE(iStrm, tempU64);
				if(!mpt::in_range<std::streamoff>(tempU64))
					{ AddReadNote(SNR_INSUFFICIENT_STREAM_OFFTYPE); return; }
				mapData[i].rposStart = static_cast<std::streamoff>(tempU64);
			}

			// Read entry size.
			if (m_nFixedEntrySize > 0)
				mapData[i].nSize = m_nFixedEntrySize;
			else if(m_Flags[RwfRMapHasSize]) // Map has datasize field.
			{
				uint64 tempU64;
				mpt::IO::ReadAdaptiveInt64LE(iStrm, tempU64);
				if(!mpt::in_range<std::streamoff>(tempU64))
					{ AddReadNote(SNR_INSUFFICIENT_STREAM_OFFTYPE); return; }
				mapData[i].nSize = static_cast<std::size_t>(tempU64);
			}

			// If there's no entry startpos in map, count start pos from datasizes.
			// Here readentry.rposStart is set to relative position from databegin.
			if(mapData[i].nSize != invalidDatasize && !m_Flags[RwfRMapHasStartpos])
				mapData[i].rposStart = (i > 0) ? mapData[i-1].rposStart + mapData[i-1].nSize : 0;

			if(m_Flags[RwfRMapHasDesc]) // Map has entrydescriptions?
			{
				uint16 size = 0;
				mpt::IO::ReadAdaptiveInt16LE(iStrm, size);
				if(m_Flags[RwfRTwoBytesDescChar])
					iStrm.ignore(size * 2);
				else
					iStrm.ignore(size);
			}
		}
		m_posMapEnd = static_cast<std::streamoff>(iStrm.tellg());
		SSB_LOG(MPT_UFORMAT("End of map(rpos): {}")(m_posMapEnd - m_posStart));
	}

	m_Flags[RwfRMapCached] = true;
	m_posDataBegin = (m_rposMapBegin == m_rposEndofHdrData) ? m_posMapEnd : m_posStart + m_rposEndofHdrData;
	iStrm.seekg(m_posDataBegin, std::ios::beg);

	// If there are no positions in the map but there are entry sizes, rposStart will
	// be relative to data start. Now that posDataBegin is known, make them relative to 
	// startpos.
	if(!m_Flags[RwfRMapHasStartpos] && (m_Flags[RwfRMapHasSize] || m_nFixedEntrySize > 0))
	{
		const std::streamoff offset = m_posDataBegin - m_posStart;
		for(size_t i = 0; i < m_nReadEntrycount; i++)
			mapData[i].rposStart += offset;
	}
}


const ReadEntry* SsbRead::Find(const ID &id)
{
	iStrm.clear();
	if(!m_Flags[RwfRMapCached])
		CacheMap();
	
	if(m_nFixedEntrySize > 0 && !m_Flags[RwfRMapHasStartpos] && !m_Flags[RwfRMapHasSize])
		iStrm.seekg(m_posDataBegin + static_cast<std::streamoff>(m_nFixedEntrySize * m_nCounter), std::ios::beg);

	if(m_Flags[RwfRMapHasId])
	{
		const size_t nEntries = mapData.size();
		for(size_t i0 = 0; i0 < nEntries; i0++)
		{
			const size_t i = (i0 + m_nNextReadHint) % nEntries;
			if(mapData[i].nIdpos < m_Idarray.size() && id == ID(&m_Idarray[mapData[i].nIdpos], mapData[i].nIdLength))
			{
				m_nNextReadHint = (i + 1) % nEntries;
				if (mapData[i].rposStart != 0)
					iStrm.seekg(m_posStart + mapData[i].rposStart, std::ios::beg);
				return &mapData[i];
			}
		}
	}
	return nullptr;
}


void SsbWrite::FinishWrite()
{
	const std::streamoff posDataEnd = static_cast<std::streamoff>(oStrm.tellp());
		
	std::streamoff posMapStart = static_cast<std::streamoff>(oStrm.tellp());

	SSB_LOG(MPT_UFORMAT("Writing map to rpos: {}")(posMapStart - m_posStart));

	if(m_Flags[RwfRwHasMap]) // Write map
	{
		oStrm.write(m_MapStreamString.c_str(), m_MapStreamString.length());
	}

	const std::streamoff posMapEnd = static_cast<std::streamoff>(oStrm.tellp());
	
	// Write entry count.
	oStrm.seekp(m_posEntrycount, std::ios::beg);

	// Write a fixed size=2 Adaptive64LE because space for this value has already been reserved berforehand.
	mpt::IO::WriteAdaptiveInt64LE(oStrm, m_nCounter, 2);

	if(m_Flags[RwfRwHasMap])
	{	// Write map start position.
		oStrm.seekp(m_posMapPosField, std::ios::beg);
		const uint64 rposMap = posMapStart - m_posStart;

		// Write a fixed size=8 Adaptive64LE because space for this value has already been reserved berforehand.
		mpt::IO::WriteAdaptiveInt64LE(oStrm, rposMap, 8);

	}

	// Seek to end.
	oStrm.seekp(std::max(posMapEnd, posDataEnd), std::ios::beg);

	SSB_LOG(MPT_UFORMAT("End of stream(rpos): {}")(static_cast<std::streamoff>(oStrm.tellp()) - m_posStart));
}

} // namespace srlztn 


OPENMPT_NAMESPACE_END
