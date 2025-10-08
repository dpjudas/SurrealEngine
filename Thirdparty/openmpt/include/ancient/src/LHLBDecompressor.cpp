/* Copyright (C) Teemu Suutari */

#include "LHLBDecompressor.hpp"

#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "DynamicHuffmanDecoder.hpp"
#include "VariableLengthCodeDecoder.hpp"
#include "common/Common.hpp"

namespace ancient::internal
{

bool LHLBDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("LHLB");
}

std::shared_ptr<XPKDecompressor> LHLBDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<LHLBDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

LHLBDecompressor::LHLBDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
}

const std::string &LHLBDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-LHLB: LZRW-compressor"};
	return name;
}

void LHLBDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,0,_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBits8(count);
	};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBits8(1U);
	};

	ForwardOutputStream outputStream(rawData,0,rawData.size());

	// Same logic as in Choloks pascal implementation
	// Differences to LH1:
	// - LHLB does not halve probabilities at 32k
	// - 314 vs. 317 sized huffman entry
	// - no end code
	// - different distance/count logic

	DynamicHuffmanDecoder<317> decoder;
	VariableLengthCodeDecoder vlcDecoder{5,5,6,6,6,7,7,7,7,8,8,8,9,9,9,10};

	while (!outputStream.eof())
	{
		uint32_t code=decoder.decode(readBit);
		if (code==316U) break;
		if (decoder.getMaxFrequency()<0x8000U) decoder.update(code);

		if (code<256U)
		{
			outputStream.writeByte(code);
		} else {
			uint32_t distance{vlcDecoder.decode(readBits,readBits(4U))};
			uint32_t count{code-255U};

			outputStream.copy(distance,count);
		}
	}
}

}
