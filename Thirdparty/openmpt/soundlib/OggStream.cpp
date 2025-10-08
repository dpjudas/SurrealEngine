/*
 * OggStream.cpp
 * -------------
 * Purpose: Basic Ogg stream parsing functionality
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "stdafx.h"
#include "OggStream.h"
#include "mpt/crc/crc.hpp"
#include "mpt/io_read/filereader.hpp"
#include "../common/FileReader.h"


OPENMPT_NAMESPACE_BEGIN


namespace Ogg
{


uint16 PageInfo::GetPagePhysicalSize() const
{
	uint16 size = 0;
	size += sizeof(PageHeader);
	size += header.page_segments;
	for(uint8 segment = 0; segment < header.page_segments; ++segment)
	{
		size += segment_table[segment];
	}
	return size;
}


uint16 PageInfo::GetPageHeaderSize() const
{
	uint16 size = 0;
	size += sizeof(PageHeader);
	size += header.page_segments;
	return size;
}


uint16 PageInfo::GetPageDataSize() const
{
	uint16 size = 0;
	for(uint8 segment = 0; segment < header.page_segments; ++segment)
	{
		size += segment_table[segment];
	}
	return size;
}


bool AdvanceToPageMagic(FileCursor &file)
{
	while(mpt::FR::CanRead(file, 4))
	{
		if(mpt::FR::ReadMagic(file, "OggS"))
		{
			mpt::FR::SkipBack(file, 4);
			return true;
		}
		mpt::FR::Skip(file, 1);
	}
	return false;
}


bool ReadPage(FileCursor &file, PageInfo &pageInfo, std::vector<uint8> *pageData)
{
	pageInfo = PageInfo();
	if(pageData)
	{
		(*pageData).clear();
	}
	if(!mpt::FR::ReadMagic(file, "OggS"))
	{
		return false;
	}
	mpt::FR::SkipBack(file, 4);
	FileCursor filePageCursor = file; // do not modify original file read position
	if(!mpt::FR::ReadStruct(filePageCursor, pageInfo.header))
	{
		return false;
	}
	if(!mpt::FR::CanRead(filePageCursor, pageInfo.header.page_segments))
	{
		return false;
	}
	uint16 pageDataSize = 0;
	for(uint8 segment = 0; segment < pageInfo.header.page_segments; ++segment)
	{
		pageInfo.segment_table[segment] = mpt::FR::ReadIntLE<uint8>(filePageCursor);
		pageDataSize += pageInfo.segment_table[segment];
	}
	if(!mpt::FR::CanRead(filePageCursor, pageDataSize))
	{
		return false;
	}
	if(pageData)
	{
		mpt::FR::ReadVector(filePageCursor , *pageData, pageDataSize);
	} else
	{
		mpt::FR::Skip(filePageCursor, pageDataSize);
	}
	mpt::FR::SkipBack(filePageCursor, pageInfo.GetPagePhysicalSize());
	{
		mpt::crc32_ogg calculatedCRC;
		uint8 rawHeader[sizeof(PageHeader)];
		MemsetZero(rawHeader);
		mpt::FR::ReadArray(filePageCursor, rawHeader);
		std::memset(rawHeader + 22, 0, 4); // clear out old crc
		calculatedCRC.process(rawHeader, rawHeader + sizeof(rawHeader));
		mpt::FR::Skip(filePageCursor, pageInfo.header.page_segments);
		calculatedCRC.process(pageInfo.segment_table, pageInfo.segment_table + pageInfo.header.page_segments);
		if(pageData)
		{
			mpt::FR::Skip(filePageCursor, pageDataSize);
			calculatedCRC.process(*pageData);
		} else
		{
			FileCursor pageDataReader = mpt::FR::ReadChunk(filePageCursor, pageDataSize);
			auto pageDataView = pageDataReader.GetPinnedView();
			calculatedCRC.process(pageDataView.GetSpan());
		}
		if(calculatedCRC != pageInfo.header.CRC_checksum)
		{
			return false;
		}
	}
	mpt::FR::Skip(file, pageInfo.GetPagePhysicalSize());
	return true;
}


bool ReadPage(FileCursor &file, PageInfo &pageInfo, std::vector<uint8> &pageData)
{
	return ReadPage(file, pageInfo, &pageData);
}


bool ReadPage(FileCursor &file)
{
	PageInfo pageInfo;
	return ReadPage(file, pageInfo);
}


bool ReadPageAndSkipJunk(FileCursor &file, PageInfo &pageInfo, std::vector<uint8> &pageData)
{
	pageInfo = PageInfo();
	pageData.clear();
	while(AdvanceToPageMagic(file))
	{
		if(ReadPage(file, pageInfo, pageData))
		{
			return true;
		} else
		{
			pageInfo = PageInfo();
			pageData.clear();
		}
		mpt::FR::Skip(file, 4);
	}
	return false;
}


bool UpdatePageCRC(PageInfo &pageInfo, const std::vector<uint8> &pageData)
{
	if(pageData.size() != pageInfo.GetPageDataSize())
	{
		return false;
	}
	mpt::crc32_ogg crc;
	pageInfo.header.CRC_checksum = 0;
	std::byte rawHeader[sizeof(PageHeader)] = {};
	std::memcpy(rawHeader, &pageInfo.header, sizeof(PageHeader));
	crc.process(rawHeader, rawHeader + sizeof(PageHeader));
	crc.process(pageInfo.segment_table, pageInfo.segment_table + pageInfo.header.page_segments);
	crc.process(pageData);
	pageInfo.header.CRC_checksum = crc;
	return true;
}


} // namespace Ogg


OPENMPT_NAMESPACE_END
