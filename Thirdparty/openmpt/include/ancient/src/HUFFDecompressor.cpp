/* Copyright (C) Teemu Suutari */

#include "HUFFDecompressor.hpp"
#include "HuffmanDecoder.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool HUFFDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("HUFF");
}

std::shared_ptr<XPKDecompressor> HUFFDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<HUFFDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

HUFFDecompressor::HUFFDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr) || packedData.size()<6)
		throw Decompressor::InvalidFormatError();
	// version: only 0 is defined
	uint16_t ver{packedData.readBE16(0)};
	if (ver)
		throw Decompressor::InvalidFormatError();
	// password: we do not support it...
	uint32_t pwd{packedData.readBE32(2)};
	if (pwd!=0xabadcafe)
		throw Decompressor::InvalidFormatError();
}

const std::string &HUFFDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-HUFF: Huffman compressor"};
	return name;
}

void HUFFDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,6,_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBits8(1);
	};
	auto readByte=[&]()->uint8_t
	{
		return inputStream.readByte();
	};

	ForwardOutputStream outputStream{rawData,0,rawData.size()};

	HuffmanDecoder<uint8_t> decoder;
	for (uint32_t i=0;i<256;i++)
	{
		uint8_t codeBits{uint8_t(readByte()+1)};
		if (!codeBits) continue;
		if (codeBits>32)
			throw Decompressor::DecompressionError();
		uint32_t code{0};
		int32_t shift{-codeBits};
		for (uint32_t j=0;j<codeBits;j+=8)
		{
			code=(code<<8)|readByte();
			shift+=8;
		}
		code=(code>>shift)&((1<<codeBits)-1);
		decoder.insert(HuffmanCode{codeBits,code,uint8_t(i)});
	}

	while (!outputStream.eof())
		outputStream.writeByte(decoder.decode(readBit));
}

}
