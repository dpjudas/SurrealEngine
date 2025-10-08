/* Copyright (C) Teemu Suutari */

#include <cstring>
#include <memory>
#include <algorithm>

#include "common/SubBuffer.hpp"
#include "common/OverflowCheck.hpp"
#include "common/Common.hpp"
#include "XPKMain.hpp"
#include "XPKDecompressor.hpp"

#include "ACCADecompressor.hpp"
#include "ARTMDecompressor.hpp"
#include "BLZWDecompressor.hpp"
#include "BZIP2Decompressor.hpp"
#include "CBR0Decompressor.hpp"
#include "CRMDecompressor.hpp"
#include "CYB2Decoder.hpp"
#include "DEFLATEDecompressor.hpp"
#include "DLTADecode.hpp"
#include "FASTDecompressor.hpp"
#include "FBR2Decompressor.hpp"
#include "FRLEDecompressor.hpp"
#include "HFMNDecompressor.hpp"
#include "HUFFDecompressor.hpp"
#include "ILZRDecompressor.hpp"
#include "IMPDecompressor.hpp"
#include "LHDecompressor.hpp"
#include "LIN1Decompressor.hpp"
#include "LIN2Decompressor.hpp"
#include "LZBSDecompressor.hpp"
#include "LZCBDecompressor.hpp"
#include "LZW2Decompressor.hpp"
#include "LZW4Decompressor.hpp"
#include "LZW5Decompressor.hpp"
#include "LZXDecompressor.hpp"
#include "MASHDecompressor.hpp"
#include "NONEDecompressor.hpp"
#include "NUKEDecompressor.hpp"
#include "PPDecompressor.hpp"
#include "PPMQDecompressor.hpp"
#include "RAKEDecompressor.hpp"
#include "RDCNDecompressor.hpp"
#include "RLENDecompressor.hpp"
#include "SDHCDecompressor.hpp"
#include "SHRXDecompressor.hpp"
#include "SLZ3Decompressor.hpp"
#include "SMPLDecompressor.hpp"
#include "SQSHDecompressor.hpp"
#include "SXSCDecompressor.hpp"
#include "TDCSDecompressor.hpp"
#include "ZENODecompressor.hpp"
#include "XPKUnimplemented.hpp"

#include <array>

namespace ancient::internal
{

bool XPKMain::detectHeader(uint32_t hdr,uint32_t footer) noexcept
{
	return hdr==FourCC("XPKF");
}

std::shared_ptr<Decompressor> XPKMain::create(const Buffer &packedData,bool verify,bool exactSizeKnown)
{
	return std::shared_ptr<Decompressor>{new XPKMain{packedData,verify,0}};
}

static std::vector<std::pair<bool(*)(uint32_t),std::shared_ptr<XPKDecompressor>(*)(uint32_t,uint32_t,const Buffer&,std::shared_ptr<XPKDecompressor::State>&,bool)>> XPKDecompressors={
	{ACCADecompressor::detectHeaderXPK,ACCADecompressor::create},
	{ARTMDecompressor::detectHeaderXPK,ARTMDecompressor::create},
	{BLZWDecompressor::detectHeaderXPK,BLZWDecompressor::create},
	{BZIP2Decompressor::detectHeaderXPK,BZIP2Decompressor::create},
	{CBR0Decompressor::detectHeaderXPK,CBR0Decompressor::create},
	{CRMDecompressor::detectHeaderXPK,CRMDecompressor::create},
	{CYB2Decoder::detectHeaderXPK,CYB2Decoder::create},
	{DEFLATEDecompressor::detectHeaderXPK,DEFLATEDecompressor::create},
	{DLTADecode::detectHeaderXPK,DLTADecode::create},
	{FASTDecompressor::detectHeaderXPK,FASTDecompressor::create},
	{FBR2Decompressor::detectHeaderXPK,FBR2Decompressor::create},
	{FRLEDecompressor::detectHeaderXPK,FRLEDecompressor::create},
	{HFMNDecompressor::detectHeaderXPK,HFMNDecompressor::create},
	{HUFFDecompressor::detectHeaderXPK,HUFFDecompressor::create},
	{ILZRDecompressor::detectHeaderXPK,ILZRDecompressor::create},
	{IMPDecompressor::detectHeaderXPK,IMPDecompressor::create},
	{LHDecompressor::detectHeaderXPK,LHDecompressor::create},
	{LIN1Decompressor::detectHeaderXPK,LIN1Decompressor::create},
	{LIN2Decompressor::detectHeaderXPK,LIN2Decompressor::create},
	{LZBSDecompressor::detectHeaderXPK,LZBSDecompressor::create},
	{LZCBDecompressor::detectHeaderXPK,LZCBDecompressor::create},
	{LZW2Decompressor::detectHeaderXPK,LZW2Decompressor::create},
	{LZW4Decompressor::detectHeaderXPK,LZW4Decompressor::create},
	{LZW5Decompressor::detectHeaderXPK,LZW5Decompressor::create},
	{LZXDecompressor::detectHeaderXPK,LZXDecompressor::create},
	{MASHDecompressor::detectHeaderXPK,MASHDecompressor::create},
	{NONEDecompressor::detectHeaderXPK,NONEDecompressor::create},
	{NUKEDecompressor::detectHeaderXPK,NUKEDecompressor::create},
	{PPDecompressor::detectHeaderXPK,PPDecompressor::create},
	{PPMQDecompressor::detectHeaderXPK,PPMQDecompressor::create},
	{RAKEDecompressor::detectHeaderXPK,RAKEDecompressor::create},
	{RDCNDecompressor::detectHeaderXPK,RDCNDecompressor::create},
	{RLENDecompressor::detectHeaderXPK,RLENDecompressor::create},
	{SDHCDecompressor::detectHeaderXPK,SDHCDecompressor::create},
	{SHRXDecompressor::detectHeaderXPK,SHRXDecompressor::create},
	{SLZ3Decompressor::detectHeaderXPK,SLZ3Decompressor::create},
	{SMPLDecompressor::detectHeaderXPK,SMPLDecompressor::create},
	{SQSHDecompressor::detectHeaderXPK,SQSHDecompressor::create},
	{SXSCDecompressor::detectHeaderXPK,SXSCDecompressor::create},
	{TDCSDecompressor::detectHeaderXPK,TDCSDecompressor::create},
	{ZENODecompressor::detectHeaderXPK,ZENODecompressor::create},
	{XPKUnimplemented::detectHeaderXPK,XPKUnimplemented::create}};

XPKMain::XPKMain(const Buffer &packedData,bool verify,uint32_t recursionLevel) :
	_packedData{packedData}
{
	if (recursionLevel>=getMaxRecursionLevel())
		throw InvalidFormatError();
	if (packedData.size()<44U)
		throw InvalidFormatError();
	uint32_t hdr{packedData.readBE32(0)};
	if (!detectHeader(hdr,0))
		throw InvalidFormatError();

	_packedSize=packedData.readBE32(4U);
	_type=packedData.readBE32(8U);
	_rawSize=packedData.readBE32(12U);

	if (!_rawSize || !_packedSize)
		throw InvalidFormatError();
	if (_rawSize>getMaxRawSize() || _packedSize>getMaxPackedSize())
		throw InvalidFormatError();

	uint8_t flags{packedData.read8(32U)};
	_longHeaders=(flags&1U)?true:false;
	if (flags&2U) _hasPassword=true;		// Late failure so we can identify format
	if (flags&4U)					// extra header
	{
		_headerSize=38U+uint32_t(packedData.readBE16(36U));
	} else {
		_headerSize=36U;
	}

	if (OverflowCheck::sum(_packedSize,8U)>packedData.size())
		throw InvalidFormatError();

	bool found=false;
	for (auto &it : XPKDecompressors)
	{
		if (it.first(_type)) 
		{
			found=true;
			break;
		}
	}
	if (!found)
		throw InvalidFormatError();

	auto headerChecksum=[](const Buffer &buffer,size_t offset,size_t len)->bool
	{
		if (!len || OverflowCheck::sum(offset,len)>buffer.size()) return false;
		uint8_t tmp{0};
		for (size_t i=0;i<len;i++)
			tmp^=buffer[offset+i];
		return !tmp;
	};

	// this implementation assumes align padding is zeros
	auto chunkChecksum=[](const Buffer &buffer,size_t offset,size_t len,uint16_t checkValue)->bool
	{
		if (!len || OverflowCheck::sum(offset,len)>buffer.size()) return false;
		std::array<uint8_t,2> tmp{0,0};
		for (size_t i=0;i<len;i++)
			tmp[i&1]^=buffer[offset+i];
		return tmp[0]==(checkValue>>8) && tmp[1]==(checkValue&0xff);
	};


	if (verify)
	{
		if (!headerChecksum(_packedData,0,36U))
			throw VerificationError();

		std::shared_ptr<XPKDecompressor::State> state;
		forEachChunk([&](const Buffer &header,const Buffer &chunk,uint32_t rawChunkSize,uint8_t chunkType)->bool
		{
			if (!headerChecksum(header,0,header.size()))
				throw VerificationError();

			uint16_t hdrCheck{header.readBE16(2U)};
			if (chunk.size() && !chunkChecksum(chunk,0,chunk.size(),hdrCheck))
				throw VerificationError();

			if (chunkType==1U)
			{
				auto sub=createDecompressor(_type,_recursionLevel,chunk,state,true);
			} else if (chunkType!=0 && chunkType!=15U)
				throw InvalidFormatError();
			return true;
		});
	}
}

const std::string &XPKMain::getName() const noexcept
{
	std::shared_ptr<XPKDecompressor> sub;
	std::shared_ptr<XPKDecompressor::State> state;
	try
	{
		forEachChunk([&](const Buffer &header,const Buffer &chunk,uint32_t rawChunkSize,uint8_t chunkType)->bool
		{
			try
			{
				sub=createDecompressor(_type,_recursionLevel,chunk,state,false);
			} catch (const Error&) {
				// should not happen since the code is already tried out,
				// however, lets handle the case gracefully
			}
			return false;
		});
	} catch (const Buffer::Error&) {
		// ditto
	}
	static std::string invName{"<invalid>"};
	return (sub)?sub->getSubName():invName;
}

size_t XPKMain::getPackedSize() const noexcept
{
	return _packedSize+8U;
}

size_t XPKMain::getRawSize() const noexcept
{
	return _rawSize;
}

void XPKMain::decompressImpl(Buffer &rawData,bool verify)
{
	if (rawData.size()<_rawSize)
		throw DecompressionError();
	if (_hasPassword)
		throw DecompressionError();

	uint32_t destOffset{0};
	std::shared_ptr<XPKDecompressor::State> state;
	forEachChunk([&](const Buffer &header,const Buffer &chunk,uint32_t rawChunkSize,uint8_t chunkType)->bool
	{
		if (OverflowCheck::sum(destOffset,rawChunkSize)>rawData.size())
			throw DecompressionError();
		if (!rawChunkSize) return true;

		ConstSubBuffer previousBuffer{rawData,0,destOffset};
		SubBuffer DestBuffer{rawData,destOffset,rawChunkSize};
		switch (chunkType)
		{
			case 0:
			if (rawChunkSize!=chunk.size())
				throw DecompressionError();;
			std::memcpy(DestBuffer.data(),chunk.data(),rawChunkSize);
			break;

			case 1:
			{
				try
				{
					auto sub{createDecompressor(_type,_recursionLevel,chunk,state,false)};
					sub->decompressImpl(DestBuffer,previousBuffer,verify);
				} catch (const InvalidFormatError&) {
					// we should throw a correct error
					throw DecompressionError();
				}
			}
			break;

			case 15U:
			break;
			
			default:
			return false;
		}

		destOffset+=rawChunkSize;
		return true;
	});

	if (destOffset!=_rawSize)
		throw DecompressionError();

	if (verify)
	{
		if (std::memcmp(_packedData.data()+16U,rawData.data(),std::min(_rawSize,16U)))
			throw DecompressionError();
	}
}

std::shared_ptr<Decompressor> XPKMain::createDecompressor(uint32_t recursionLevel,const Buffer &buffer,bool verify)
{
	return std::shared_ptr<Decompressor>{new XPKMain{buffer,verify,recursionLevel+1U}};
}

std::shared_ptr<XPKDecompressor> XPKMain::createDecompressor(uint32_t type,uint32_t recursionLevel,const Buffer &buffer,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	for (auto &it : XPKDecompressors)
	{
		if (it.first(type)) return it.second(type,recursionLevel,buffer,state,verify);
	}
	throw InvalidFormatError();
}

template <typename F>
void XPKMain::forEachChunk(F func) const
{
	uint32_t currentOffset{0};
	uint32_t rawSize,packedSize;
	bool isLast{false};

	while (currentOffset<_packedSize+8U && !isLast)
	{
		auto readDualValue=[&](uint32_t offsetShort,uint32_t offsetLong,uint32_t &value)
		{
			if (_longHeaders)
			{
				value=_packedData.readBE32(currentOffset+offsetLong);
			} else {
				value=uint32_t(_packedData.readBE16(currentOffset+offsetShort));
			}
		};

		uint32_t chunkHeaderLen{_longHeaders?12U:8U};
		if (!currentOffset)
		{
			// return first;
			currentOffset=_headerSize;
		} else {
			uint32_t tmp;
			readDualValue(4U,4U,tmp);
			tmp=((tmp+3U)&~3U);
			if (OverflowCheck::sum(tmp,currentOffset,chunkHeaderLen)>_packedSize)
				throw InvalidFormatError();
			currentOffset+=chunkHeaderLen+tmp;
		}
		readDualValue(4U,4U,packedSize);
		readDualValue(6U,8U,rawSize);
		
		ConstSubBuffer hdr{_packedData,currentOffset,chunkHeaderLen};
		ConstSubBuffer chunk{_packedData,currentOffset+chunkHeaderLen,packedSize};

		uint8_t type{_packedData.read8(currentOffset)};
		if (!func(hdr,chunk,rawSize,type)) return;

		if (type==15U) isLast=true;
	}
	if (!isLast)
		throw InvalidFormatError();
}

}
