/* Copyright (C) Teemu Suutari */

#include "RAKEDecompressor.hpp"
#include "HuffmanDecoder.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"

#include <array>

namespace ancient::internal
{

bool RAKEDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return (hdr==FourCC("FRHT") || hdr==FourCC("RAKE"));
}

std::shared_ptr<XPKDecompressor> RAKEDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<RAKEDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

RAKEDecompressor::RAKEDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData},
	_isRAKE{hdr==FourCC("RAKE")}
{
	if (!detectHeaderXPK(hdr) || packedData.size()<4)
		throw Decompressor::InvalidFormatError();

	_midStreamOffset=packedData.readBE16(2);
	if (_midStreamOffset>=packedData.size())
		throw Decompressor::InvalidFormatError();
}

const std::string &RAKEDecompressor::getSubName() const noexcept
{
	static std::string nameFRHT{"XPK-FRHT: LZ77-compressor"};
	static std::string nameRAKE{"XPK-RAKE: LZ77-compressor"};
	return (_isRAKE)?nameRAKE:nameFRHT;
}

void RAKEDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	// 2 streams
	// 1st: bit stream starting from _midStreamOffset(+1) going to packedSize
	// 2nd: byte stream starting from _midStreamOffset going backwards to 4
	ForwardInputStream forwardInputStream{_packedData,_midStreamOffset+(_midStreamOffset&1),_packedData.size()};
	BackwardInputStream backwardInputStream{_packedData,4,_midStreamOffset};
	MSBBitReader<ForwardInputStream> bitReader{forwardInputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBitsBE32(count);
	};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBitsBE32(1);
	};
	auto readByte=[&]()->uint8_t
	{
		return backwardInputStream.readByte();
	};
	{
		uint16_t tmp{_packedData.readBE16(0)};
	        if (tmp>32)
	        	throw Decompressor::DecompressionError();
		uint32_t content{forwardInputStream.readBE32()};
		bitReader.reset(content>>tmp,32-tmp);
	}

	BackwardOutputStream outputStream{rawData,0,rawData.size()};

	HuffmanDecoder<uint8_t> lengthDecoder;
	// is there some logic into this?
	const std::array<std::array<uint8_t,2>,255> decTable{{
		{ 1,0x01},{ 3,0x03},{ 5,0x05},{ 6,0x09},{ 7,0x0c},{ 9,0x13},{12,0x34},{18,0xc0},
		{18,0xc2},{18,0xc3},{18,0xc6},{16,0x79},{18,0xc7},{18,0xd6},{18,0xd7},{18,0xd8},
		{17,0xa8},{17,0x92},{17,0x8a},{17,0x82},{16,0x6c},{17,0x94},{18,0xda},{18,0xca},
		{16,0x7b},{13,0x36},{13,0x39},{13,0x48},{14,0x49},{14,0x50},{15,0x62},{15,0x5e},
		{16,0x6f},{17,0x83},{17,0x87},{15,0x56},{11,0x21},{12,0x31},{13,0x38},{13,0x3d},
		{ 8,0x0f},{ 4,0x04},{ 6,0x08},{10,0x1c},{12,0x27},{13,0x42},{13,0x3a},{12,0x30},
		{12,0x32},{ 9,0x16},{ 8,0x11},{ 7,0x0b},{ 5,0x06},{10,0x19},{10,0x1a},{10,0x18},
		{11,0x26},{17,0x98},{17,0x99},{17,0x9b},{17,0x9e},{17,0x9f},{17,0xa6},{16,0x73},
		{17,0x7f},{17,0x81},{17,0x84},{17,0x85},{15,0x5d},{14,0x4d},{14,0x4f},{13,0x45},
		{13,0x3c},{ 9,0x17},{10,0x1d},{12,0xff},{13,0x41},{17,0x8c},{18,0xaa},{19,0xdb},
		{19,0xdc},{16,0x77},{15,0x63},{16,0x7c},{16,0x76},{16,0x71},{16,0x7d},{12,0x2c},
		{13,0x3b},{16,0x7a},{16,0x75},{15,0x55},{15,0x60},{16,0x74},{17,0xa4},{18,0xab},
		{18,0xac},{ 7,0x0a},{ 6,0x07},{ 9,0x15},{11,0x20},{11,0x24},{10,0x1b},{ 8,0x10},
		{ 9,0x12},{12,0x33},{14,0x4b},{15,0x53},{19,0xdd},{19,0xde},{18,0xad},{19,0xdf},
		{19,0xe0},{18,0xae},{17,0x88},{18,0xaf},{19,0xe1},{19,0xe2},{13,0x37},{12,0x2e},
		{18,0xb0},{18,0xb1},{19,0xe3},{19,0xe4},{18,0xb2},{18,0xb3},{19,0xe5},{19,0xe6},
		{19,0xe7},{19,0xe8},{18,0xb4},{17,0x9a},{18,0xb5},{18,0xb6},{18,0xb7},{19,0xe9},
		{19,0xea},{18,0xb8},{19,0xeb},{19,0xec},{19,0xed},{19,0xee},{18,0xb9},{19,0xef},
		{19,0xf0},{18,0xbb},{18,0xbc},{19,0xf1},{19,0xf2},{18,0xbd},{18,0xbe},{19,0xf3},
		{19,0xf4},{18,0xbf},{18,0xc1},{19,0xf5},{19,0xf6},{18,0xc4},{18,0xc5},{17,0x95},
		{18,0xc8},{18,0xc9},{19,0xf7},{19,0xf8},{18,0xcb},{18,0xcc},{19,0xf9},{19,0xfa},
		{18,0xcd},{18,0xce},{17,0x96},{18,0xcf},{18,0xd0},{19,0xfb},{19,0xfc},{18,0xd1},
		{18,0xd2},{18,0xd3},{17,0x9c},{17,0x9d},{18,0xd4},{18,0xd5},{17,0xa0},{17,0xa1},
		{17,0xa2},{17,0xa3},{17,0xa5},{19,0xfd},{19,0xfe},{18,0xd9},{17,0xa7},{16,0x66},
		{15,0x54},{15,0x57},{16,0x6b},{16,0x68},{14,0x4c},{14,0x4e},{12,0x28},{11,0x23},
		{ 8,0x0e},{ 7,0x0d},{10,0x1f},{13,0x47},{15,0x64},{15,0x58},{15,0x59},{15,0x5a},
		{12,0x29},{13,0x3e},{15,0x5f},{17,0x8e},{18,0xba},{18,0xa9},{16,0x70},{14,0x4a},
		{12,0x2a},{ 9,0x14},{11,0x22},{12,0x2f},{16,0x7e},{16,0x67},{16,0x69},{16,0x65},
		{15,0x51},{16,0x78},{16,0x6a},{13,0x46},{11,0x25},{16,0x72},{16,0x6e},{15,0x5b},
		{15,0x61},{15,0x52},{13,0x40},{13,0x43},{13,0x44},{13,0x3f},{15,0x5c},{17,0x93},
		{17,0x80},{17,0x8d},{17,0x8b},{17,0x86},{17,0x89},{17,0x97},{17,0x8f},{17,0x90},
		{17,0x91},{16,0x6d},{12,0x2b},{12,0x2d},{12,0x35},{10,0x1e},{ 3,0x02}}};

	uint32_t hufCode{0};
	for (auto &it: decTable)
	{
		lengthDecoder.insert(HuffmanCode{it[0],hufCode>>(32-it[0]),it[1]});
		hufCode+=1<<(32-it[0]);
	}

	while (!outputStream.eof())
	{
		if (!readBit())
		{
			outputStream.writeByte(readByte());
		} else {
			uint32_t count{lengthDecoder.decode(readBit)};
			count+=2;

			uint32_t distance;
			if (!readBit())
			{
				distance=uint32_t(readByte())+1;
			} else {
				if (!readBit())
				{
					distance=((readBits(3)<<8)|uint32_t(readByte()))+0x101;
				} else {
					distance=((readBits(6)<<8)|uint32_t(readByte()))+0x901;
				}
			}
			outputStream.copy(distance,count);
		}
	}
}

}
