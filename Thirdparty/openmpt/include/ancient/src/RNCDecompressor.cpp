/* Copyright (C) Teemu Suutari */

#include <algorithm>

#include "RNCDecompressor.hpp"
#include "HuffmanDecoder.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/CRC16.hpp"
#include "common/OverflowCheck.hpp"
#include "common/Common.hpp"

#include "VariableLengthCodeDecoder.hpp"

#include <array>

// This allows decompression of pc compressed files from unonfficial (and unpatched) compressor
// PC games do not need chunk count, and are happy to read these files.
// Official tools put it and amiga decompressors require it
#define ALLOW_MISSING_CHUNKS 1

namespace ancient::internal
{

bool RNCDecompressor::detectHeader(uint32_t hdr,uint32_t footer) noexcept
{
	return hdr==FourCC("RNC\001") || hdr==FourCC("RNC\002")
		|| hdr==FourCC("...\001");		// Total Carnage
}

std::shared_ptr<Decompressor> RNCDecompressor::create(const Buffer &packedData,bool exactSizeKnown,bool verify)
{
	return std::make_shared<RNCDecompressor>(packedData,verify);
}

RNCDecompressor::RNCDecompressor(const Buffer &packedData,bool verify) :
	_packedData{packedData}
{
	uint32_t hdr{packedData.readBE32(0)};
	_rawSize=packedData.readBE32(4);
	_packedSize=packedData.readBE32(8);
	if (!_rawSize || !_packedSize ||
		_rawSize>getMaxRawSize() || _packedSize>getMaxPackedSize())
			throw InvalidFormatError();

	bool verified{false};
	if (hdr==FourCC("RNC\001"))
	{
		// now detect between old and new version
		// since the old and the new version share the same id, there is no foolproof way
		// to tell them apart. It is easier to prove that it is not something by finding
		// specific invalid bitstream content.

		// well, this is silly though but lets assume someone has made old format RNC1 with total size less than 19
		if (packedData.size()<=18U)
		{
			_ver=Version::RNC1Old;
		} else {
			uint8_t newStreamStart{packedData.read8(18U)};
			uint8_t oldStreamStart{packedData.read8(_packedSize+11U)};

			// Check that stream starts with a literal(s)
			if (!(oldStreamStart&0x80U))
				_ver=Version::RNC1New;
			// New stream have two bits in start as a filler on new stream. Those are always 0
			// (although this is not strictly mandated)
			// +
			// Even though it is possible to make new RNC1 stream which starts with zero literal table size,
			// it is extremely unlikely
			else if ((newStreamStart&3U) || !(newStreamStart&0x7cU))
				_ver=Version::RNC1Old;
			// now the last resort: check CRC.
			else if (_packedData.size()>=OverflowCheck::sum(_packedSize,18U) && CRC16(_packedData,18U,_packedSize,0)==packedData.readBE16(14U))
			{
				_ver=Version::RNC1New;
				verified=true;
			} else _ver=Version::RNC1Old;
		}
	} else if (hdr==FourCC("RNC\002")) {
		// ...and detect between the new and old format of RNC2
		if (packedData.size()<=18U)
		{
			_ver=Version::RNC2Old;
		} else {
			// RNC2Old is very similar to RNC1Old, RNC2 has padding at start which makes things more complex
			uint8_t newStreamStart{packedData.read8(18U)};
			uint8_t oldStreamStart{packedData.read8(_packedSize+10U)};

			// Check that stream starts with a literal(s)
			if (!(oldStreamStart&0x80U))
				_ver=Version::RNC2New;
			// First command needs to be LIT/MOV! (after 2 ballast bits)
			else if ((newStreamStart&0x30U)==0x30U)
				_ver=Version::RNC2Old;
			// now the last resort: check CRC.
			else if (_packedData.size()>=OverflowCheck::sum(_packedSize,18U) && CRC16(_packedData,18U,_packedSize,0)==packedData.readBE16(14U))
			{
				_ver=Version::RNC2New;
				verified=true;
			} else _ver=Version::RNC2Old;
		}
	} else if (hdr==FourCC("...\001")) {
		_ver=Version::RNC1New;
	} else throw InvalidFormatError();

	uint32_t hdrSize{(_ver==Version::RNC1Old || _ver==Version::RNC2Old)?12U:18U};
	if (OverflowCheck::sum(_packedSize,hdrSize)>packedData.size())
		throw InvalidFormatError();

	if (_ver!=Version::RNC1Old && _ver!=Version::RNC2Old)
	{
		_rawCRC=packedData.readBE16(12U);
		_chunks=packedData.read8(17U);
		if (verify && !verified)
		{
			if (CRC16(_packedData,18,_packedSize,0)!=packedData.readBE16(14))
				throw VerificationError();
		}
	}
}

const std::string &RNCDecompressor::getName() const noexcept
{
	static std::string names[4]={
		{"RNC1: Rob Northen RNC1 Compressor (old)"},
		{"RNC1: Rob Northen RNC1 Compressor"},
		{"RNC2: Rob Northen RNC2 Compressor (old)"},
		{"RNC2: Rob Northen RNC2 Compressor"}};
	return names[static_cast<uint32_t>(_ver)];
}

size_t RNCDecompressor::getPackedSize() const noexcept
{
	if (_ver==Version::RNC1Old || _ver==Version::RNC2Old) return _packedSize+12U;
		else return _packedSize+18U;
}

size_t RNCDecompressor::getRawSize() const noexcept
{
	return _rawSize;
}

void RNCDecompressor::decompressImpl(Buffer &rawData,bool verify)
{
	if (rawData.size()<_rawSize)
		throw DecompressionError();

	switch (_ver)
	{
		case Version::RNC1Old:
		return RNCDecompressOld(rawData,verify,false);

		case Version::RNC1New:
		return RNC1DecompressNew(rawData,verify);

		case Version::RNC2Old:
		return RNCDecompressOld(rawData,verify,true);

		case Version::RNC2New:
		return RNC2DecompressNew(rawData,verify);

		default:
		throw DecompressionError();
	}
}

void RNCDecompressor::RNCDecompressOld(Buffer &rawData,bool verify,bool rnc2)
{
	BackwardInputStream inputStream{_packedData,12U,_packedSize+12U};
	MSBBitReader<BackwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBits8(count);
	};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBits8(1);
	};
	auto readByte=[&]()->uint8_t
	{
		return inputStream.readByte();
	};

	uint32_t lastDistanceBits{12};
	uint32_t lastLengthBits{10};
	if (rnc2)
	{
		uint32_t tmp=readByte()+1U;
		lastDistanceBits=tmp&0xfU;
		lastLengthBits=(tmp>>4U)+1U;
	}

	// the anchor-bit does not seem always to be at the correct place
	{
		uint8_t halfByte{readByte()};
		for (uint32_t i=0;i<7;i++)
			if (halfByte&(1<<i))
			{
				bitReader.reset(halfByte>>(i+1),7-i);
				break;
			}
	}

	BackwardOutputStream outputStream{rawData,0,_rawSize};

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
		HuffmanCode{1,0b00,uint8_t{1}},
		HuffmanCode{2,0b10,uint8_t{0}},
		HuffmanCode{2,0b11,uint8_t{2}}
	};

	VariableLengthCodeDecoder litVlcDecoder1{1,1,2,2,3,10};
	VariableLengthCodeDecoder litVlcDecoder2{1,1,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
	VariableLengthCodeDecoder lengthVlcDecoder{0,0,1,2,lastLengthBits};
	VariableLengthCodeDecoder distanceVlcDecoder{5,8,lastDistanceBits};

	for (;;)
	{
		uint32_t litLength{rnc2?litVlcDecoder2.decodeCascade(readBits):litVlcDecoder1.decodeCascade(readBits)};
		for (uint32_t i=0;i<litLength;i++) outputStream.writeByte(readByte());
	
		// the only way to successfully end the loop!
		if (outputStream.eof()) break;

		uint32_t count{lengthVlcDecoder.decode(readBits,lengthDecoder.decode(readBit))+2U};

		uint32_t distance;
		if (count!=2U)
		{
			distance=distanceVlcDecoder.decode(readBits,distanceDecoder.decode(readBit));
		} else {
			if (!readBit()) distance=readBits(6U);
				else distance=readBits(9U)+64U;
		}

		outputStream.copy((distance)?distance+count-1U:1U,count);
	}
}

void RNCDecompressor::RNC1DecompressNew(Buffer &rawData,bool verify)
{
	ForwardInputStream inputStream{_packedData,18U,_packedSize+18U,1U};
	LSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBitsLE16(count);
	};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBitsLE16(1);
	};
	auto readByte=[&]()->uint8_t
	{
		return inputStream.readByte();
	};

	ForwardOutputStream outputStream{rawData,0,_rawSize};

	typedef HuffmanDecoder<uint32_t> RNC1HuffmanDecoder;

	// helpers
	auto readHuffmanTable=[&](RNC1HuffmanDecoder &dec)
	{
		uint32_t length{readBits(5)};
		if (!length) return;
		std::array<uint8_t,31> lengthTable;
		for (uint32_t i=0;i<length;i++)
			lengthTable[i]=readBits(4);

		dec.createOrderlyHuffmanTable(lengthTable,length);
	};

	auto huffmanDecode=[&](const RNC1HuffmanDecoder &dec)->uint32_t
	{
		// this is kind of non-specced
		uint32_t ret{dec.decode(readBit)};
		if (ret>=2U)
			ret=(1U<<(ret-1U))|readBits(ret-1U);
		return ret;
	};

	auto processLiterals=[&](const RNC1HuffmanDecoder &dec)
	{
		uint32_t litLength{huffmanDecode(dec)};
		for (uint32_t i=0;i<litLength;i++) outputStream.writeByte(readByte());
	};

	readBits(2);
#ifdef ALLOW_MISSING_CHUNKS
	while (!outputStream.eof())
#else
	for (uint8_t chunks=0;chunks<_chunks;chunks++)
#endif
	{
		RNC1HuffmanDecoder litDecoder,distanceDecoder,lengthDecoder;
		readHuffmanTable(litDecoder);
		readHuffmanTable(distanceDecoder);
		readHuffmanTable(lengthDecoder);
		uint32_t count{readBits(16)};

		for (uint32_t sub=1;sub<count;sub++)
		{
			processLiterals(litDecoder);
			uint32_t distance{huffmanDecode(distanceDecoder)};
			uint32_t subCount{huffmanDecode(lengthDecoder)};
			distance++;
			subCount+=2;
			outputStream.copy(distance,subCount);
		}
		processLiterals(litDecoder);
	}

	if (!outputStream.eof())
		throw DecompressionError();
	if (verify && CRC16(rawData,0,_rawSize,0)!=_rawCRC)
		throw VerificationError();
}

void RNCDecompressor::RNC2DecompressNew(Buffer &rawData,bool verify)
{
	ForwardInputStream inputStream{_packedData,18U,_packedSize+18U};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBits8(count);
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

	// Huffman decoding
	enum class Cmd
	{
		LIT=0,	// 0, Literal
		MOV,	// 10, Move bytes + length + distance, Get bytes if length=9 + 4bits
		MV2,	// 110, Move 2 bytes
		MV3,	// 1110, Move 3 bytes
		CND	// 1111, Conditional copy, or EOF
		
	};

	HuffmanDecoder<Cmd> cmdDecoder
	{
		HuffmanCode{1,0b0000,Cmd::LIT},
		HuffmanCode{2,0b0010,Cmd::MOV},
		HuffmanCode{3,0b0110,Cmd::MV2},
		HuffmanCode{4,0b1110,Cmd::MV3},
		HuffmanCode{4,0b1111,Cmd::CND}
	};

	/* length of 9 is a marker for literals */
	HuffmanDecoder<uint8_t> lengthDecoder
	{
		HuffmanCode{2,0b000,uint8_t{4}},
		HuffmanCode{2,0b010,uint8_t{5}},
		HuffmanCode{3,0b010,uint8_t{6}},
		HuffmanCode{3,0b011,uint8_t{7}},
		HuffmanCode{3,0b110,uint8_t{8}},
		HuffmanCode{3,0b111,uint8_t{9}}
	};
	
	HuffmanDecoder<uint8_t> distanceDecoder
	{
		HuffmanCode{1,0b000000,uint8_t{0}},
		HuffmanCode{3,0b000110,uint8_t{1}},
		HuffmanCode{4,0b001000,uint8_t{2}},
		HuffmanCode{4,0b001001,uint8_t{3}},
		HuffmanCode{5,0b010101,uint8_t{4}},
		HuffmanCode{5,0b010111,uint8_t{5}},
		HuffmanCode{5,0b011101,uint8_t{6}},
		HuffmanCode{5,0b011111,uint8_t{7}},
		HuffmanCode{6,0b101000,uint8_t{8}},
		HuffmanCode{6,0b101001,uint8_t{9}},
		HuffmanCode{6,0b101100,uint8_t{10}},
		HuffmanCode{6,0b101101,uint8_t{11}},
		HuffmanCode{6,0b111000,uint8_t{12}},
		HuffmanCode{6,0b111001,uint8_t{13}},
		HuffmanCode{6,0b111100,uint8_t{14}},
		HuffmanCode{6,0b111101,uint8_t{15}}
	};

	// helpers
	auto readDistance=[&]()->uint32_t
	{
		uint8_t distMult{distanceDecoder.decode(readBit)};
		uint8_t distByte{readByte()};
		return (uint32_t(distByte)|(uint32_t(distMult)<<8))+1;
	};
	
	auto moveBytes=[&](uint32_t distance,uint32_t count)->void
	{
		if (!count)
			throw DecompressionError();
		outputStream.copy(distance,count);
	};

	readBit();
	readBit();
	uint8_t foundChunks{0};
	bool done{false};
	while (!done && foundChunks<_chunks)
	{
		Cmd cmd{cmdDecoder.decode(readBit)};
		switch (cmd) {
			case Cmd::LIT:
			outputStream.writeByte(readByte());
			break;

			case Cmd::MOV:
			{
				uint8_t count{lengthDecoder.decode(readBit)};
				if (count!=9)
					moveBytes(readDistance(),count);
				else {
					uint32_t rep{(readBits(4U)+3U)*4U};
					for (uint32_t i=0;i<rep;i++)
						outputStream.writeByte(readByte());
				}
			}
			break;

			case Cmd::MV2:
			moveBytes(uint32_t{readByte()}+1U,2U);
			break;

			case Cmd::MV3:
			moveBytes(readDistance(),3U);
			break;

			case Cmd::CND:
			{
				uint8_t count{readByte()};
				if (count)
					moveBytes(readDistance(),uint32_t{count}+8U);
				else {
					foundChunks++;
					done=!readBit();
				}
				
			}			
			break;
		}
	}

	if (!outputStream.eof() || _chunks!=foundChunks)
		throw DecompressionError();
	if (verify && CRC16(rawData,0,_rawSize,0)!=_rawCRC)
		throw VerificationError();
}

}
