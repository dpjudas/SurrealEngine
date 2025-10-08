/* Copyright (C) Teemu Suutari */

#include "SQSHDecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "HuffmanDecoder.hpp"
#include "VariableLengthCodeDecoder.hpp"
#include "common/Common.hpp"

#include <array>

namespace ancient::internal
{

bool SQSHDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("SQSH");
}

std::shared_ptr<XPKDecompressor> SQSHDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<SQSHDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

SQSHDecompressor::SQSHDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr) || packedData.size()<3)
		throw Decompressor::InvalidFormatError();
	_rawSize=packedData.readBE16(0);
	if (!_rawSize)
		throw Decompressor::InvalidFormatError();
}

const std::string &SQSHDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-SQSH: Compressor for sampled sounds"};
	return name;
}
	
void SQSHDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	if (rawData.size()!=_rawSize)
		throw Decompressor::DecompressionError();

	ForwardInputStream inputStream{_packedData,2,_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBits8(count);
	};
	auto readSignedBits=[&](uint8_t bits)->int32_t
	{
		int32_t ret{int32_t(readBits(bits))};
		if (ret&(1<<(bits-1)))
			ret|=~0U<<bits;
		return ret;
	};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBits8(1);
	};
	auto readByte=[&]()->uint8_t
	{
		return inputStream.readByte();
	};

	ForwardOutputStream outputStream{rawData,0,_rawSize};

	HuffmanDecoder<uint8_t> modDecoder
	{
		HuffmanCode{1,0b0001,uint8_t{0}},
		HuffmanCode{2,0b0000,uint8_t{1}},
		HuffmanCode{3,0b0010,uint8_t{2}},
		HuffmanCode{4,0b0110,uint8_t{3}},
		HuffmanCode{4,0b0111,uint8_t{4}}
	};

	HuffmanDecoder<uint8_t> lengthDecoder
	{
		HuffmanCode{1,0b0000,uint8_t{0}},
		HuffmanCode{2,0b0010,uint8_t{1}},
		HuffmanCode{3,0b0110,uint8_t{2}},
		HuffmanCode{4,0b1110,uint8_t{3}},
		HuffmanCode{4,0b1111,uint8_t{4}}
	};

	HuffmanDecoder<uint8_t> distanceDecoder
	{
		HuffmanCode{1,0b01,uint8_t{1}},
		HuffmanCode{2,0b00,uint8_t{0}},
		HuffmanCode{2,0b01,uint8_t{2}}
	};

	// first byte is special
	uint8_t currentSample{readByte()};
	outputStream.writeByte(currentSample);

	uint32_t accum1{0};
	uint32_t accum2{0};
	uint32_t prevBits{0};

	const std::array<std::array<uint8_t,8>,7> bitLengthTable{{
		{2,3,4,5,6,7,8,0},
		{3,2,4,5,6,7,8,0},
		{4,3,5,2,6,7,8,0},
		{5,4,6,2,3,7,8,0},
		{6,5,7,2,3,4,8,0},
		{7,6,8,2,3,4,5,0},
		{8,7,6,2,3,4,5,0}}};

	VariableLengthCodeDecoder lengthVlcDecoder{1,1,1,3,5};
	VariableLengthCodeDecoder distanceVlcDecoder{8,12,14};

	while (!outputStream.eof())
	{
		uint8_t bits{0};
		uint32_t count{0};
		bool doRepeat{false};

		if (accum1>=8U)
		{

			auto handleCondCase=[&]()
			{
				if (bits==8U) {
					if (accum2<20U)
					{
						count=1U;
					} else {
						count=2U;
						accum2+=8U;
					}
				} else {
					count=5U;
					accum2+=8U;
				}
			};

			auto handleTable=[&](uint32_t newBits)
			{
				if (prevBits<2U || !newBits)
					throw Decompressor::DecompressionError();
				bits=bitLengthTable[prevBits-2U][newBits-1U];
				if (!bits)
					throw Decompressor::DecompressionError();
				handleCondCase();
			};

			switch (modDecoder.decode(readBit))
			{
				case 0:
				if (prevBits==8U)
				{
					bits=8U;
					handleCondCase();
				} else {
					bits=prevBits;
					count=5U;
					accum2+=8U;
				}
				break;

				case 1U:
				doRepeat=true;
				break;

				case 2U:
				handleTable(2U);
				break;

				case 3U:
				handleTable(3U);
				break;

				case 4U:
				handleTable(readBits(2U)+4U);
				break;

				default:
				throw Decompressor::DecompressionError();
			}
		} else {
			if (readBit())
			{
				doRepeat=true;
			} else {
				count=1U;
				bits=8U;
			}
		}

		if (doRepeat) {
/*			uint32_t lengthIndex=lengthDecoder.decode(readBit);
			static const uint8_t lengthBits[5]={1,1,1,3,5};
			static const uint32_t lengthAdditions[5]={2,4,6,8,16};
			count=readBits(lengthBits[lengthIndex])+lengthAdditions[lengthIndex];*/
			count=lengthVlcDecoder.decode(readBits,lengthDecoder.decode(readBit))+2U;
			if (count>=3)
			{
				if (accum1) accum1--;
				if (count>3 && accum1) accum1--;
			}
/*			uint32_t distanceIndex=distanceDecoder.decode(readBit);
			static const uint8_t distanceBits[3]={12,8,14};
			static const uint32_t distanceAdditions[3]={0x101,1,0x1101};
			uint32_t distance=readBits(distanceBits[distanceIndex])+distanceAdditions[distanceIndex];*/
			uint32_t distance{distanceVlcDecoder.decode(readBits,distanceDecoder.decode(readBit))+1U};
			count=std::min(count,uint32_t(_rawSize-outputStream.getOffset()));
			currentSample=outputStream.copy(distance,count);
		} else {
			count=std::min(count,uint32_t(_rawSize-outputStream.getOffset()));
			for (uint32_t i=0;i<count;i++)
			{
				currentSample-=readSignedBits(bits);
				outputStream.writeByte(currentSample);
			}
			if (accum1!=31U) accum1++;
			prevBits=bits;
		}

		accum2-=accum2>>3U;
	}
}

}
