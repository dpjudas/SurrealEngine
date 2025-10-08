/* Copyright (C) Teemu Suutari */

#include "StoneCrackerDecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "HuffmanDecoder.hpp"
#include "VariableLengthCodeDecoder.hpp"
#include "common/Common.hpp"
#include "common/MemoryBuffer.hpp"


namespace ancient::internal
{

bool StoneCrackerDecompressor::detectHeaderAndGeneration(uint32_t hdr,uint32_t &generation) noexcept
{
	// Stonecracker 2.71 (and others from 2.69 - 2.8 series) do not have any sensible identification value
	// first 3 bytes are constants for RLE-encoder (byte values for RLE-encoder, at least they are unique)
	// last byte is bit length.
	//
	// 2.92 and 2.99 do not have either any proper identification word either, however its
	// bit lengts for decompressor are stored in the first 4 bytes, which forms identifiable
	// value.
	//
	// Thus for detecting 2.71 and friends, we are creating lots of false positives here.
	// At least we can rule those out later when detecting the actual header content
	// Final complication is that values for 2.71/2.9X overlap, this we need to handle
	// later as well

	if (hdr>=0x08090a08U && hdr<=0x08090a0eU && hdr!=0x08090a09U)
	{
		// can be generation 1 as well. needs to be determined later
		generation=2;
		return true;
	}

	if ((hdr&0xffU)>=0x08U&&(hdr&0xffU)<=0x0eU)
	{
		uint8_t byte0{uint8_t(hdr>>24U)};
		uint8_t byte1{uint8_t(hdr>>16U)};
		uint8_t byte2{uint8_t(hdr>>8U)};
		// only limiter I can think of apart from the last byte is that the rle-bytes are unique
		if (byte0!=byte1 && byte0!=byte2 && byte1!=byte2)
		{
			generation=1;
			return true;
		}
	}

	// Specials
	switch (hdr&0xffff'ff00U)
	{
		case FourCC("1AM\0"):		// Reunion
		generation=3;
		return true;

		case FourCC("2AM\0"):		// Reunion
		generation=6;
		return true;

		default:
		break;
	}

	// From 3.00 and onwards we can be certain of the format
	switch (hdr)
	{
		case FourCC("S300"):
		generation=3;
		return true;

		case FourCC("S310"):
		generation=4;
		return true;

		case FourCC("S400"):
		generation=5;
		return true;

		case FourCC("S401"):
		generation=6;
		return true;

		case FourCC("S403"):
		[[fallthrough]];
		case FourCC("Z&G!"):		//  Switchback / Rebels
		[[fallthrough]];
		case FourCC("ZULU"):		//  Whammer Slammer / Rebels
		generation=7;
		return true;

		case FourCC("S404"):
		[[fallthrough]];
		case FourCC("AYS!"):		// High Anxiety / Abyss
		generation=8;
		return true;

		default:
		return false;
	}
}

bool StoneCrackerDecompressor::detectHeader(uint32_t hdr,uint32_t footer) noexcept
{
	uint32_t dummy;
	return detectHeaderAndGeneration(hdr,dummy);
}

std::shared_ptr<Decompressor> StoneCrackerDecompressor::create(const Buffer &packedData,bool exactSizeKnown,bool verify)
{
	return std::make_shared<StoneCrackerDecompressor>(packedData,exactSizeKnown,verify);
}

void StoneCrackerDecompressor::initialize(const Buffer &packedData,uint32_t hdr)
{
	auto readModes=[&](uint32_t value)
	{
		for (uint32_t i=0;i<4U;i++)
		{
			_modes[i]=value>>24U;
			if (_modes[i]<8U || _modes[i]>14U)
				throw InvalidFormatError();
			value<<=8U;
		}
	};

	switch (_generation)
	{
		case 1:
		_dataOffset=18U;
		_rle[0]=hdr>>24U;
		_rle[1]=hdr>>16U;
		_rle[2]=hdr>>8U;
		_modes[0]=hdr;
		if (packedData.size()<_dataOffset)
			throw InvalidFormatError();
		for (uint32_t i=1;i<3U;i++)
		{
			_modes[i]=packedData.read8(i+15U);
			if (_modes[i]<4U || _modes[i]>7U)
				throw InvalidFormatError();
		}
		_rleSize=packedData.readBE32(4U);
		_rawSize=packedData.readBE32(8U);
		_packedSize=packedData.readBE32(12U);
		break;

		case 2:
		readModes(hdr);
		[[fallthrough]];
		case 4:
		[[fallthrough]];
		case 5:
		[[fallthrough]];
		case 6:
		_dataOffset=12U;
		if (packedData.size()<_dataOffset)
			throw InvalidFormatError();
		_rawSize=packedData.readBE32(4U);
		_packedSize=packedData.readBE32(8U);
		break;

		case 3:
		_dataOffset=16U;
		if (packedData.size()<_dataOffset)
			throw InvalidFormatError();
		readModes(packedData.readBE32(4));
		_rawSize=packedData.readBE32(8U);
		_packedSize=packedData.readBE32(12U);
		break;

		case 7:
		case 8:
		_dataOffset=16U;
		if (packedData.size()<_dataOffset+2U)
			throw InvalidFormatError();
		_rawSize=packedData.readBE32(8U);
		_packedSize=packedData.readBE32(12U)+2U;
		break;

		default:
		throw InvalidFormatError();
		break;
	}

	if (_packedSize>getMaxPackedSize() || _rawSize>getMaxRawSize())
		throw InvalidFormatError();

	// Final sanity checks on old formats, especially on 2.71 which can still be false positive
	// For a sanity check we assume the compressor is actually compressing, which is not a exactly wrong thing to do
	// (of course there could be expanding files, but it is doubtful someone would actually used them)
	// Also, the compressor seem to crash on large files. Lets cap the filesize to 1M
	if (_generation==1 && (_rleSize>_rawSize || _packedSize>_rleSize || _rawSize>1048'576U))
		throw InvalidFormatError();
	_packedSize+=_dataOffset;
	if (_packedSize>packedData.size())
		throw InvalidFormatError();
}

StoneCrackerDecompressor::StoneCrackerDecompressor(const Buffer &packedData,bool exactSizeKnown,bool verify) :
	_packedData{packedData}
{
	uint32_t hdr{packedData.readBE32(0)};
	if (!detectHeaderAndGeneration(hdr,_generation))
		throw InvalidFormatError();

	bool initialized{false};
	// this is the fallback if we have accidentally identified the wrong version
	if (_generation==2)
	{
		try
		{
			initialize(packedData,hdr);
			initialized=true;
		} catch (const Error &) {
			_generation=1;
		}
	}
	if (!initialized) initialize(packedData,hdr);
}

const std::string &StoneCrackerDecompressor::getName() const noexcept
{
	switch (_generation)
	{
		case 1:
		{
			static std::string name{"SC: StoneCracker v2.69 - v2.81"};
			return name;
		}

		case 2:
		{
			static std::string name{"SC: StoneCracker v2.92, v2.99"};
			return name;
		}

		case 3:
		{
			static std::string name{"S300: StoneCracker v3.00"};
			return name;
		}

		case 4:
		{
			static std::string name{"S310: StoneCracker v3.10, v3.11b"};
			return name;
		}

		case 5:
		{
			static std::string name{"S400: StoneCracker pre v4.00"};
			return name;
		}

		case 6:
		{
			static std::string name{"S401: StoneCracker v4.01"};
			return name;
		}

		case 7:
		{
			static std::string name{"S403: StoneCracker v4.02a"};
			return name;
		}

		case 8:
		{
			static std::string name{"S404: StoneCracker v4.10"};
			return name;
		}
	}
	static std::string dummy="";
	return dummy;
}

size_t StoneCrackerDecompressor::getPackedSize() const noexcept
{
	return _packedSize;
}

size_t StoneCrackerDecompressor::getRawSize() const noexcept
{
	return _rawSize;
}

// v2.71
void StoneCrackerDecompressor::decompressGen1(Buffer &rawData)
{
	BackwardInputStream inputStream{_packedData,_dataOffset,_packedSize};
	MSBBitReader<BackwardInputStream> bitReader{inputStream};

	MemoryBuffer tmpBuffer(_rleSize);
	BackwardOutputStream outputStream{tmpBuffer,0,_rleSize};

	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBitsBE32(count);
	};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBitsBE32(1);
	};

	// anchor-bit handling
	{
		uint32_t value{inputStream.readBE32()};
		uint32_t tmp{value};
		uint32_t count{0};
		while (tmp)
		{
			tmp<<=1;
			count++;
		}
		if (count) count--;
		if (count)
			bitReader.reset(value>>(32U-count),count);
	}

	VariableLengthCodeDecoder countVlcDecoder{3,_modes[2]+1U,-1,0,0,0,_modes[1]+1U};
	std::array<uint8_t,7> distanceBits{0,0,0,8,9,10,uint8_t(_modes[0]+1U)};

	HuffmanDecoder<uint8_t> scDecoder
	{
		HuffmanCode{2,0b000,uint8_t{0x80}},
		HuffmanCode{3,0b010,uint8_t{0x81}},
		HuffmanCode{3,0b011,uint8_t{6}},
		HuffmanCode{2,0b010,uint8_t{3}},
		HuffmanCode{3,0b110,uint8_t{4}},
		HuffmanCode{3,0b111,uint8_t{5}}
	};

	while (!outputStream.eof())
	{
		uint8_t index{scDecoder.decode(readBit)};
		uint32_t count{countVlcDecoder.decode(readBits,index&0x7fU)};
		if (index&0x80U)
		{
			if (!count) count=8U;				// why?!?
			for (uint32_t i=0;i<count;i++)
				outputStream.writeByte(readBits(8));
		} else {
			uint32_t distance{readBits(distanceBits[index])};
			outputStream.copy(distance,count);
		}
	}

	ForwardInputStream finalInputStream{tmpBuffer,0,_rleSize};
	ForwardOutputStream finalOutputStream{rawData,0,_rawSize};
	while (!finalOutputStream.eof())
	{
		uint8_t cmd{finalInputStream.readByte()};
		if (cmd==_rle[0])
		{
			uint32_t count{uint32_t(finalInputStream.readByte())+1U};
			if (count==1) finalOutputStream.writeByte(cmd);
			else {
				uint8_t value{finalInputStream.readByte()};
				for (uint32_t i=0;i<=count;i++) finalOutputStream.writeByte(value);
			}
		} else if (cmd==_rle[1]) {
			uint32_t count{uint32_t(finalInputStream.readByte())+1U};
			if (count==1) finalOutputStream.writeByte(cmd);
			else {
				for (uint32_t i=0;i<=count;i++) finalOutputStream.writeByte(_rle[2]);
			}
		} else finalOutputStream.writeByte(cmd);
	}
}

// v2.92, v2.99
// v3.00
void StoneCrackerDecompressor::decompressGen23(Buffer &rawData)
{
	BackwardInputStream inputStream{_packedData,_dataOffset,_packedSize};
	LSBBitReader<BackwardInputStream> bitReader{inputStream};

	BackwardOutputStream outputStream{rawData,0,_rawSize};

	// anchor-bit handling
	{
		uint32_t value{inputStream.readBE32()};
		if (value) for (uint32_t i=31U;i>0;i--)
		{
			if (value&(1U<<i))
			{
				bitReader.reset(value&((1U<<i)-1U),i);
				break;
			}
		}
	}

	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBitsBE32(count);
	};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBitsBE32(1);
	};

	auto readCount=[&](uint32_t threshold,uint32_t bits)->uint32_t
	{
		uint32_t ret{0};
		uint32_t tmp;
		do
		{
			tmp=rotateBits(readBits(bits),bits);
			ret+=tmp;
		} while (tmp==threshold);
		return ret;
	};

	bool gen3{_generation>=3U};
	while (!outputStream.eof())
	{
		if (readBit())
		{
			uint32_t count{readCount(7U,3U)};
			if (gen3) count++;
			// for 2.92 zero count could meant count of 65536
			// for 2.99 it would be 4G
			// nevertheless it is an error
			// 3.00 fixes this by +1
			if (!count)
				throw DecompressionError();
			for (uint32_t i=0;i<count;i++)
				outputStream.writeByte(readBits(8));
		} else {
			uint32_t mode{rotateBits(readBits(2U),2U)};
			uint32_t count;
			uint32_t modeBits{_modes[mode]+1U};
			if (mode==3U)
			{
				if (readBit()) {
					count=readCount(7U,3U)+5U;
					if (gen3) modeBits=8U;
				} else count=readCount(127U,7U)+(gen3?5U:19U);
			} else {
				count=mode+2U;
			}
			uint32_t distance{rotateBits(readBits(modeBits),modeBits)+1U};
			outputStream.copy(distance,count);
		}
	}
}

// v3.10, v3.11b
// pads output file, and this new size is also defined as rawSize in header!
//
// pre v4.00, v4.01
void StoneCrackerDecompressor::decompressGen456(Buffer &rawData)
{
	ForwardInputStream inputStream{_packedData,_dataOffset,_packedSize};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};

	ForwardOutputStream outputStream{rawData,0,_rawSize};

	auto readBits=[&](uint32_t count)->uint32_t
	{
		if (_generation==4U) return bitReader.readBitsBE32(count);
			else return bitReader.readBitsBE16(count);
	};
	auto readBit=[&]()->uint32_t
	{
		return readBits(1);
	};

	auto readCount=[&](uint32_t threshold,uint32_t bits)->uint32_t
	{
		uint32_t ret{0};
		uint32_t tmp;
		do
		{
			tmp=readBits(bits);
			ret+=tmp;
		} while (tmp==threshold);
		return ret;
	};

	const std::array<uint8_t,4> modes{10U,11U,12U,8U};

	while (!outputStream.eof())
	{
		if (readBit())
		{
			uint32_t mode{readBits(2U)};
			uint32_t distance{readBits(modes[mode])};
			// will obviously throw if distance is 0
			uint32_t count;
			if (mode>=2U)
			{
				if (_generation==4) count=readCount(15U,4U)+3U;
					else count=readCount(7U,3U)+3U;
			} else {
				count=mode+3U;
			}
			// yet another bug
			if (count>_rawSize-uint32_t(outputStream.getOffset()))
				count=_rawSize-uint32_t(outputStream.getOffset());
			outputStream.copy(distance,count);
		} else {
			uint32_t count=readCount(7U,3U);
			// A regression in 3.10 that is not fixed until 4.01
			if (_generation==6U) count++;
			if (!count)
				throw DecompressionError();
			for (uint32_t i=0;i<count;i++)
				outputStream.writeByte(readBits(8));
		}
	}
}

// v4.02a
void StoneCrackerDecompressor::decompressGen7(Buffer &rawData)
{
	BackwardInputStream inputStream{_packedData,_dataOffset,_packedSize-2U};
	LSBBitReader<BackwardInputStream> bitReader{inputStream};

	BackwardOutputStream outputStream{rawData,0,_rawSize};

	// incomplete value at start
	{
		uint16_t bitCount{_packedData.readBE16(_packedSize-2U)};
		if (bitCount>16U)
			throw DecompressionError();
		uint16_t value{inputStream.readBE16()};
		bitReader.reset(value,bitCount&0xff);
	}

	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBitsBE16(count);
	};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBitsBE16(1);
	};

	VariableLengthCodeDecoder vlcDecoder{5,8,10,12};

	while (!outputStream.eof())
	{
		if (readBit())
		{
			uint32_t distance{vlcDecoder.decode(readBits,readBits(2U))+1U};
			// christmas tree!
			uint32_t count{2U};
			if (!readBit())
			{
				count++;
				if (!readBit())
				{
					count++;
					if (!readBit())
					{
						count++;
						uint32_t tmp;
						do
						{
							tmp=readBits(3U);
							count+=tmp;
						} while (tmp==7U);
					}
				}
			}
			outputStream.copy(distance,count);
		} else {
			outputStream.writeByte(readBits(8));
		}
	}
}

// v4.10
void StoneCrackerDecompressor::decompressGen8(Buffer &rawData)
{
	BackwardInputStream inputStream{_packedData,_dataOffset,_packedSize-2U};
	MSBBitReader<BackwardInputStream> bitReader{inputStream};

	BackwardOutputStream outputStream{rawData,0,_rawSize};

	// incomplete value at start
	uint16_t modeBits{0};
	{
		uint16_t bitCount{uint16_t(_packedData.readBE16(_packedSize-2U)&15U)};
		uint16_t value{inputStream.readBE16()};
		bitReader.reset(value>>(16U-bitCount),bitCount&0xff);

		modeBits=inputStream.readBE16();
		if (modeBits<10U || modeBits>14U)
			throw DecompressionError();
	}

	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBitsBE16(count);
	};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBitsBE16(1);
	};

	VariableLengthCodeDecoder countVlcDecoder{0,1,2,3,2,1,0,8,-3,2,0,5};
	VariableLengthCodeDecoder distanceVlcDecoder{5,9,modeBits};

	HuffmanDecoder<uint8_t> countDecoder
	{
		HuffmanCode{1,0b00000000,uint8_t{0x80}},
		HuffmanCode{2,0b00000011,uint8_t{1}},
		HuffmanCode{3,0b00000101,uint8_t{2}},
		HuffmanCode{4,0b00001000,uint8_t{7}},
		HuffmanCode{5,0b00010010,uint8_t{3}},
		HuffmanCode{6,0b00100110,uint8_t{4}},
		HuffmanCode{7,0b01001110,uint8_t{5}},
		HuffmanCode{8,0b10011110,uint8_t{6}},
		HuffmanCode{8,0b10011111,uint8_t{0x8bU}}
	};

	HuffmanDecoder<uint8_t> distanceDecoder
	{
		HuffmanCode{1,0b01,uint8_t{2}},
		HuffmanCode{2,0b00,uint8_t{1}},
		HuffmanCode{2,0b01,uint8_t{0}}
	};

	while (!outputStream.eof())
	{
		uint8_t countIndex{countDecoder.decode(readBit)};
		uint32_t count{countVlcDecoder.decode(readBits,countIndex&0x7fU)+1U};
		uint32_t tmp;
		if (count==278U) do
		{
			tmp=readBits(8U);
			count+=tmp;
		} while (tmp==0xffU);
		if (countIndex&0x80U)
		{
			for (uint32_t i=0;i<count;i++)
				outputStream.writeByte(readBits(8U));
		} else {
			uint32_t distance{distanceVlcDecoder.decode(readBits,distanceDecoder.decode(readBit))+1U};
			outputStream.copy(distance,count);
		}
	}
}

void StoneCrackerDecompressor::decompressImpl(Buffer &rawData,bool verify)
{
	if (rawData.size()<_rawSize)
		throw DecompressionError();
	if (!_rawSize) return;

	// The algorithms are all simple LZ compressors. However they will differ from version to version
	// on which order they read data, how much and on some other details
	// some of them can be nicely combined, others not so.
	//
	// To the creators credit, you can see generally improving performance as the versions increase
	// Although it is still limited by the simple nature of the implementation and does not really
	// compare to the more generic data compression algorithms
	switch (_generation)
	{
		case 1:
		decompressGen1(rawData);
		break;

		case 2:
		[[fallthrough]];
		case 3:
		decompressGen23(rawData);
		break;

		case 4:
		[[fallthrough]];
		case 5:
		[[fallthrough]];
		case 6:
		decompressGen456(rawData);
		break;

		case 7:
		decompressGen7(rawData);
		break;

		case 8:
		decompressGen8(rawData);
		break;

		default:
		throw DecompressionError();
	}
}

}
