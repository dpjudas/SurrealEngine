/* Copyright (C) Teemu Suutari */

#include "SMPLDecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "HuffmanDecoder.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool SMPLDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("SMPL");
}

std::shared_ptr<XPKDecompressor> SMPLDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<SMPLDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

SMPLDecompressor::SMPLDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr) || packedData.size()<2)
		throw Decompressor::InvalidFormatError();

	if (packedData.readBE16(0)!=1U)
		throw Decompressor::InvalidFormatError();
}

const std::string &SMPLDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-SMPL: Huffman compressor with delta encoding"};
	return name;
}

void SMPLDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,2U,_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBits8(count);
	};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBits8(1);
	};

	ForwardOutputStream outputStream{rawData,0,rawData.size()};

	HuffmanDecoder<uint32_t> decoder;

	for (uint32_t i=0;i<256;i++)
	{
		uint32_t codeLength{readBits(4U)};
		if (!codeLength) continue;
		if (codeLength==15) codeLength=readBits(4U)+15U;
		uint32_t code{readBits(codeLength)};
		decoder.insert(HuffmanCode{codeLength,code,i});
	}

	uint8_t accum{0};
	while (!outputStream.eof())
	{
		uint32_t code{decoder.decode(readBit)};
		accum+=code;
		outputStream.writeByte(accum);
	}
}

}
