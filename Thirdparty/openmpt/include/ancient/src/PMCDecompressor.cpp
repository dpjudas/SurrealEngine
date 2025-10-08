/* Copyright (C) Teemu Suutari */

#include "PMCDecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "DynamicHuffmanDecoder.hpp"
#include "VariableLengthCodeDecoder.hpp"
#include "common/Common.hpp"
#include "common/OverflowCheck.hpp"
#include "common/SubBuffer.hpp"
#include "LHDecompressor.hpp"
#include "DLTADecode.hpp"

namespace ancient::internal
{

bool PMCDecompressor::detectHeader(uint32_t hdr,uint32_t footer) noexcept
{
	return (hdr==FourCC("SFHD"))||(hdr==FourCC("SFCD"));
}

std::shared_ptr<Decompressor> PMCDecompressor::create(const Buffer &packedData,bool exactSizeKnown,bool verify)
{
	return std::make_shared<PMCDecompressor>(packedData,verify);
}

PMCDecompressor::PMCDecompressor(const Buffer &packedData,bool verify) :
	_packedData{packedData}
{
	uint32_t hdr{packedData.readBE32(0)};
	if (!detectHeader(hdr,0) || packedData.size()<12)
		throw InvalidFormatError();
	_ver=(hdr==FourCC("SFHD"))?0:1;

	_rawSize=packedData.readBE32(4U);
	if (!_rawSize || _rawSize>getMaxRawSize())
		throw InvalidFormatError();
	_packedSize= OverflowCheck::sum(packedData.readBE32(8U),12U);
	if (!_packedSize || _packedSize>packedData.size() || _packedSize>getMaxPackedSize())
		throw InvalidFormatError();
}

const std::string &PMCDecompressor::getName() const noexcept
{
	static std::string names[2]={
		{"PMC: PowerPlayer Music Compressor 1.x"},
		{"PMC: PowerPlayer Music Compressor 2.x"}};
	return names[_ver];
}

size_t PMCDecompressor::getPackedSize() const noexcept
{
	return _packedSize;
}

size_t PMCDecompressor::getRawSize() const noexcept
{
	return _rawSize;
}

void PMCDecompressor::decompressImpl(Buffer &rawData,bool verify)
{
	// thats all folks!
	ConstSubBuffer subPackedData(_packedData,12,_packedSize-12);

	LHDecompressor::decompressLhLib(rawData,subPackedData);
	if (_ver) DLTADecode::decode(rawData,rawData,0,_rawSize);
}

}
