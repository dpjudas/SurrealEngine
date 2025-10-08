/* Copyright (C) Teemu Suutari */

#include <cstdint>
#include <cstring>

#include "LZXDecompressor.hpp"
#include "HuffmanDecoder.hpp"
#include "DLTADecode.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "VariableLengthCodeDecoder.hpp"
#include "common/CRC32.hpp"
#include "common/Common.hpp"
#include "common/OverflowCheck.hpp"


namespace ancient::internal
{

bool LZXDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("ELZX") || hdr==FourCC("SLZX");
}

std::shared_ptr<XPKDecompressor> LZXDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<LZXDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

LZXDecompressor::LZXDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
	if (hdr==FourCC("SLZX")) _isSampled=true;
	// There is no good spec on the LZX header content -> lots of unknowns here
	if (_packedData.size()<41)
		throw Decompressor::InvalidFormatError();
	// XPK LZX compression is embedded single file of LZX -> read first file. Ignore rest
	// this will include flags, which need to be zero anyway
	uint32_t streamHdr=_packedData.readBE32(0);
	if (streamHdr!=FourCC("LZX\0"))
		throw Decompressor::InvalidFormatError();

	_rawSize=_packedData.readLE32(12);
	_packedSize=_packedData.readLE32(16);

	_rawCRC=_packedData.readLE32(32);
	uint32_t headerCRC{_packedData.readLE32(36)};

	uint8_t tmp{_packedData.read8(21)};
	if (tmp && tmp!=2)
		throw Decompressor::InvalidFormatError();
	if (tmp==2) _isCompressed=true;

	_packedOffset=41U+uint32_t(_packedData.read8(40U));
	_packedOffset+=_packedData.read8(24U);
	_packedSize+=_packedOffset;

	if (_packedSize>_packedData.size())
		throw Decompressor::InvalidFormatError();
	if (verify)
	{
		uint32_t crc{CRC32(_packedData,10,26,0)};
		for (uint32_t i=0;i<4;i++) crc=CRC32Byte(0,crc);
		crc=CRC32(_packedData,40,_packedOffset-40,crc);
		if (crc!=headerCRC)
			throw Decompressor::VerificationError(); 
	}
}

const std::string &LZXDecompressor::getSubName() const noexcept
{
	static std::string nameE{"XPK-ELZX: LZX-compressor"};
	static std::string nameS{"XPK-SLZX: LZX-compressor with delta encoding"};
	return (_isSampled)?nameS:nameE;
}

void LZXDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	if (rawData.size()!=_rawSize)
		throw Decompressor::DecompressionError();
	if (!_isCompressed)
	{
		if (_packedSize!=_rawSize)
			throw Decompressor::DecompressionError();
		std::memcpy(rawData.data(),_packedData.data()+_packedOffset,_rawSize);
		return;
	}

	ForwardInputStream inputStream{_packedData,_packedOffset,_packedSize};
	LSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBitsBE16(count);
	};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBitsBE16(1U);
	};

	ForwardOutputStream outputStream{rawData,0,rawData.size()};

	typedef HuffmanDecoder<uint32_t> LZXDecoder;

	// possibly padded/reused later if multiple blocks
	std::array<uint8_t,768U> literalTable;
	for (uint32_t i=0;i<768U;i++) literalTable[i]=0;
	LZXDecoder literalDecoder;
	uint32_t previousDistance{1};

	VariableLengthCodeDecoder vlcDecoder{
		 0, 0, 0, 0, 1, 1, 2, 2,
		 3, 3, 4, 4, 5, 5, 6, 6,
		 7, 7, 8, 8, 9, 9, 10,10,
		11,11,12,12,13,13,14,14};

	while (!outputStream.eof())
	{
		auto createHuffmanTable=[&](LZXDecoder &dec,const auto &bitLengths,uint32_t bitTableLength)
		{
			uint8_t minDepth{16};
			uint8_t maxDepth{0};
			for (uint32_t i=0;i<bitTableLength;i++)
			{
				if (bitLengths[i] && bitLengths[i]<minDepth) minDepth=bitLengths[i];
				if (bitLengths[i]>maxDepth) maxDepth=bitLengths[i];
			}
			if (!maxDepth) return;

			dec.createOrderlyHuffmanTable(bitLengths,bitTableLength);
		};

		uint32_t method{readBits(3U)};
		if (method<1U || method>3U)
			throw Decompressor::DecompressionError();

		LZXDecoder distanceDecoder;
		if (method==3U)
		{
			std::array<uint8_t,8> bitLengths;
			for (uint32_t i=0;i<8U;i++) bitLengths[i]=readBits(3U);
			createHuffmanTable(distanceDecoder,bitLengths,8U);
		}

		size_t blockLength{readBits(8)<<16};
		blockLength|=readBits(8)<<8;
		blockLength|=readBits(8);
		if (OverflowCheck::sum(blockLength,outputStream.getOffset())>_rawSize)
			throw Decompressor::DecompressionError();

		if (method!=1U)
		{
			literalDecoder.reset();
			for (uint32_t pos=0,block=0;block<2;block++)
			{
				uint32_t adjust{(block)?0:1U};
				uint32_t maxPos{(block)?768U:256U};
				LZXDecoder bitLengthDecoder;
				{
					std::array<uint8_t,20> lengthTable;
					for (uint32_t i=0;i<20U;i++) lengthTable[i]=readBits(4U);
					createHuffmanTable(bitLengthDecoder,lengthTable,20U);
				}
				while (pos<maxPos)
				{
					uint32_t symbol=bitLengthDecoder.decode(readBit);

					auto doRepeat=[&](uint32_t count,uint8_t value)
					{
						if (count>maxPos-pos) count=maxPos-pos;
						while (count--) literalTable[pos++]=value;
					};
					
					auto symDecode=[&](uint32_t value)->uint32_t
					{
						return (literalTable[pos]+17U-value)%17U;
					};

					switch (symbol)
					{
						case 17U:
						doRepeat(readBits(4U)+3U+adjust,0);
						break;

						case 18U:
						doRepeat(readBits(6U-adjust)+19U+adjust,0);
						break;

						case 19U:
						{
							uint32_t count{readBit()+3U+adjust};
							doRepeat(count,symDecode(bitLengthDecoder.decode(readBit)));
						}
						break;

						default:
						literalTable[pos++]=symDecode(symbol);
						break;
					}
				}
			}
			createHuffmanTable(literalDecoder,literalTable,768U);
		}
		
		while (blockLength)
		{
			uint32_t symbol{literalDecoder.decode(readBit)};
			if (symbol<256U) {
				outputStream.writeByte(symbol);
				blockLength--;
			} else {
				symbol-=256U;
				uint32_t distance;
				if ((symbol&0x1fU)>=8U && method==3U)
				{
					distance=vlcDecoder.decode([&](uint32_t count)
					{
						uint32_t tmp={readBits(count-3U)<<3U};
						return tmp|distanceDecoder.decode(readBit);
					},symbol&0x1fU);
				} else {
					distance=vlcDecoder.decode(readBits,symbol&0x1fU);
					if (!distance) distance=previousDistance;
				}
				previousDistance=distance;
				uint32_t count{vlcDecoder.decode(readBits,symbol>>5U)+3U};
				if (count>blockLength)
					throw Decompressor::DecompressionError();
				outputStream.copy(distance,count);
				blockLength-=count;
			}
		}
	}
	if (verify)
	{
		uint32_t crc{CRC32(rawData,0,_rawSize,0)};
		if (crc!=_rawCRC)
			throw Decompressor::VerificationError();
	}
	if (_isSampled)
		DLTADecode::decode(rawData,rawData,0,_rawSize);
}

}
