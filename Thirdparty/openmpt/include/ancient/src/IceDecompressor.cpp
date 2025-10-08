/* Copyright (C) Teemu Suutari */

#include "IceDecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "VariableLengthCodeDecoder.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool IceDecompressor::detectHeader(uint32_t hdr,uint32_t footer) noexcept
{
	return (
		// ver 0
		footer==FourCC("Ice!"))||
		// ver 1
		(hdr==FourCC("Ice!"))||
		(hdr==FourCC("TMM!"))||		// Demo Numb/Movement
		(hdr==FourCC("TSM!"))||		// Lots of Amiga games
		(hdr==FourCC("SHE!"))||		// Demo Overload2/JetSet
		// ver 2
		(hdr==FourCC("ICE!"));
}

std::shared_ptr<Decompressor> IceDecompressor::create(const Buffer &packedData,bool exactSizeKnown,bool verify)
{
	return std::make_shared<IceDecompressor>(packedData,exactSizeKnown,verify);
}

IceDecompressor::IceDecompressor(const Buffer &packedData,bool exactSizeKnown,bool verify) :
	_packedData{packedData}
{
	if (packedData.size()<8)
		throw InvalidFormatError();
	uint32_t hdr{packedData.readBE32(0)};
	uint32_t footer{exactSizeKnown?packedData.readBE32(packedData.size()-4U):0};

	if (!detectHeader(hdr,footer))
		throw InvalidFormatError();

	// In theory a bitstream can be v1 and v2 at the same time
	// we prefer v1 in case of conflict
	if (footer==FourCC("Ice!"))
	{
		_packedSize=packedData.size();
		_rawSize=packedData.readBE32(packedData.size()-8U);
		_ver=0;
	} else {
		_packedSize=packedData.readBE32(4U);
		if (!_packedSize || _packedSize>packedData.size() || _packedSize>getMaxPackedSize())
			throw InvalidFormatError();
		_rawSize=packedData.readBE32(8U);
		_ver=(hdr==FourCC("ICE!"))?2U:1U;
	}
	if (!_rawSize || _rawSize>getMaxRawSize())
		throw InvalidFormatError();
}

const std::string &IceDecompressor::getName() const noexcept
{
	static std::string names[3]={
		{"Ice: Pack-Ice v1.1 - v1.14"},
		{"Ice: Pack-Ice v2.0 - v2.20"},
		{"ICE: Pack-Ice v2.31+"}};
	return names[_ver];
}

size_t IceDecompressor::getPackedSize() const noexcept
{
	return _packedSize;
}

size_t IceDecompressor::getRawSize() const noexcept
{
	return _rawSize;
}

void IceDecompressor::decompressInternal(Buffer &rawData,bool useBytes)
{
	BackwardInputStream inputStream{_packedData,_ver?12U:0,_packedSize-(_ver?0:8U)};
	MSBBitReader<BackwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		if (useBytes) return bitReader.readBits8(count);
			else return bitReader.readBitsBE32(count);
	};
	auto readByte=[&]()->uint8_t
	{
		return inputStream.readByte();
	};

	// anchor-bit handling
	{
		uint32_t value{useBytes?inputStream.readByte():inputStream.readBE32()};
		uint32_t tmp{value};
		uint32_t count{0};
		while (tmp)
		{
			tmp<<=1;
			count++;
		}
		if (count) count--;
		if (count)
			bitReader.reset(value>>(32U-count),count-(useBytes?24U:0));
	}

	BackwardOutputStream outputStream{rawData,0,_rawSize};

	VariableLengthCodeDecoder litVlcDecoderOld{1,2,2,3,10};
	VariableLengthCodeDecoder litVlcDecoderNew{1,2,2,3,8,15};
	VariableLengthCodeDecoder countBaseDecoder{1,1,1,1};
	VariableLengthCodeDecoder countDecoder{0,0,1,2,10};
	VariableLengthCodeDecoder distanceBaseDecoder{1,1};
	VariableLengthCodeDecoder distanceDecoder{5,8,12};

	// Early versions have pretty broken distance/length handling
	// which was later improved at the same time bitstream was changed to
	// byte-based format. This is why these 2 are bundled in here.
	for (;;)
	{
		if (readBits(1U))
		{
			uint32_t litLength{(_ver?litVlcDecoderNew.decodeCascade(readBits):litVlcDecoderOld.decodeCascade(readBits))+1U};
			for (uint32_t i=0;i<litLength;i++)
				outputStream.writeByte(readByte());
		}
		// exit criteria
		if (outputStream.eof()) break;

		uint32_t countBase{countBaseDecoder.decodeCascade(readBits)};
		uint32_t count{countDecoder.decode(readBits,countBase)+2U};
		uint32_t distance;
		if (count==2U)
		{
			if (readBits(1U)) distance=readBits(9U)+0x40U;
				else distance=readBits(6U);
			distance+=count-(useBytes?1U:0);
		} else {
			uint32_t distanceBase{distanceBaseDecoder.decodeCascade(readBits)};
			if (distanceBase<2U) distanceBase^=1U;
			distance=distanceDecoder.decode(readBits,distanceBase);
			if (useBytes)
			{
				if (distance) distance+=count-1;
					else distance+=1;
			} else distance+=count;
		}
		outputStream.copy(distance,count);
	}

	// picture mode
	if (_ver && readBits(1U))
	{
		uint32_t pictureSize=32000U;
		if (_ver==2)
		{
			// magic: Format changes between versions. ID does not
			if (bitReader.available()>=17U && readBits(1U))
				pictureSize=readBits(16U)*8U+8U;
		}

		if (_rawSize<pictureSize)
			throw DecompressionError();
		// C2P vibes here
		for (uint32_t i=_rawSize-pictureSize;i<_rawSize;i+=8)
		{
			uint16_t values[4];
			for (uint32_t j=0;j<8U;j+=2U)
			{
				uint16_t tmp=rawData.readBE16(i+6U-j);
				for (uint32_t k=0;k<16U;k++)
				{
					values[k&3U]=(values[k&3U]<<1U)|(tmp>>15U);
					tmp<<=1U;
				}
			}
			for (uint32_t j=0;j<4U;j++)
			{
				rawData[i+j*2U]=values[j]>>8U;
				rawData[i+j*2U+1U]=uint8_t(values[j]);
			}
		}
	}
	// final sanity checking
	if (!inputStream.eof())
		throw DecompressionError();
}

void IceDecompressor::decompressImpl(Buffer &rawData,bool verify)
{
	if (_ver)
	{
		if (_ver==1)
		{
			// there is a mix of v1 and v2 with a single id.
			// thus we need to try both. start with v1
			try
			{
				decompressInternal(rawData,false);
				return;
			} catch (const Error &) {
				// nothing needed
			}
		}
		decompressInternal(rawData,true);
	} else decompressInternal(rawData,false);
}

}
