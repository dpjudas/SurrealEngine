/* Copyright (C) Teemu Suutari */

#include "BLZWDecompressor.hpp"
#include "LZWDecoder.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool BLZWDecompressor::detectHeaderXPK(uint32_t hdr)
{
	return hdr==FourCC("BLZW");
}

std::shared_ptr<XPKDecompressor> BLZWDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<BLZWDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

BLZWDecompressor::BLZWDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
	_maxBits=_packedData.readBE16(0);
	if (_maxBits<9 || _maxBits>20)
		throw Decompressor::InvalidFormatError();;
	_stackLength=uint32_t(_packedData.readBE16(2))+5;
}

const std::string &BLZWDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-BLZW: LZW-compressor"};
	return name;
}

void BLZWDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,4,_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBits8(count);
	};

	ForwardOutputStream outputStream{rawData,0,rawData.size()};
	auto writeByte=[&](uint8_t value)
	{
		outputStream.writeByte(value);
	};

	uint32_t codeBits{9U};
	auto readCode=[&]()->uint32_t
	{
		return readBits(codeBits);
	};

	uint32_t firstCode=readCode();
	LZWDecoder decoder{1U<<_maxBits,259U,_stackLength,firstCode};
	decoder.write(firstCode,false,writeByte);

	while (!outputStream.eof())
	{
		switch (uint32_t code{readBits(codeBits)};code)
		{
			case 256:
			throw Decompressor::DecompressionError();
			break;

			case 257:
			codeBits=9U;
			firstCode=readCode();
			decoder.reset(firstCode);
			decoder.write(firstCode,false,writeByte);
			break;

			case 258:
			if (codeBits>=24)
				throw Decompressor::DecompressionError();
			codeBits++;
			break;

			default:
			decoder.write(code,!decoder.isLiteral(code),writeByte);
			decoder.add(code);
			break;
		}
	}
}

}
