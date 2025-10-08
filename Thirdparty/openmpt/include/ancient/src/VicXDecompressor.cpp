/* Copyright (C) Teemu Suutari */

#include <cstdint>
#include <cstring>
#include <array>

#include "VicXDecompressor.hpp"
#include "HuffmanDecoder.hpp"
#include "DynamicHuffmanDecoder.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"
#include "common/OverflowCheck.hpp"

namespace ancient::internal
{

bool VicXDecompressor::detectHeader(uint32_t hdr,uint32_t footer) noexcept
{
	return hdr==FourCC("Vice") || hdr==FourCC("Vic2");
}

std::shared_ptr<Decompressor> VicXDecompressor::create(const Buffer &packedData,bool exactSizeKnown,bool verify)
{
	return std::make_shared<VicXDecompressor>(packedData,exactSizeKnown,verify);
}

VicXDecompressor::VicXDecompressor(const Buffer &packedData,bool exactSizeKnown,bool verify) :
	_packedData{packedData}
{
	uint32_t hdr{_packedData.readBE32(0)};
	if (!detectHeader(hdr,0))
		throw InvalidFormatError();
	_isVic2=hdr==FourCC("Vic2");

	if (_isVic2)
	{
		_rawSize=_packedData.readBE32(4U);
		_packedSize=OverflowCheck::sum(_packedData.readBE32(8U),12U);
		if (!_rawSize || _packedSize<1036U || _packedSize>packedData.size() ||
			_rawSize>getMaxRawSize() || _packedSize>getMaxPackedSize())
				throw InvalidFormatError();
	}
}

const std::string &VicXDecompressor::getName() const noexcept
{
	static std::string names[2]{
		"Vice: Huffman compressor with RLE",
		"Vic2: Huffman compressor with RLE"};
	return names[_isVic2?1:0U];
}

size_t VicXDecompressor::getPackedSize() const noexcept
{
	// no way to know before decompressing for Vice
	return _packedSize;
}


size_t VicXDecompressor::getRawSize() const noexcept
{
	// no way to know before decompressing for Vice
	return _rawSize;
}

void VicXDecompressor::decompressImpl(Buffer &rawData,bool verify)
{
	ForwardInputStream inputStream{_packedData,_isVic2?12U:4U,_packedSize?_packedSize:_packedData.size()};
	LSBBitReader<ForwardInputStream> bitReader{inputStream};

	HuffmanDecoder<uint8_t> decoder;
	{
		std::array<uint16_t,511> tree;
		for (uint32_t i=0;i<511U;i++)
			tree[i]=inputStream.readBE16();
		inputStream.readBE16(); 		// ballast

		auto branch=[&](uint32_t node,uint32_t length,uint32_t bits,auto branch)->void
		{
			uint32_t tmp=tree[node];
			if (tmp&0x8000U)
			{
				if (!length)
					throw DecompressionError();
				decoder.insert(HuffmanCode{length,bits,uint8_t(tmp)});
			} else {
				if ((tmp&1U) || tmp>=0x1feU)
					throw DecompressionError();
				length++;
				if (length>24U)
					throw DecompressionError();
				bits<<=1U;
				tmp>>=1U;
				branch(tmp,length,bits,branch);
				branch(tmp+256U,length,bits|1U,branch);
			}
		};
		branch(255U,0,0,branch);
	}

	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBitsBE32(1U);
	};

	if (_isVic2)
	{
		ForwardOutputStream outputStream{rawData,0,_rawSize};

		uint8_t rleMarker=decoder.decode(readBit);
		while (outputStream.getOffset()!=_rawSize)
		{
			uint8_t code{decoder.decode(readBit)};
			if (code==rleMarker)
			{
				uint32_t count{decoder.decode(readBit)};
				if (count)
				{
					count+=3U;
					code=decoder.decode(readBit);
					for (uint32_t i=0;i<count;i++)
						outputStream.writeByte(uint8_t(code));
				} else outputStream.writeByte(rleMarker);
			} else outputStream.writeByte(uint8_t(code));
		}
	} else {
		AutoExpandingForwardOutputStream outputStream{rawData};

		for (;;)
		{
			uint8_t count{decoder.decode(readBit)};
			if (!count) break;
			if (count&0x80U)
			{
				count&=0x7fU;
				uint8_t code{decoder.decode(readBit)};
				for (uint32_t i=0;i<count;i++)
					outputStream.writeByte(uint8_t(code));
			} else {
				for (uint32_t i=0;i<count;i++)
					outputStream.writeByte(decoder.decode(readBit));
			}
		}
		_packedSize=inputStream.getOffset();
		_rawSize=outputStream.getOffset();
	}
}

}
