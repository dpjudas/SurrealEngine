/* Copyright (C) Teemu Suutari */

#include "LZW4Decompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool LZW4Decompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("LZW4");
}

std::shared_ptr<XPKDecompressor> LZW4Decompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<LZW4Decompressor>(hdr,recursionLevel,packedData,state,verify);
}

LZW4Decompressor::LZW4Decompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
}

const std::string &LZW4Decompressor::getSubName() const noexcept
{
	static std::string name{"XPK-LZW4: LZW4 CyberYAFA compressor"};
	return name;
}

void LZW4Decompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,0,_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBitsBE32(1U);
	};
	auto readByte=[&]()->uint8_t
	{
		return inputStream.readByte();
	};

	ForwardOutputStream outputStream{rawData,0,rawData.size()};

	while (!outputStream.eof())
	{
		if (!readBit())
		{
			outputStream.writeByte(readByte());
		} else {
			uint32_t distance={uint32_t(readByte())<<8U};
			distance|=readByte();
			if (!distance)
				throw Decompressor::DecompressionError();
			distance=65536U-distance;
			uint32_t count=uint32_t(readByte())+3;

			outputStream.copy(distance,count);
		}
	}
}

}
