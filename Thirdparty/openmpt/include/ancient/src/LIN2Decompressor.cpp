/* Copyright (C) Teemu Suutari */

#include "LIN2Decompressor.hpp"
#include "HuffmanDecoder.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"
#include "common/OverflowCheck.hpp"
#include "common/SubBuffer.hpp"

namespace ancient::internal
{

bool LIN2Decompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("LIN2") || hdr==FourCC("LIN4");
}

std::shared_ptr<XPKDecompressor> LIN2Decompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<LIN2Decompressor>(hdr,recursionLevel,packedData,state,verify);
}

LIN2Decompressor::LIN2Decompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
	_ver=(hdr==FourCC("LIN2"))?2U:4U;
	if (packedData.size()<10U)
		throw Decompressor::InvalidFormatError();

	uint32_t tmp{packedData.readBE32(0)};
	if (tmp)
		throw Decompressor::InvalidFormatError();	// password set

	// LIN4 is very similar to LIN2 - it only has 5 bit literals instead of 4 bit literals
	// (and thus larger table at the end of the stream)
	// Also, the huffman decoder for length is different

	_endStreamOffset=packedData.size()-1U;
	while (_endStreamOffset && _packedData[--_endStreamOffset]!=0xffU);
	// end stream
	// 1 byte, byte before 0xff
	// 0x10 bytes/0x20 for table
	if (_endStreamOffset<0x11U+0xaU)
		throw Decompressor::InvalidFormatError();
	_endStreamOffset-=(_ver==2U)?0x11U:0x21U;

	size_t midStreamOffset{_ver==2?0x16U:0x26U};
	// midstream
	// from endstream without
	// add 0x10/0x20 byte back to point after table
	// add 6 bytes to point to correct place

	tmp=packedData.readBE32(4U);
	if (OverflowCheck::sum(_endStreamOffset,midStreamOffset)<OverflowCheck::sum(tmp,10U) || tmp<midStreamOffset)
		throw Decompressor::InvalidFormatError();
	_midStreamOffset=_endStreamOffset-tmp+midStreamOffset;
}

const std::string &LIN2Decompressor::getSubName() const noexcept
{
	static std::string name2{"XPK-LIN2: LIN2 LINO packer"};
	static std::string name4{"XPK-LIN4: LIN4 LINO packer"};
	return (_ver==2)?name2:name4;
}

void LIN2Decompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	// three streams.
	// 1. ordinary bit stream out of words (readBits)
	// 2. bit stream for literals (readBit)
	// 3. nibble stream for literal (read4Bits)
	// at the end of the stream there is a literal table of 16/32 bytes
	// apart from confusing naming, there are also some nasty
	// interdependencies :(
	ForwardInputStream forwardInputStream{_packedData,10U,_midStreamOffset};
	ForwardInputStream middleInputStream{_packedData,_midStreamOffset,_endStreamOffset};
	BackwardInputStream backwardInputStream{_packedData,_midStreamOffset,_endStreamOffset};
	middleInputStream.link(backwardInputStream);
	backwardInputStream.link(middleInputStream);
	MSBBitReader<ForwardInputStream> bitsReader{forwardInputStream};
	MSBBitReader<ForwardInputStream> bitReader{middleInputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitsReader.readBits8(count);
	};

	{
		uint8_t tmp{middleInputStream.readByte()};
		if (tmp>8U)
			throw Decompressor::DecompressionError();
		bitReader.reset(middleInputStream.readByte()>>tmp,8U-tmp);
	}
	auto readBit=[&]()->uint8_t
	{
		return bitReader.readBits8(1U);
	};

	bool buf4Incomplete{false};
	uint8_t nibbleContent{0};
	{
		uint8_t tmp{_packedData.read8(9U)};
		buf4Incomplete=!!tmp;
		if (buf4Incomplete)
			nibbleContent=backwardInputStream.readByte();
	}
	// this is a rather strange thing...
	auto read4Bits=[&](bool fullByte)->uint8_t
	{
		if (!fullByte)
		{
			buf4Incomplete=!buf4Incomplete;
			if (!buf4Incomplete)
			{
				return nibbleContent&0xfU;
			} else {
				nibbleContent=backwardInputStream.readByte();
				return nibbleContent>>4U;
			}
		} else {
			if (buf4Incomplete)
			{
				uint8_t ret{uint8_t(nibbleContent&0xfU)};
				nibbleContent=backwardInputStream.readByte();
				ret|=nibbleContent&0xf0U;
				return ret;
			} else {
				return backwardInputStream.readByte();
			}
		}
	};

	const ConstSubBuffer literalTable{_packedData,_endStreamOffset,_packedData.size()-_endStreamOffset};

	size_t rawSize=rawData.size();
	ForwardOutputStream outputStream{rawData,0,rawSize};

	// little meh to initialize both (intentionally deleted copy/assign)
	HuffmanDecoder<uint8_t> lengthDecoder2
	{
		HuffmanCode{1,0b000000,uint8_t{3}},
		HuffmanCode{3,0b000100,uint8_t{4}},
		HuffmanCode{3,0b000101,uint8_t{5}},
		HuffmanCode{3,0b000110,uint8_t{6}},
		HuffmanCode{6,0b111000,uint8_t{7}},
		HuffmanCode{6,0b111001,uint8_t{8}},
		HuffmanCode{6,0b111010,uint8_t{9}},
		HuffmanCode{6,0b111011,uint8_t{10}},
		HuffmanCode{6,0b111100,uint8_t{11}},
		HuffmanCode{6,0b111101,uint8_t{12}},
		HuffmanCode{6,0b111110,uint8_t{13}},
		HuffmanCode{6,0b111111,uint8_t{0}}
	};

	HuffmanDecoder<uint8_t> lengthDecoder4
	{
		HuffmanCode{2,0b0000000,uint8_t{3}},
		HuffmanCode{2,0b0000001,uint8_t{4}},
		HuffmanCode{2,0b0000010,uint8_t{5}},
		HuffmanCode{4,0b0001100,uint8_t{6}},
		HuffmanCode{4,0b0001101,uint8_t{7}},
		HuffmanCode{4,0b0001110,uint8_t{8}},
		HuffmanCode{7,0b1111000,uint8_t{9}},
		HuffmanCode{7,0b1111001,uint8_t{10}},
		HuffmanCode{7,0b1111010,uint8_t{11}},
		HuffmanCode{7,0b1111011,uint8_t{12}},
		HuffmanCode{7,0b1111100,uint8_t{13}},
		HuffmanCode{7,0b1111101,uint8_t{14}},
		HuffmanCode{7,0b1111110,uint8_t{15}},
		HuffmanCode{7,0b1111111,uint8_t{0}}
	};
	auto &lengthDecoder=_ver==2?lengthDecoder2:lengthDecoder4;

	uint32_t minBits{1};

	while (!outputStream.eof())
	{
		if (!readBits(1U))
		{
			if (readBit())
			{
				outputStream.writeByte(read4Bits(true));
			} else {
				if (_ver==4U)
				{
					outputStream.writeByte(literalTable[(read4Bits(false)<<1U)+readBit()]);
				} else outputStream.writeByte(literalTable[read4Bits(false)]);
			}
		} else {
			uint32_t count=lengthDecoder.decode([&](){return readBits(1);});
			if (!count)
			{
				count=readBits(4U);
				if (count==0xfU)
				{
					count=readBits(8U);
					if (count!=0xffU) count+=3U;
						else throw Decompressor::DecompressionError();
				} else count+=(_ver==2U)?14U:16U;
			}

			uint32_t distance;
			bool isMax{false};
			do {
				uint32_t bits{readBits(3U)+minBits};
				distance=readBits(bits);
				isMax=(distance==((1U<<bits)-1U))&&(bits==minBits+7U);
				if (isMax) minBits++;
				distance+=(((1U<<bits)-1U)&~((1U<<minBits)-1U))+1U;
			} while (isMax);

			// buggy compressors
			count=std::min(count,uint32_t(rawSize-outputStream.getOffset()));
			if (!count)
				throw Decompressor::DecompressionError();

			outputStream.copy(distance,count);
		}
	}
}

}
