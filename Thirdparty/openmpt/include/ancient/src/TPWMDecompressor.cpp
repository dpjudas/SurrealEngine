/* Copyright (C) Teemu Suutari */

#include "TPWMDecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool TPWMDecompressor::detectHeader(uint32_t hdr,uint32_t footer) noexcept
{
	return hdr==FourCC("TPWM");
}

std::shared_ptr<Decompressor> TPWMDecompressor::create(const Buffer &packedData,bool exactSizeKnown,bool verify)
{
	return std::make_shared<TPWMDecompressor>(packedData,verify);
}

TPWMDecompressor::TPWMDecompressor(const Buffer &packedData,bool verify) :
	_packedData{packedData}
{
	uint32_t hdr{packedData.readBE32(0)};
	if (!detectHeader(hdr,0) || packedData.size()<12)
		throw InvalidFormatError();

	_rawSize=packedData.readBE32(4);
	if (!_rawSize || _rawSize>getMaxRawSize())
		throw InvalidFormatError();
}

const std::string &TPWMDecompressor::getName() const noexcept
{
	static std::string name="TPWM: Turbo Packer";
	return name;
}

size_t TPWMDecompressor::getPackedSize() const noexcept
{
	// No packed size in the stream :(
	// After decompression, we can tell how many bytes were actually used
	return _decompressedPackedSize;
}

size_t TPWMDecompressor::getRawSize() const noexcept
{
	return _rawSize;
}

void TPWMDecompressor::decompressImpl(Buffer &rawData,bool verify)
{
	if (rawData.size()<_rawSize)
		throw DecompressionError();

	ForwardInputStream inputStream{_packedData,8U,_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBits8(1U);
	};
	auto readByte=[&]()->uint8_t
	{
		return inputStream.readByte();
	};

	ForwardOutputStream outputStream{rawData,0,_rawSize};

	while (!outputStream.eof())
	{
		if (readBit())
		{
			uint8_t byte1{readByte()};
			uint8_t byte2{readByte()};
			uint32_t distance{(uint32_t(byte1&0xf0U)<<4U)|byte2};
			uint32_t count{uint32_t(byte1&0xfU)+3U};

			count=std::min(count,uint32_t(_rawSize-outputStream.getOffset()));
			outputStream.copy(distance,count);
		} else {
			outputStream.writeByte(readByte());
		}
	}

	_decompressedPackedSize=inputStream.getOffset();
}

}
