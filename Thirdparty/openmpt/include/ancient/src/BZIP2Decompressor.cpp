/* Copyright (C) Teemu Suutari */

#include <cstdint>
#include <cstring>

#include "BZIP2Decompressor.hpp"
#include "HuffmanDecoder.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/MemoryBuffer.hpp"
#include "common/CRC32.hpp"
#include "common/Common.hpp"

#include <array>

namespace ancient::internal
{

bool BZIP2Decompressor::detectHeader(uint32_t hdr,uint32_t footer) noexcept
{
	return ((hdr&0xffff'ff00U)==FourCC("BZh\0") && (hdr&0xffU)>='1' && (hdr&0xffU)<='9');
}

bool BZIP2Decompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return (hdr==FourCC("BZP2"));
}

std::shared_ptr<Decompressor> BZIP2Decompressor::create(const Buffer &packedData,bool exactSizeKnown,bool verify)
{
	return std::make_shared<BZIP2Decompressor>(packedData,exactSizeKnown,verify);
}

std::shared_ptr<XPKDecompressor> BZIP2Decompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<BZIP2Decompressor>(hdr,recursionLevel,packedData,state,verify);
}

BZIP2Decompressor::BZIP2Decompressor(const Buffer &packedData,bool exactSizeKnown,bool verify) :
	_packedData{packedData},
	_packedSize{0}
{
	uint32_t hdr=packedData.readBE32(0);
	if (!detectHeader(hdr,0))
		throw Decompressor::InvalidFormatError();;
	_blockSize=((hdr&0xffU)-'0')*100'000;
}

BZIP2Decompressor::BZIP2Decompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData},
	_packedSize{_packedData.size()}
{
	uint32_t blockHdr=packedData.readBE32(0);
	if (!detectHeader(blockHdr,0))
		throw Decompressor::InvalidFormatError();;
	_blockSize=((blockHdr&0xffU)-'0')*100'000;
}

const std::string &BZIP2Decompressor::getName() const noexcept
{
	static std::string name{"bz2: bzip2"};
	return name;
}

const std::string &BZIP2Decompressor::getSubName() const noexcept
{
	static std::string name{"XPK-BZP2: bzip2"};
	return name;
}

size_t BZIP2Decompressor::getPackedSize() const noexcept
{
	// no way to know before decompressing
	return _packedSize;
}


size_t BZIP2Decompressor::getRawSize() const noexcept
{
	// same thing, decompression needed first
	return _rawSize;
}

void BZIP2Decompressor::decompressImpl(Buffer &rawData,bool verify)
{
	size_t packedSize=_packedSize?_packedSize:_packedData.size();

	ForwardInputStream inputStream{_packedData,4,packedSize};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBits8(count);
	};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBits8(1);
	};

	AutoExpandingForwardOutputStream outputStream{rawData};

	// stream verification
	//
	// there is so much wrong in bzip2 CRC-calculation :(
	// 1. The bit ordering is opposite what everyone else does with CRC32
	// 2. The block CRCs are calculated separately, no way of calculating a complete
	//    CRC without knowing the block layout
	// 3. The CRC is the end of the stream and the stream is bit aligned. You
	//    can't read CRC without decompressing the stream.
	uint32_t crc=0;
	auto calculateBlockCRC=[&](size_t blockPos,size_t blockSize)
	{
		crc=(crc<<1)|(crc>>31);
		crc^=CRC32Rev(rawData,blockPos,blockSize,0);
	};

	HuffmanDecoder<uint8_t> selectorDecoder
	{
		// incomplete Huffman table. errors possible
		HuffmanCode{1,0b000000,uint8_t{0}},
		HuffmanCode{2,0b000010,uint8_t{1}},
		HuffmanCode{3,0b000110,uint8_t{2}},
		HuffmanCode{4,0b001110,uint8_t{3}},
		HuffmanCode{5,0b011110,uint8_t{4}},
		HuffmanCode{6,0b111110,uint8_t{5}}
	};

	HuffmanDecoder<int32_t> deltaDecoder
	{
		HuffmanCode{1,0b00,0},
		HuffmanCode{2,0b10,1},
		HuffmanCode{2,0b11,-1}
	};

	MemoryBuffer tmpBuffer{_blockSize};

	// This is the dark, ancient secret of bzip2.
	// versions before 0.9.5 had a data randomization for "too regular"
	// data problematic for the bwt-implementation at that time.
	// although it is never utilized anymore, the support is still there
	// And this is exactly the kind of ancient stuff we want to support :)
	//
	// On this specific part (since it is a table of magic numbers)
	// we have no way other than copying it from the original reference

// Table has a separate copyright, lets have it as a separate file as well
#include "BZIP2Table.hpp"

	for (;;)
	{
		uint32_t blockHdrHigh{readBits(32)};
		uint32_t blockHdrLow{readBits(16)};
		if (blockHdrHigh==0x31415926U && blockHdrLow==0x5359U)
		{
			// a block

			// this is rather spaghetti...
			readBits(32);	// block crc, not interested
			bool randomized{!!readBit()};

			// basically the random inserted is one LSB after n-th bytes
			// per defined in the table.
			uint32_t randomPos{1};
			uint32_t randomCounter{randomTable[0]-1U};
			auto randomBit=[&]()->bool
			{
				// Beauty is in the eye of the beholder: this is smallest form to hide the ugliness
				return (!randomCounter--)?randomCounter=randomTable[randomPos++&511]:false;
			};

			uint32_t currentPtr{readBits(24)};

			uint32_t currentBlockSize{0};
			{
				uint32_t numHuffmanItems{2};
				std::array<uint32_t,256> huffmanValues;

				{
					// this is just a little bit inefficient but still we reading bit by bit since
					// reference does it. (bitsream format details do not spill over)
					std::array<bool,16> usedMap;
					for (uint32_t i=0;i<16;i++) usedMap[i]=readBit();

					std::array<bool,256> huffmanMap;
					for (uint32_t i=0;i<16;i++)
					{
						for (uint32_t j=0;j<16;j++)
							huffmanMap[i*16+j]=(usedMap[i])?readBit():false;
					}

					for (uint32_t i=0;i<256;i++) if (huffmanMap[i]) numHuffmanItems++;
					if (numHuffmanItems==2)
						throw DecompressionError();

					for (uint32_t currentValue=0,i=0;i<256;i++)
						if (huffmanMap[i]) huffmanValues[currentValue++]=i;
				}

				uint32_t huffmanGroups{readBits(3)};
				if (huffmanGroups<2 || huffmanGroups>6)
					throw DecompressionError();

				uint32_t selectorsUsed{readBits(15)};
				if (!selectorsUsed)
					throw DecompressionError();

				std::vector<uint8_t> huffmanSelectorList(selectorsUsed);

				auto unMTF=[](uint8_t value,auto &map)->uint8_t
				{
					uint8_t ret{map[value]};
					if (value)
					{
						uint8_t tmp=map[value];
						for (uint32_t i=value;i;i--)
							map[i]=map[i-1];
						map[0]=tmp;
					}
					return ret;
				};

				// create Huffman selectors
				std::array<uint8_t,6> selectorMTFMap{0,1,2,3,4,5};

				for (uint32_t i=0;i<selectorsUsed;i++)
				{
					uint8_t item{unMTF(selectorDecoder.decode(readBit),selectorMTFMap)};
					if (item>=huffmanGroups)
						throw DecompressionError();
					huffmanSelectorList[i]=item;
				}

				typedef HuffmanDecoder<uint32_t> BZIP2Decoder;
				std::vector<BZIP2Decoder> dataDecoders{huffmanGroups};

				// Create all tables
				for (uint32_t i=0;i<huffmanGroups;i++)
				{
					std::array<uint8_t,258> bitLengths;

					uint32_t currentBits{readBits(5)};
					for (uint32_t j=0;j<numHuffmanItems;j++)
					{
						int32_t delta;
						do
						{
							delta=deltaDecoder.decode(readBit);
							currentBits+=delta;
						} while (delta);
						if (currentBits<1 || currentBits>20)
							throw DecompressionError();
						bitLengths[j]=currentBits;
					}

					dataDecoders[i].createOrderlyHuffmanTable(bitLengths,numHuffmanItems);
				}

				// Huffman decode + unRLE + unMTF
				BZIP2Decoder *currentHuffmanDecoder{nullptr};
				uint32_t currentHuffmanIndex{0};
				std::array<uint8_t,256> dataMTFMap;
				for (uint32_t i=0;i<numHuffmanItems-2;i++) dataMTFMap[i]=i;

				uint32_t currentRunLength{0};
				uint32_t currentRLEWeight{1};

				auto decodeRLE=[&]()
				{
					if (currentRunLength)
					{
						if (currentBlockSize+currentRunLength>_blockSize)
							throw DecompressionError();
						for (uint32_t i=0;i<currentRunLength;i++) tmpBuffer[currentBlockSize++]=huffmanValues[dataMTFMap[0]];
					}
					currentRunLength=0;
					currentRLEWeight=1;
				};

				for (uint32_t streamIndex=0;;streamIndex++)
				{
					if (!(streamIndex%50))
					{
						if (currentHuffmanIndex>=selectorsUsed)
							throw DecompressionError();
						currentHuffmanDecoder=&dataDecoders[huffmanSelectorList[currentHuffmanIndex++]];
					}
					uint32_t symbolMTF{currentHuffmanDecoder->decode(readBit)};
					// stop marker is referenced only once, and it is the last one
					// This means we do no have to un-MTF it for detection
					if (symbolMTF==numHuffmanItems-1) break;
					if (currentBlockSize>=_blockSize)
						throw DecompressionError();
					if (symbolMTF<2)
					{
						currentRunLength+=currentRLEWeight<<symbolMTF;
						currentRLEWeight<<=1;
					} else {
						decodeRLE();
						uint8_t symbol{unMTF(symbolMTF-1,dataMTFMap)};
						if (currentBlockSize>=_blockSize)
							throw DecompressionError();
						tmpBuffer[currentBlockSize++]=huffmanValues[symbol];
					}
				}
				decodeRLE();
				if (currentPtr>=currentBlockSize)
					throw DecompressionError();
			}

			// inverse BWT + final RLE decoding.
			// there are a few dark corners here as well
			// 1. Can the stream end at 4 literals without count? I assume it is a valid optimization (and that this does not spillover to next block)
			// 2. Can the RLE-step include counts 252 to 255 even if reference does not do them? I assume yes here as here as well
			// 3. Can the stream be empty? We do not take issue here about that (that should be culled out earlier already)
			std::array<uint32_t,256> sums;
			for (uint32_t i=0;i<256;i++) sums[i]=0;

			for (uint32_t i=0;i<currentBlockSize;i++)
				sums[tmpBuffer[i]]++;

			std::array<uint32_t,256> rank;
			for (uint32_t tot=0,i=0;i<256;i++)
			{
				rank[i]=tot;
				tot+=sums[i];
			}

			// not at all happy about the memory consumption, but it simplifies the implementation a lot
			// and by sacrificing 4*size (size as in actual block size) we do not have to have slow search nor another temporary buffer
			// since by calculating forward table we can do forward decoding of the data on the same pass as iBWT
			//
			// also, because I'm lazy
			std::vector<uint32_t> forwardIndex(currentBlockSize);
			for (uint32_t i=0;i<currentBlockSize;i++)
				forwardIndex[rank[tmpBuffer[i]]++]=i;

			// output + final RLE decoding
			uint8_t currentCh{0};
			uint32_t currentChCount{0};
			auto outputByte=[&](uint8_t ch)
			{
				if (randomized && randomBit()) ch^=1;
				if (!currentChCount)
				{
					currentCh=ch;
					currentChCount=1;
				} else {
					if (ch==currentCh && currentChCount!=4)
					{
						currentChCount++;
					} else {
						auto outputBlock=[&](uint32_t count)
						{
							for (uint32_t i=0;i<count;i++) outputStream.writeByte(currentCh);
						};

						if (currentChCount==4)
						{
							outputBlock(uint32_t(ch)+4);
							currentChCount=0;
						} else {
							outputBlock(currentChCount);
							currentCh=ch;
							currentChCount=1;
						}
					}
				}
			};

			size_t destOffsetStart{outputStream.getOffset()};

			// and now the final iBWT + unRLE is easy...
			for (uint32_t i=0;i<currentBlockSize;i++)
			{
				currentPtr=forwardIndex[currentPtr];
				outputByte(tmpBuffer[currentPtr]);
			}
			// cleanup the state, a bit hackish way to do it
			if (currentChCount) outputByte(currentChCount==4?0:~currentCh);

			if (verify)
				calculateBlockCRC(destOffsetStart,outputStream.getOffset()-destOffsetStart);

		} else if (blockHdrHigh==0x17724538U && blockHdrLow==0x5090U) {
			// end of blocks
			uint32_t rawCRC{readBits(32)};
			if (verify && crc!=rawCRC)
				throw VerificationError();
			break;
		} else throw DecompressionError();
	}

	_rawSize=outputStream.getOffset();
	_packedSize=inputStream.getOffset();
}

void BZIP2Decompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	return decompressImpl(rawData,verify);
}

}
