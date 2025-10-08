/* Copyright (C) Teemu Suutari */

#include "LZW2Decompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool LZW2Decompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("LZW2") || hdr==FourCC("LZW3");
}

std::shared_ptr<XPKDecompressor> LZW2Decompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<LZW2Decompressor>(hdr,recursionLevel,packedData,state,verify);
}

LZW2Decompressor::LZW2Decompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
	_ver=(hdr==FourCC("LZW2"))?2:3;
}

const std::string &LZW2Decompressor::getSubName() const noexcept
{
	static std::string name2{"XPK-LZW2: LZW2 CyberYAFA compressor"};
	static std::string name3{"XPK-LZW3: LZW3 CyberYAFA compressor"};
	return (_ver==2)?name2:name3;
}

void LZW2Decompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,0,_packedData.size()};
	LSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBitsBE32(1);
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
			uint32_t distance{uint32_t(readByte())<<8};
			distance|=uint32_t(readByte());
			if (!distance)
				throw Decompressor::DecompressionError();
			distance=65536-distance;
			uint32_t count{uint32_t(readByte())+4};

			outputStream.copy(distance,count);
		}
	}
}

}
