/* Copyright (C) Teemu Suutari */

#include "FreezeDecompressor.hpp"
#include "HuffmanDecoder.hpp"
#include "DynamicHuffmanDecoder.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool FreezeDecompressor::detectHeader(uint32_t hdr,uint32_t footer) noexcept
{
	return ((hdr>>16)==0x1f9eU||(hdr>>16)==0x1f9fU);
}

std::shared_ptr<Decompressor> FreezeDecompressor::create(const Buffer &packedData,bool exactSizeKnown,bool verify)
{
	return std::make_shared<FreezeDecompressor>(packedData,exactSizeKnown,verify);
}

FreezeDecompressor::FreezeDecompressor(const Buffer &packedData,bool exactSizeKnown,bool verify) :
	_packedData{packedData},
	_exactSizeKnown{exactSizeKnown}
{
	if (_packedData.size()<2U)
		throw InvalidFormatError();
	uint32_t hdr{_packedData.readBE16(0)};
	if (!detectHeader(hdr<<16,0))
		throw InvalidFormatError();
	_isOldVersion=hdr==0x1f9eU;
	if (_isOldVersion)
	{
		_hufTable={0,0,1,3,8,12,24,16};
	} else {
		if (_packedData.size()<5U)
			throw InvalidFormatError();
		uint16_t tmp{_packedData.readLE16(2U)};
		if (tmp&0x8000U)
			throw InvalidFormatError();
		uint8_t tmp2{_packedData.read8(4U)};
		if (tmp2&0xc0U)
			throw InvalidFormatError();
		_hufTable={uint8_t(tmp&1U),uint8_t((tmp>>1)&3U),uint8_t((tmp>>3)&7U),uint8_t((tmp>>6)&0xfU),uint8_t(tmp>>10),tmp2};

		uint32_t count{62};
		uint32_t weights{256};
		for (uint32_t i=0;i<6U;i++) count-=_hufTable[i];
		for (uint32_t i=0,j=7U;i<6U;i++,j--) weights-=uint32_t(_hufTable[i])<<j;
		if (weights<count || count*2U<weights)
			throw InvalidFormatError();
		_hufTable[6]=weights-count;
		_hufTable[7]=count*2U-weights;
	}
	if (exactSizeKnown) _packedSize=packedData.size();
}

const std::string &FreezeDecompressor::getName() const noexcept
{
	static std::string names[2]={
		"F: Freeze/Melt 1.x",
		"F: Freeze/Melt 2.x"};
	return names[_isOldVersion?0:1U];
}

size_t FreezeDecompressor::getPackedSize() const noexcept
{
	// no way to know before decompressing
	return _packedSize;
}


size_t FreezeDecompressor::getRawSize() const noexcept
{
	// same thing, decompression needed first
	return _rawSize;
}

void FreezeDecompressor::decompressImpl(Buffer &rawData,bool verify)
{
	ForwardInputStream inputStream(_packedData,_isOldVersion?2U:5U,_packedSize?_packedSize:_packedData.size());

	// Special case for empty file
	if (inputStream.eof())
	{
		_rawSize=0U;
		if (_exactSizeKnown && _packedSize!=inputStream.getOffset())
			throw DecompressionError();
		_packedSize=inputStream.getOffset();
	}

	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBits8(count);
	};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBits8(1);
	};

	AutoExpandingForwardOutputStream outputStream{rawData};
	DynamicHuffmanDecoder<511U> decoder{_isOldVersion?315U:511U};
	HuffmanDecoder<uint8_t> distanceDecoder;
	{
		std::array<uint8_t,64> distanceHighBits;
		uint32_t j{0};
		for (uint32_t i=0;i<8U;i++)
		{
			if (j+_hufTable[i]>64U)
				throw DecompressionError();
			for (uint32_t k=0;k<_hufTable[i];k++)
				distanceHighBits[j++]=i+1U;
		}
		distanceDecoder.createOrderlyHuffmanTable(distanceHighBits,j);
	}

	uint32_t distanceBits{_isOldVersion?6U:7U};
	for(;;)
	{
		uint32_t code{decoder.decode(readBit)};
		if (decoder.getMaxFrequency()==0x8000U) decoder.halve();
		decoder.update(code);
		if (code==256U) break;
		if (code<256U)
		{
			outputStream.writeByte(code);
		} else {
			uint32_t distance{uint32_t(distanceDecoder.decode(readBit)<<distanceBits)};
			distance|=readBits(distanceBits);
			distance++;
			uint32_t count{code-254U};
			outputStream.copy(distance,count,0x20);
		}
	}
	_rawSize=outputStream.getOffset();
	if (_exactSizeKnown && inputStream.getOffset()!=_packedSize)
		throw DecompressionError();
	_packedSize=inputStream.getOffset();
}

}
