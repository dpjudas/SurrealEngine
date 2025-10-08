/* Copyright (C) Teemu Suutari */

#include "LZW5Decompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool LZW5Decompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("LZW5");
}

std::shared_ptr<XPKDecompressor> LZW5Decompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<LZW5Decompressor>(hdr,recursionLevel,packedData,state,verify);
}

LZW5Decompressor::LZW5Decompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
}

const std::string &LZW5Decompressor::getSubName() const noexcept
{
	static std::string name{"XPK-LZW5: LZW5 CyberYAFA compressor"};
	return name;
}

void LZW5Decompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,0,_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto read2Bits=[&]()->uint32_t
	{
		return bitReader.readBitsBE32(2U);
	};
	auto readByte=[&]()->uint8_t
	{
		return inputStream.readByte();
	};

	ForwardOutputStream outputStream{rawData,0,rawData.size()};

	while (!outputStream.eof())
	{
		uint32_t distance,count;

		auto readld=[&]()->uint32_t
		{
			uint32_t ret{uint32_t(readByte())<<8};
			ret|=readByte();
			if (!ret)
				throw Decompressor::DecompressionError();
			return ret;
		};

		switch (read2Bits())
		{
			case 0:
			outputStream.writeByte(readByte());
			break;
			
			case 1U:
			distance=readld();
			count=(distance&3U)+2U;
			distance=0x4000-(distance>>2U);
			outputStream.copy(distance,count);
			break;

			case 2U:
			distance=readld();
			count=(distance&15U)+2U;
			distance=0x1000U-(distance>>4U);
			outputStream.copy(distance,count);
			break;

			case 3U:
			distance=readld();
			count=uint32_t(readByte())+3U;
			distance=0x10000U-distance;
			outputStream.copy(distance,count);
			break;
			
			default:
			throw Decompressor::DecompressionError();
		}
	}
}

}
