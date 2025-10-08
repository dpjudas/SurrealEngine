/* Copyright (C) Teemu Suutari */

#include <algorithm>

#include "PPDecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

PPDecompressor::PPState::PPState(uint32_t mode) noexcept :
	_cachedMode{mode}
{
	// nothing needed
}

bool PPDecompressor::detectHeader(uint32_t hdr,uint32_t footer) noexcept
{
	return hdr==FourCC("PP11") || hdr==FourCC("PP20") || hdr==FourCC("PX20")
		|| hdr==FourCC("CHFC")		// Sky High Stuntman
		|| hdr==FourCC("DEN!")		// Jewels - Crossroads
		|| hdr==FourCC("DXS9")		// Hopp oder Top, Punkt Punkt Punkt
		|| hdr==FourCC("H.D.")		// F1 Challenge
		|| hdr==FourCC("RVV!");		// Hoi AGA Remix
}

bool PPDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("PWPK");
}

std::shared_ptr<Decompressor> PPDecompressor::create(const Buffer &packedData,bool exactSizeKnown,bool verify)
{
	return std::make_shared<PPDecompressor>(packedData,exactSizeKnown,verify);
}

std::shared_ptr<XPKDecompressor> PPDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<PPDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

PPDecompressor::PPDecompressor(const Buffer &packedData,bool exactSizeKnown,bool verify) :
	_packedData{packedData}
{
	if (!exactSizeKnown || packedData.size()<16U)
		throw InvalidFormatError();		// no scanning support
	_dataStart=_packedData.size()-4;

	uint32_t hdr{packedData.readBE32(0)};
	if (!detectHeader(hdr,0))
		throw InvalidFormatError(); 
	if (hdr==FourCC("PX20"))
	{
		if (packedData.size()<18U)
			throw InvalidFormatError();
		_isObsfuscated=true;
	}
	uint32_t mode{packedData.readBE32(_isObsfuscated?6U:4U)};
	if (mode!=0x9090909 && mode!=0x90a0a0a && mode!=0x90a0b0b && mode!=0x90a0c0c && mode!=0x90a0c0d)
		throw InvalidFormatError();
	for (uint32_t i=0;i<4;i++)
	{
		_modeTable[i]=mode>>24U;
		mode<<=8;
	}

	uint32_t tmp{packedData.readBE32(_dataStart)};

	_rawSize=tmp>>8U;
	_startShift=tmp&0xffU;
	if (!_rawSize || _startShift>=0x20U || _rawSize>getMaxRawSize())
		throw InvalidFormatError();
}

PPDecompressor::PPDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr) || packedData.size()<4U)
		throw InvalidFormatError(); 
	_dataStart=_packedData.size()-4;

	uint32_t mode;
	if (state.get())
	{
		mode=static_cast<PPState*>(state.get())->_cachedMode;
	} else {
		mode=packedData.readBE32(_dataStart);
		if (mode>4)
			throw InvalidFormatError();
		state.reset(new PPState(mode));
		if (_dataStart<4U)
			 throw InvalidFormatError();
		_dataStart-=4;
	}

	const std::array<uint32_t,5> modeMap{0x9090909,0x90a0a0a,0x90a0b0b,0x90a0c0c,0x90a0c0d};
	mode=modeMap[mode];
	for (uint32_t i=0;i<4;i++)
	{
		_modeTable[i]=mode>>24;
		mode<<=8;
	}

	uint32_t tmp{packedData.readBE32(_dataStart)};

	_rawSize=tmp>>8U;
	_startShift=tmp&0xffU;
	if (!_rawSize || _startShift>=0x20U || _rawSize>getMaxRawSize())
		throw InvalidFormatError();

	_isXPK=true;
}

const std::string &PPDecompressor::getName() const noexcept
{
	static std::string name{"PP: PowerPacker"};
	return name;
}

const std::string &PPDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-PWPK: PowerPacker"};
	return name;
}

size_t PPDecompressor::getPackedSize() const noexcept
{
	return 0;
}

size_t PPDecompressor::getRawSize() const noexcept
{
	return _rawSize;
}

/*
	Here comes the unobsfuscation (It would be stretch to call it as encryption cracking)

	The PowerPacker uses following method for obsfuscating files.

	First string of maximum of 16 characters is taken as a password (ISO8859-1 assumed,
	although anything with null termination goes. In fact the 16 characters limit is UI thing)

	Then both a 32-bit key and 16-bit checksum are calculated from the string.

	the algorithm for the key is as follows

	uint32_t ret=0;
	while (uint8_t ch=*(str++))
	{
		ret<<<1U;
		ret+=ch;
		swap(ret);
	}
	return ret;

	and the algorithm for the checksum (stored in header at offset 4) is as follows

	uint16_t ret=0;
	while (uint8_t ch=*(str++))
	{
		ret>>>=ch&0xfU;
		ret+=ch;
	}
	return ret;

	When the official decompressor tries to decrypt, it first checks the checksum of the key.
	(This is the only checksum PowerPacker has)

	After that the 32-bit is used to xor every long word read from the stream.


	Now, there are many ways you could skin this cat.

	One simple thing would be to attack the password. By limiting to printable characters
	and having dictionary of usual word would greatly help here. But this would add complexity to the
	code and by adding any dictionary would just be more ballast.

	Since the actual decompression is 32-bit xor, it would also be beneficial to do just a histogram
	with 4 buckets. However, the input files are pretty short and PP algorithm is very good at mangling
	bytes and bits so there is not neccessarily enough data to get a good idea of the key.

	Going through the 32-bit keyspace is not complicated in itself either. The only complication
	is that since there are not checksum for the output, the only way to validate the key is to check that
	input forms a correct bitstream. This is what ppcrack does. It takes minutes / hours or maybe even a day
	to go through a single file depending the size of the file, possible key and whether you are lucky.

	It obviously could be optimized by parallelization (or even dumping the whole set to GPU). Although
	this would bring results faster, it would make this library system dependent and also probably
	would raise a few eyebrows (am I accidentally running mining operation?!?) so the best option is actually
	to do it smarter. And smarter here is really a stretch but we try

	Due to the nature of the format, the fail paths happen early in the process, and if you know that some tree path
	does not yield results, it can be completely ignored. Thus making a tree of the possible choices taken
	wrt. to key bits is beneficial. In naive complexity calculation this would divide the effort into half. But
	in practice the algorithm fails quickly i.e. when some input does not fit it is usually the end of the possibilities..
	And this will bring the time into sub-second to go through the keyspace.

	This would in fact be enough, but we are not done yet. We can leverage the weaknesses in the implementation.
	First of all, first bit of the input needs to be zero (literal character). This reduces the keyspace to a
	maximum of 31-bits.

	But this is not all. There are filler bits at the end. Last long word of the input is something where
	possible incomplete word is stored (ranging from 1 to 31 bits of data). It would have been prudent
	to clear these bits after the xor-bits but they are not. Instead they are filled pre-xor with (depending on version)
	1. address of buffer when encoded + size. And since mostly Amiga allocates buffers
	   with 8 byte alignment this means 3 free bits of key
	2. stack pointer. Due to the memory allocation alignment and predictable path we can guess that last three bits are 110
	   (This is not applicable when PP library is used)
	If these approaches fail the "slower" path is chosen.

	This makes the algorithm fast in general, and even faster when we can leverage the filler bits.
	I believe you could run this on unaccelerated Amiga 500 and get the key in no time. :D

	For the actual user, the obsfuscation is now nothing but a minor speed bump.


	Little bit on the code style.

	I started this with co-routines in my mind. But I do not want to go with c++20. At least not yet.
	Also it would be questionable whether it would perform.

	The next thing that I did I removed the exception throwing on the failure case for finding a key.
	Unfortunately it doubles the running speed compared to the c-style flagged variable break.

	I think the code went from readable to whacky in this process. Sorry about that


	In any case this should serve as a warning to anyone trying to create their own crypto.
*/

void PPDecompressor::findKeyRound(BackwardInputStream &inputStream,LSBBitReader<BackwardInputStream> &bitReader,uint32_t keyBits,uint32_t keyMask,uint32_t outputPosition,uint32_t &iterCount)
{

	uint32_t inputOffset;
	uint32_t bufContent;
	uint8_t bufLength;
	uint32_t savedOutputPosition;

	bool failed{false};

	auto readBit=[&]()->uint32_t
	{
		if (failed) return 0;

		uint32_t bitPos{32U-bitReader.getBufLength()};
		if (bitPos==32U) bitPos=0;
		if (!bitPos && inputStream.getOffset()<=10U)
		{
			failed=true;
			return 0;
		}

		uint32_t bit{bitReader.readBitsBE32(1U)};

		if ((keyMask>>bitPos)&1U)
			return bit^((keyBits>>bitPos)&1U);

		// meh
		uint32_t tmpInputOffset{uint32_t(inputStream.getOffset())};
		uint32_t tmpBufContent{bitReader.getBufContent()};
		uint8_t tmpBufLength{bitReader.getBufLength()};

		// try 0
		inputStream.setOffset(inputOffset);
		bitReader.reset(bufContent,bufLength);
		findKeyRound(inputStream,bitReader,keyBits,keyMask|(1U<<bitPos),savedOutputPosition,iterCount);

		// try 1
		inputStream.setOffset(tmpInputOffset);
		bitReader.reset(tmpBufContent,tmpBufLength);
		keyBits|=(1U<<bitPos);
		keyMask|=(1U<<bitPos);
		return bit^1U;
	};

	auto readBits=[&](uint32_t count)->uint32_t
	{
		if (failed) return 0;

		uint32_t ret{0};
		while (count--)
		{
			ret=(ret<<1U)|readBit();
			if (failed) return 0;
		}
		return ret;
	};

	auto consumeBits=[&](uint32_t count)
	{
		if (failed) return;

		uint8_t bufLength{bitReader.getBufLength()};
		uint32_t bits{std::min(uint32_t(bufLength),count)};
		bitReader.readBitsBE32(bits);
		count-=bits;
		if (!count) return;
		uint32_t bytes{(count>>3U)&~3U};
		uint32_t offset{uint32_t(inputStream.getOffset())};
		if (offset<bytes+10U)
		{
			failed=true;
			return;
		}
		inputStream.setOffset(offset-bytes);
		count&=0x1fU;
		if (!count) return;
		if (inputStream.getOffset()<=10U)
		{
			failed=true;
			return;
		}
		bitReader.readBitsBE32(count);
	};

	// TODO: Random constant. For decompression/keyfinding bombs
	while (iterCount<1048576)
	{
		// this is the checkpoint. Hardly ideal, but best we can do without co-routines
		inputOffset=uint32_t(inputStream.getOffset());
		bufContent=bitReader.getBufContent();
		bufLength=bitReader.getBufLength();
		savedOutputPosition=outputPosition;

		if (!readBit())
		{
			if (failed) break;
			uint32_t count{1};
			for (;;)
			{
				uint32_t tmp{readBits(2)};
				if (failed) break;
				count+=tmp;
				if (tmp<3) break;
			}
			if (count>outputPosition)
				failed=true;
			if (failed) break;
			consumeBits(count*8U);
			outputPosition-=count;
		}
		if (!outputPosition || failed)
			 break;
		uint32_t modeIndex{readBits(2)};
		if (failed) break;
		uint32_t count,distance;
		if (modeIndex==3)
		{
			distance=readBits(readBit()?_modeTable[modeIndex]:7)+1;
			if (failed) break;
			count=5;
			for (;;)
			{
				uint32_t tmp{readBits(3)};
				if (failed) break;
				count+=tmp;
				if (tmp<7) break;
			}
		} else {
			count=modeIndex+2;
			distance=readBits(_modeTable[modeIndex])+1;
		}
		if (outputPosition+distance>_rawSize || count>outputPosition)
			failed=true;
		if (failed) break;
		outputPosition-=count;

		iterCount++;
	}
	if (failed) return;
	// If not all bits are resolved, that is bad
	if (keyMask==0xffff'ffffU)
		throw DoneException(keyBits);
}

void PPDecompressor::findKey(uint32_t keyBits,uint32_t keyMask)
{
	BackwardInputStream inputStream{_packedData,10,_dataStart};
	LSBBitReader<BackwardInputStream> bitReader{inputStream};

	bitReader.readBitsBE32(_startShift);

	uint32_t iterCount=0;
	findKeyRound(inputStream,bitReader,keyBits,keyMask,uint32_t(_rawSize),iterCount);
}

void PPDecompressor::decompressImpl(Buffer &rawData,bool verify)
{
	if (rawData.size()<_rawSize)
		throw DecompressionError();

	uint32_t key=0;
	if (_isObsfuscated)
	{
		uint32_t fillerData{_packedData.readBE32(_dataStart-4U)};

		// although this not help too much since we always start from zero, it will
		// make the stack frame shorter
		uint32_t keyBits{fillerData};
		uint32_t keyMask{1U<<_startShift};
		keyBits&=keyMask;

		// now the fuzzy
		try {
			if (_startShift)
			{
				// SP
				uint32_t bitCount{std::min(uint32_t(_startShift),3U)};
				uint32_t bitMask{((1U<<bitCount)-1U)<<(_startShift-bitCount)};
				findKey(keyBits|((fillerData^(3U<<(_startShift-bitCount)))&bitMask),keyMask|bitMask);

				// Size
				findKey(keyBits|((fillerData^(rotateBits(_rawSize&7U,3U)<<(_startShift-bitCount)))&bitMask),keyMask|bitMask);
			}
			findKey(keyBits,keyMask);
			throw DecompressionError();
		} catch (const DoneException &e) {
			key=e.getKey();
		}
	}

	BackwardInputStream inputStream{_packedData,_isXPK?0:(_isObsfuscated?10U:8U),_dataStart};
	LSBBitReader<BackwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return rotateBits(bitReader.readBitsGeneric(count,[&](){
			return std::make_pair(inputStream.readBE32()^key,32U);
		}),count);
	};
	auto readBit=[&]()->uint32_t
	{
		return readBits(1);
	};

	readBits(_startShift);
	BackwardOutputStream outputStream{rawData,0,_rawSize};

	for (;;)
	{
		if (!readBit())
		{
			uint32_t count{1};
			// This does not make much sense I know. But it is what it is...
			for (;;)
			{
				uint32_t tmp{readBits(2)};
				count+=tmp;
				if (tmp<3) break;
			}
			for (uint32_t i=0;i<count;i++) outputStream.writeByte(readBits(8));
		}
		if (outputStream.eof()) break;
		uint32_t modeIndex{readBits(2)};
		uint32_t count,distance;
		if (modeIndex==3)
		{
			distance=readBits(readBit()?_modeTable[modeIndex]:7)+1;
			// ditto
			count=5;
			for (;;)
			{
				uint32_t tmp{readBits(3)};
				count+=tmp;
				if (tmp<7) break;
			}
		} else {
			count=modeIndex+2;
			distance=readBits(_modeTable[modeIndex])+1;
		}
		outputStream.copy(distance,count);
	}
}

void PPDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	if (_rawSize!=rawData.size())
		throw DecompressionError();
	decompressImpl(rawData,verify);
}

}
