/* Copyright (C) Teemu Suutari */

#include <cstring>

#include "MMCMPDecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/OverflowCheck.hpp"
#include "common/Common.hpp"

#include <array>

namespace ancient::internal
{

bool MMCMPDecompressor::detectHeader(uint32_t hdr,uint32_t footer) noexcept
{
	return hdr==FourCC("ziRC");
}

std::shared_ptr<Decompressor> MMCMPDecompressor::create(const Buffer &packedData,bool exactSizeKnown,bool verify)
{
	return std::make_shared<MMCMPDecompressor>(packedData,exactSizeKnown,verify);
}

MMCMPDecompressor::MMCMPDecompressor(const Buffer &packedData,bool exactSizeKnown,bool verify) :
	_packedData{packedData}
{
	if (!detectHeader(packedData.readBE32(0),0) || packedData.readBE32(4U)!=FourCC("ONia") ||
		packedData.readLE16(8U)!=14U || packedData.size()<24U)
		throw InvalidFormatError();
	_version=packedData.readLE16(10U);
	_blocks=packedData.readLE16(12U);
	_blocksOffset=packedData.readLE32(18U);
	_rawSize=packedData.readLE32(14U);
	if (_rawSize>getMaxRawSize())
		throw InvalidFormatError();
	if (OverflowCheck::sum(_blocksOffset,uint32_t(_blocks)*4U)>packedData.size())
		throw InvalidFormatError();

	_packedSize=0;
	for (uint32_t i=0;i<_blocks;i++)
	{
		uint32_t blockAddr{packedData.readLE32(OverflowCheck::sum(_blocksOffset,i*4U))};
		if (OverflowCheck::sum(blockAddr,20U)>=packedData.size())
			throw InvalidFormatError();
		uint32_t blockSize{OverflowCheck::sum(packedData.readLE32(blockAddr+4U),uint32_t(packedData.readLE16(blockAddr+12U))*8U+20U)};
		_packedSize=std::max(_packedSize,OverflowCheck::sum(blockAddr,blockSize));
	}
	if (_packedSize>packedData.size())
		throw InvalidFormatError();
}

const std::string &MMCMPDecompressor::getName() const noexcept
{
	static std::string name="MMCMP: Music Module Compressor";
	return name;
}

size_t MMCMPDecompressor::getPackedSize() const noexcept
{
	return _packedSize;
}

size_t MMCMPDecompressor::getRawSize() const noexcept
{
	return _rawSize;
}

void MMCMPDecompressor::decompressImpl(Buffer &rawData,bool verify)
{
	if (rawData.size()<_rawSize)
		throw DecompressionError();
	// MMCMP allows gaps in data. Although not used in practice still we memset before decompressing to be sure
	std::memset(rawData.data(),0,rawData.size());

	const std::array<uint8_t,8> extraBits8{3,3,3,3, 2,1,0,0};
	const std::array<uint8_t,16> extraBits16{4,4,4,4, 3,2,1,0, 0,0,0,0, 0,0,0,0};

	for (uint32_t i=0;i<_blocks;i++)
	{
		uint32_t blockAddr{_packedData.readLE32(_blocksOffset+i*4U)};

		uint32_t unpackedBlockSize{_packedData.readLE32(blockAddr)};
		uint32_t packedBlockSize{_packedData.readLE32(blockAddr+4U)};
		uint32_t fileChecksum{_packedData.readLE32(blockAddr+8U)};
		uint32_t subBlocks{_packedData.readLE16(blockAddr+12U)};
		uint16_t flags{_packedData.readLE16(blockAddr+14U)};

		uint32_t packTableSize{_packedData.readLE16(blockAddr+16U)};
		if (packTableSize>packedBlockSize)
			throw DecompressionError();
		uint32_t bitCount{uint32_t{_packedData.readLE16(blockAddr+18U)}+1U};

		ForwardInputStream inputStream{_packedData,OverflowCheck::sum(blockAddr,subBlocks*8U,20U,packTableSize),OverflowCheck::sum(blockAddr,subBlocks*8U,20U,packedBlockSize)};
		LSBBitReader<ForwardInputStream> bitReader{inputStream};
		auto readBits=[&](uint32_t count)->uint32_t
		{
			return bitReader.readBits8(count);
		};

		uint32_t currentSubBlock{0};
		uint32_t outputOffset{0};
		uint32_t outputSize{0};
		auto readNextSubBlock=[&]()
		{
			if (currentSubBlock>=subBlocks)
				throw DecompressionError();
			outputOffset=_packedData.readLE32(blockAddr+currentSubBlock*8U+20U);
			outputSize=_packedData.readLE32(blockAddr+currentSubBlock*8U+24U);
			if (OverflowCheck::sum(outputOffset,outputSize)>_rawSize)
				throw DecompressionError();
			currentSubBlock++;
		};

		uint32_t checksum{0};
		uint32_t checksumPartial{0};
		uint32_t checksumRot{0};
		auto writeByte=[&](uint8_t value,bool allowOverrun=false)
		{
			while (!outputSize)
			{
				if (allowOverrun && currentSubBlock>=subBlocks) return;
				readNextSubBlock();
			}
			outputSize--;
			rawData[outputOffset++]=value;
			if (verify)
			{
				if (_version>=0x1310U)
				{
					checksum^=value;
					checksum=(checksum<<1U)|(checksum>>31U);
				} else {
					checksumPartial|=((uint32_t)value)<<checksumRot;
					checksumRot+=8U;
					if (checksumRot==32U)
					{
						checksum^=checksumPartial;
						checksumPartial=0;
						checksumRot=0;
					}
				}
			}
		};
		
		// flags are
		// 0 = compressed
		// 1 = delta mode
		// 2 = 16 bit mode
		// 8 = stereo
		// 9 = abs16
		// 10 = endian
		// flags do not combine nicely
		// no compress - no other flags
		// compressed 8 bit - only delta (and presumably stereo matters)
		// compressed 16 bit - all flags matter
		if (!(flags&0x1U))
		{
			// not compressed
			for (uint32_t j=0;j<packedBlockSize;j++)
				writeByte(inputStream.readByte());
		} else if (!(flags&0x4U)) {
			// 8 bit compression

			// in case the bit-count is not enough to store a value, symbol at the end
			// of the codemap is created and this marks as a new bitCount
			if (bitCount>8)
				throw DecompressionError();
			std::array<uint8_t,2> oldValue{0,0};
			uint32_t chIndex{0};
			uint32_t tableOffset{blockAddr+subBlocks*8U+20U};
			for (uint32_t j=0;j<unpackedBlockSize;j++)
			{
				uint8_t extraBitCount{extraBits8[bitCount-1U]};
				uint8_t threshold{uint8_t((1U<<bitCount)-(1<<(3U-extraBitCount)))};
				uint8_t value{uint8_t(readBits(bitCount))};
				if (value>=threshold)
				{
					if (uint32_t newBitCount{(readBits(extraBitCount)|((value-threshold)<<extraBitCount))+1U};bitCount!=newBitCount)
					{
						bitCount=newBitCount;
						j--;
						continue;
					} else {
						value=0xf8U|readBits(3U);
						if (value==0xffU && readBits(1U)) break;
					}
				}
				if (value>=packTableSize)
					throw DecompressionError();
				value=_packedData[tableOffset+value];
				if (flags&0x2U)
				{
					// delta
					value+=oldValue[chIndex];
					oldValue[chIndex]=value;
					if (flags&0x100U) chIndex^=1U;		// stereo
				}
				writeByte(value);
			}
		} else {
			// 16 bit compression

			// shameless copy-paste from 8-bit variant, with minor changes
			if (bitCount>16)
				throw DecompressionError();
			std::array<int16_t,2> oldValue{0,0};
			uint32_t chIndex{0};
			for (uint32_t j=0;j<unpackedBlockSize;j+=2U)
			{
				uint8_t extraBitCount{extraBits16[bitCount-1U]};
				uint16_t threshold{uint16_t((1U<<bitCount)-(1<<(4U-extraBitCount)))};
				uint16_t value{uint16_t(readBits(bitCount))};
				if (value>=threshold)
				{
					if (uint32_t newBitCount{(readBits(extraBitCount)|((value-threshold)<<extraBitCount))+1U};bitCount!=newBitCount)
					{
						bitCount=newBitCount;
						j-=2U;
						continue;
					} else {
						value=0xfff0U|readBits(4U);
						if (value==0xffffU && readBits(1U)) break;
					}
				}
				int32_t sValue=value;
				if (sValue&1U) sValue=-sValue-1;
				sValue>>=1;
				if (flags&0x2U)
				{
					// delta
					sValue+=oldValue[chIndex];
					oldValue[chIndex]=sValue;
					if (flags&0x100U) chIndex^=1U;		// stereo
				} else if (!(flags&0x200U)) sValue^=0x8000;	// abs16
				if (flags&0x400U)
				{
					// big ending
					writeByte(sValue>>8);
					writeByte(sValue,true);
				} else {
					// little endian
					writeByte(sValue);
					writeByte(sValue>>8,true);
				}
			}
		}
		if (verify && checksum!=fileChecksum)
			throw VerificationError();
	}
}

}
