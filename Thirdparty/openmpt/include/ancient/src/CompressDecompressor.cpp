/* Copyright (C) Teemu Suutari */

#include "CompressDecompressor.hpp"
#include "LZWDecoder.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool CompressDecompressor::detectHeader(uint32_t hdr,uint32_t footer) noexcept
{
	return ((hdr>>16)==0x1f9dU);
}

std::shared_ptr<Decompressor> CompressDecompressor::create(const Buffer &packedData,bool exactSizeKnown,bool verify)
{
	return std::make_shared<CompressDecompressor>(packedData,exactSizeKnown,verify);
}

CompressDecompressor::CompressDecompressor(const Buffer &packedData,bool exactSizeKnown,bool verify) :
	_packedData{packedData}
{
	// Can't do anything with undefined size stream
	if (!exactSizeKnown)
		throw InvalidFormatError();
	if (_packedData.size()<3U)
		throw InvalidFormatError();
	uint32_t hdr=_packedData.readBE16(0);
	if (!detectHeader(hdr<<16,0))
		throw InvalidFormatError();
	uint8_t tmp{_packedData.read8(2)};
	_hasBlocks=tmp&0x80U;
	tmp&=0x7fU;
	if (tmp<9U||tmp>16U)
		throw InvalidFormatError();
	_maxBits=tmp;
}

const std::string &CompressDecompressor::getName() const noexcept
{
	static std::string names[2]={
		"Z: Compress (old)",
		"Z: Compress (new)"};
	return names[_hasBlocks?1:0];
}

size_t CompressDecompressor::getPackedSize() const noexcept
{
	// only from input
	return _packedData.size();
}


size_t CompressDecompressor::getRawSize() const noexcept
{
	// same thing, decompression needed first
	return _rawSize;
}

void CompressDecompressor::decompressImpl(Buffer &rawData,bool verify)
{
	// Special case for empty file
	if (_packedData.size()==3U)
	{
		_rawSize=0U;
		return;
	}

	ForwardInputStream inputStream{_packedData,3,_packedData.size()};
	LSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBits8(count);
	};

	AutoExpandingForwardOutputStream outputStream{rawData};
	auto writeByte=[&](uint8_t value)
	{
		outputStream.writeByte(value);
	};

	uint32_t codeBits{9U};
	size_t prevCodePos=inputStream.getOffset();

	uint32_t firstCode{readBits(codeBits)};
	LZWDecoder decoder{1U<<_maxBits,_hasBlocks?257U:256U,8192U,firstCode};
	decoder.write(firstCode,false,writeByte);

	// This is actually surprising for a compressor
	// that was popular at time: There are silly bugs
	// and wastage of bytes since codes are read in blocks
	// and thrown away in reset and code size change
	// the worst thing is setting max bits to 9 which turns instead
	// unbounded bit size compressor (here we throw in case we can more than 16 bits)
	// gzip seems to limit the minimum max bits to 12ish, which is more sane
	// (debugging that was entertaining though)
	uint32_t codeCounter{1};			// previous read for first byte included
	auto reset=[&]()
	{
		bitReader.reset(0,0);
		prevCodePos+=codeBits;
		inputStream.setOffset(prevCodePos);
		codeCounter=0;
	};

	bool maxReached{false};
	auto readCode=[&]()->uint32_t
	{
		if (!maxReached && decoder.getCurrentIndex()>=(1U<<codeBits))
		{
			reset();
			if (codeBits!=_maxBits || _maxBits==9U) codeBits++;
				else maxReached=true;
			if (codeBits>16U)
				throw DecompressionError();
		}
		if (!codeCounter)
			prevCodePos=inputStream.getOffset();
		if (++codeCounter==8U)
			codeCounter=0;
		return readBits(codeBits);
	};

	// since the codes are larger than 8 bits, we can do this
	while (!inputStream.eof())
	{
		if (uint32_t code{readCode()};_hasBlocks && code==256U)
		{
			reset();
			if (inputStream.eof())
				break;

			codeBits=9U;
			maxReached=false;
			firstCode=readBits(codeBits);
			codeCounter++;
			decoder.reset(firstCode);
			decoder.write(firstCode,false,writeByte);
		} else {
			decoder.write(code,!decoder.isLiteral(code),writeByte);
			decoder.add(code);
		}
	}

	_rawSize=outputStream.getOffset();
}

}
