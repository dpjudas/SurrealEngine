/* Copyright (C) Teemu Suutari */

#include "SCOCompressDecompressor.hpp"
#include "HuffmanDecoder.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"

#include <array>

namespace ancient::internal
{

bool SCOCompressDecompressor::detectHeader(uint32_t hdr,uint32_t footer) noexcept
{
	return ((hdr>>16)==0x1fa0U);
}

std::shared_ptr<Decompressor> SCOCompressDecompressor::create(const Buffer &packedData,bool exactSizeKnown,bool verify)
{
	return std::make_shared<SCOCompressDecompressor>(packedData,exactSizeKnown,verify);
}

SCOCompressDecompressor::SCOCompressDecompressor(const Buffer &packedData,bool exactSizeKnown,bool verify) :
	_packedData{packedData},
	_exactSizeKnown{exactSizeKnown}
{
	if (_packedData.size()<4U)
		throw InvalidFormatError();
	uint32_t hdr{_packedData.readBE16(0)};
	if (!detectHeader(hdr<<16,0))
		throw InvalidFormatError();

	if (exactSizeKnown)
		_packedSize=packedData.size();
}

const std::string &SCOCompressDecompressor::getName() const noexcept
{
	static std::string name{"SCO Compress LZH"};
	return name;
}

size_t SCOCompressDecompressor::getPackedSize() const noexcept
{
	// no way to know before decompressing
	return _packedSize;
}


size_t SCOCompressDecompressor::getRawSize() const noexcept
{
	// same thing, decompression needed first
	return _rawSize;
}

void SCOCompressDecompressor::decompressImpl(Buffer &rawData,bool verify)
{
	ForwardInputStream inputStream{_packedData,2U,_packedSize?_packedSize:_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBits8(count);
	};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBits8(1U);
	};

	AutoExpandingForwardOutputStream outputStream{rawData};

	OptionalHuffmanDecoder<uint32_t> decoder;
	OptionalHuffmanDecoder<uint32_t> distanceDecoder;

	// Almost straight steal from LHX
	uint32_t blockRemaining{0};
	for(;;)
	{
		if (!blockRemaining)
		{
			blockRemaining=readBits(16);
			// 0 is break in SCO
			if (!blockRemaining) break;

			auto createTable=[&](OptionalHuffmanDecoder<uint32_t> &dest,uint32_t bits,bool enableHole)
			{
				std::array<uint8_t,31> symbolBits;
				uint32_t length{readBits(bits)};
				if (!length)
				{
					dest.setEmpty(readBits(bits));
				} else {
					for (uint32_t i=0;i<length;)
					{
						uint32_t value{readBits(3)};
						// SCO has a max of 16-3 bits
						if (value==7U) for (uint32_t j=0;j<13U;j++)
						{
							if (readBit()) value++;
								else break;
						}
						symbolBits[i++]=value;
						if (i==3 && enableHole)
						{
							uint32_t zeros{readBits(2)};
							if (i+zeros>length)
								throw DecompressionError();
							for (uint32_t j=0;j<zeros;j++) symbolBits[i++]=0;
						}
					}
					dest.createOrderlyHuffmanTable(symbolBits,length);
				}
			};

			OptionalHuffmanDecoder<uint32_t> tmpDecoder;
			createTable(tmpDecoder,5,true);

			decoder.reset();

			std::array<uint8_t,511> symbolBits;
			uint32_t length=readBits(9);
			if (!length)
			{
				decoder.setEmpty(readBits(9));
			} else {
				for (uint32_t i=0;i<length;)
				{
					uint32_t value=tmpDecoder.decode(readBit);
					uint32_t rep;
					switch (value)
					{
						case 0:
						value=0;
						rep=1;
						break;

						case 1:
						value=0;
						rep=readBits(4)+3;
						break;

						case 2:
						value=0;
						rep=readBits(9)+20;
						break;

						default:
						value-=2;
						rep=1;
						break;
					}
					if (i+rep>length)
						throw DecompressionError();
					for (uint32_t j=0;j<rep;j++) symbolBits[i++]=value;
				}
				decoder.createOrderlyHuffmanTable(symbolBits,length);
			}

			distanceDecoder.reset();
			createTable(distanceDecoder,4,false);
		}
		blockRemaining--;

		uint32_t code{decoder.decode(readBit)};

		if (code<256)
		{
			outputStream.writeByte(code);
		} else {
			// SCO has ending code
			if (code==510U)
				break;
			uint32_t distanceBits{distanceDecoder.decode(readBit)};
			uint32_t distance{distanceBits?((1<<(distanceBits-1))|readBits(distanceBits-1)):0};
			distance++;

			uint32_t count{code-253};

			// SCO has no defaults
			outputStream.copy(distance,count);
		}
	}

	_rawSize=outputStream.getOffset();
	if (_exactSizeKnown && inputStream.getOffset()!=_packedSize)
		throw DecompressionError();
	_packedSize=inputStream.getOffset();
}

}
