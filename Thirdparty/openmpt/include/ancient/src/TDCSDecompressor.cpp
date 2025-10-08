/* Copyright (C) Teemu Suutari */

#include "TDCSDecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool TDCSDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("TDCS");
}

std::shared_ptr<XPKDecompressor> TDCSDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<TDCSDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

TDCSDecompressor::TDCSDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
}

const std::string &TDCSDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-TDCS: LZ77-compressor"};
	return name;
}

void TDCSDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,0,_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto read2Bits=[&]()->uint32_t
	{
		return bitReader.readBitsBE32(2);
	};
	auto readByte=[&]()->uint8_t
	{
		return inputStream.readByte();
	};

	ForwardOutputStream outputStream{rawData,0,rawData.size()};

	while (!outputStream.eof())
	{
		uint32_t distance{0};
		uint32_t count{0};
		uint32_t tmp;
		switch (read2Bits())
		{
			case 0:
			outputStream.writeByte(readByte());
			break;

			case 1:
			tmp=uint32_t(readByte())<<8U;
			tmp|=uint32_t(readByte());
			count=(tmp&3U)+3U;
			distance=((tmp>>2U)^0x3fffU)+1U;
			break;

			case 2:
			tmp=uint32_t(readByte())<<8U;
			tmp|=uint32_t(readByte());
			count=(tmp&0xfU)+3U;
			distance=((tmp>>4U)^0xfffU)+1U;
			break;

			case 3:
			distance=uint32_t(readByte())<<8U;
			distance|=uint32_t(readByte());
			count=uint32_t(readByte())+3U;
			if (!distance)
				throw Decompressor::DecompressionError();
			distance=(distance^0xffffU)+1U;
			break;
			
			default:
			throw Decompressor::DecompressionError();
		}
		if (count && distance)
			outputStream.copy(distance,count);
	}
}

}
