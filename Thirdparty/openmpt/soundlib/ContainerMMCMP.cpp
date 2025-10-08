/*
 * ContainerMMCMP.cpp
 * ------------------
 * Purpose: Handling of MMCMP compressed modules
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"

#include "../common/FileReader.h"
#include "Container.h"
#include "Sndfile.h"
#include "BitReader.h"


OPENMPT_NAMESPACE_BEGIN


#if !defined(MPT_WITH_ANCIENT)


#ifdef MPT_ALL_LOGGING
#define MMCMP_LOG
#endif


struct MMCMPFileHeader
{
	char     id[8];    // "ziRCONia"
	uint16le hdrsize;  // size of all the remaining header data
	uint16le version;
	uint16le nblocks;
	uint32le filesize;
	uint32le blktable;
	uint8le  glb_comp;
	uint8le  fmt_comp;

	bool Validate() const
	{
		if(std::memcmp(id, "ziRCONia", 8) != 0)
			return false;
		if(hdrsize != 14)
			return false;
		if(nblocks == 0)
			return false;
		if(filesize == 0)
			return false;
		if(filesize >= 0x80000000)
			return false;
		if(blktable < sizeof(MMCMPFileHeader))
			return false;
		return true;
	}
};

MPT_BINARY_STRUCT(MMCMPFileHeader, 24)

struct MMCMPBlock
{
	uint32le unpk_size;
	uint32le pk_size;
	uint32le xor_chk;
	uint16le sub_blk;
	uint16le flags;
	uint16le tt_entries;
	uint16le num_bits;
};

MPT_BINARY_STRUCT(MMCMPBlock, 20)

struct MMCMPSubBlock
{
	uint32le position;
	uint32le size;

	bool Validate(std::vector<char> &unpackedData, const uint32 unpackedSize) const
	{
		if(position >= unpackedSize)
			return false;
		if(size > unpackedSize)
			return false;
		if(size > unpackedSize - position)
			return false;
		if(size == 0)
			return false;
		if(unpackedData.size() < position + size)
			unpackedData.resize(position + size);
		return true;
	}
};

MPT_BINARY_STRUCT(MMCMPSubBlock, 8)

enum MMCMPFlags : uint16
{
	MMCMP_COMP   = 0x0001,
	MMCMP_DELTA  = 0x0002,
	MMCMP_16BIT  = 0x0004,
	MMCMP_STEREO = 0x0100,
	MMCMP_ABS16  = 0x0200,
	MMCMP_ENDIAN = 0x0400,
};

static constexpr uint8 MMCMP8BitCommands[8] =
{
	0x01, 0x03, 0x07, 0x0F, 0x1E, 0x3C, 0x78, 0xF8
};

static constexpr uint8 MMCMP8BitFetch[8] =
{
	3, 3, 3, 3, 2, 1, 0, 0
};

static constexpr uint16 MMCMP16BitCommands[16] =
{
	0x01,  0x03,  0x07,  0x0F,  0x1E,   0x3C,   0x78,   0xF0,
	0x1F0, 0x3F0, 0x7F0, 0xFF0, 0x1FF0, 0x3FF0, 0x7FF0, 0xFFF0
};

static constexpr uint8 MMCMP16BitFetch[16] =
{
	4, 4, 4, 4, 3, 2, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderMMCMP(MemoryFileReader file, const uint64 *pfilesize)
{
	MMCMPFileHeader mfh;
	if(!file.ReadStruct(mfh))
		return ProbeWantMoreData;
	if(!mfh.Validate())
		return ProbeFailure;
	MPT_UNREFERENCED_PARAMETER(pfilesize);
	return ProbeSuccess;
}


bool UnpackMMCMP(std::vector<ContainerItem> &containerItems, FileReader &file, ContainerLoadingFlags loadFlags)
{
	file.Rewind();
	containerItems.clear();

	MMCMPFileHeader mfh;
	if(!file.ReadStruct(mfh))
		return false;
	if(!mfh.Validate())
		return false;
	if(loadFlags == ContainerOnlyVerifyHeader)
		return true;
	if(!file.LengthIsAtLeast(mfh.blktable))
		return false;
	if(!file.LengthIsAtLeast(mfh.blktable + 4 * mfh.nblocks))
		return false;

	containerItems.emplace_back();
	containerItems.back().data_cache = std::make_unique<std::vector<char> >();
	auto &unpackedData = *(containerItems.back().data_cache);

	// Generally it's not so simple to establish an upper limit for the uncompressed data size (blocks can be reused, etc.),
	// so we just reserve a realistic amount of memory.
	const uint32 unpackedSize = mfh.filesize;
	unpackedData.reserve(std::min(unpackedSize, std::min(mpt::saturate_cast<uint32>(file.GetLength()), uint32_max / 20u) * 20u));
	// 8-bit deltas
	uint8 ptable[256] = { 0 };

	std::vector<MMCMPSubBlock> subblks;
	for(uint32 nBlock = 0; nBlock < mfh.nblocks; nBlock++)
	{
		if(!file.Seek(mfh.blktable + 4 * nBlock))
			return false;
		if(!file.CanRead(4))
			return false;
		uint32 blkPos = file.ReadUint32LE();
		if(!file.Seek(blkPos))
			return false;
		MMCMPBlock blk;
		if(!file.ReadStruct(blk))
			return false;
		if(!file.ReadVector(subblks, blk.sub_blk))
			return false;
		const MMCMPSubBlock *psubblk = blk.sub_blk > 0 ? subblks.data() : nullptr;

		if(blkPos + sizeof(MMCMPBlock) + blk.sub_blk * sizeof(MMCMPSubBlock) >= file.GetLength())
			return false;
		uint32 memPos = blkPos + static_cast<uint32>(sizeof(MMCMPBlock)) + blk.sub_blk * static_cast<uint32>(sizeof(MMCMPSubBlock));

#ifdef MMCMP_LOG
		MPT_LOG_GLOBAL(LogDebug, "MMCMP", MPT_UFORMAT("block {}: flags={} sub_blocks={}")(nBlock, mpt::ufmt::HEX0<4>(static_cast<uint16>(blk.flags)), static_cast<uint16>(blk.sub_blk)));
		MPT_LOG_GLOBAL(LogDebug, "MMCMP", MPT_UFORMAT(" pksize={} unpksize={}")(static_cast<uint32>(blk.pk_size), static_cast<uint32>(blk.unpk_size)));
		MPT_LOG_GLOBAL(LogDebug, "MMCMP", MPT_UFORMAT(" tt_entries={} num_bits={}")(static_cast<uint16>(blk.tt_entries), static_cast<uint16>(blk.num_bits)));
#endif
		if(!(blk.flags & MMCMP_COMP))
		{
			// Data is not packed
			for(uint32 i = 0; i < blk.sub_blk; i++)
			{
				if(!psubblk)
					return false;
				if(!psubblk->Validate(unpackedData, unpackedSize))
					return false;
#ifdef MMCMP_LOG
				MPT_LOG_GLOBAL(LogDebug, "MMCMP", MPT_UFORMAT("  Unpacked sub-block {}: offset {}, size={}")(i, static_cast<uint32>(psubblk->position), static_cast<uint32>(psubblk->size)));
#endif
				if(!file.Seek(memPos))
					return false;
				if(file.ReadRaw(mpt::span(&(unpackedData[psubblk->position]), psubblk->size)).size() != psubblk->size)
					return false;
				psubblk++;
			}
		} else if(blk.flags & MMCMP_16BIT)
		{
			// Data is 16-bit packed
			uint32 subblk = 0;
			if(!psubblk)
				return false;
			if(!psubblk[subblk].Validate(unpackedData, unpackedSize))
				return false;
			char *pDest = &(unpackedData[psubblk[subblk].position]);
			uint32 dwSize = psubblk[subblk].size & ~1u;
			if(!dwSize)
				return false;
			uint32 dwPos = 0;
			uint32 numbits = blk.num_bits;
			uint32 oldval = 0;

#ifdef MMCMP_LOG
			MPT_LOG_GLOBAL(LogDebug, "MMCMP", MPT_UFORMAT("  16-bit block: pos={} size={} {} {}")(psubblk->position, psubblk->size, (blk.flags & MMCMP_DELTA) ? U_("DELTA ") : U_(""), (blk.flags & MMCMP_ABS16) ? U_("ABS16 ") : U_("")));
#endif
			if(numbits > 15) return false;
			if(!file.Seek(memPos + blk.tt_entries)) return false;
			if(!file.CanRead(blk.pk_size - blk.tt_entries)) return false;
			BitReader bitFile{ file.GetChunk(blk.pk_size - blk.tt_entries) };

			try
			{
				while (subblk < blk.sub_blk)
				{
					uint32 newval = 0x10000;
					uint32 d = bitFile.ReadBits(numbits + 1);

					uint32 command = MMCMP16BitCommands[numbits & 0x0F];
					if(d >= command)
					{
						uint32 nFetch = MMCMP16BitFetch[numbits & 0x0F];
						uint32 newbits = bitFile.ReadBits(nFetch) + ((d - command) << nFetch);
						if(newbits != numbits)
						{
							numbits = newbits & 0x0F;
						} else if((d = bitFile.ReadBits(4)) == 0x0F)
						{
							if(bitFile.ReadBits(1))
								break;
							newval = 0xFFFF;
						} else
						{
							newval = 0xFFF0 + d;
						}
					} else
					{
						newval = d;
					}
					if(newval < 0x10000)
					{
						newval = (newval & 1) ? (uint32)(-(int32)((newval + 1) >> 1)) : (uint32)(newval >> 1);
						if(blk.flags & MMCMP_DELTA)
						{
							newval += oldval;
							oldval = newval;
						} else if(!(blk.flags & MMCMP_ABS16))
						{
							newval ^= 0x8000;
						}
						if(blk.flags & MMCMP_ENDIAN)
						{
								pDest[dwPos + 0] = static_cast<uint8>(newval >> 8);
								pDest[dwPos + 1] = static_cast<uint8>(newval & 0xFF);
						} else
						{
							pDest[dwPos + 0] = static_cast<uint8>(newval & 0xFF);
							pDest[dwPos + 1] = static_cast<uint8>(newval >> 8);
						}
						dwPos += 2;
					}
					if(dwPos >= dwSize)
					{
						subblk++;
						dwPos = 0;
						if(!(subblk < blk.sub_blk))
							break;
						if(!psubblk[subblk].Validate(unpackedData, unpackedSize))
							return false;
						dwSize = psubblk[subblk].size & ~1u;
						if(!dwSize)
							return false;
						pDest = &(unpackedData[psubblk[subblk].position]);
					}
				}
			} catch(const BitReader::eof &)
			{
			}
		} else
		{
			// Data is 8-bit packed
			uint32 subblk = 0;
			if(!psubblk)
				return false;
			if(!psubblk[subblk].Validate(unpackedData, unpackedSize))
				return false;
			char *pDest = &(unpackedData[psubblk[subblk].position]);
			uint32 dwSize = psubblk[subblk].size;
			uint32 dwPos = 0;
			uint32 numbits = blk.num_bits;
			uint32 oldval = 0;
			if(blk.tt_entries > sizeof(ptable)
				|| numbits > 7
				|| !file.Seek(memPos)
				|| file.ReadRaw(mpt::span(ptable, blk.tt_entries)).size() < blk.tt_entries)
				return false;

			if(!file.CanRead(blk.pk_size - blk.tt_entries)) return false;
			BitReader bitFile{ file.GetChunk(blk.pk_size - blk.tt_entries) };

			try
			{
				while (subblk < blk.sub_blk)
				{
					uint32 newval = 0x100;
					uint32 d = bitFile.ReadBits(numbits + 1);

					uint32 command = MMCMP8BitCommands[numbits & 0x07];
					if(d >= command)
					{
						uint32 nFetch = MMCMP8BitFetch[numbits & 0x07];
						uint32 newbits = bitFile.ReadBits(nFetch) + ((d - command) << nFetch);
						if(newbits != numbits)
						{
							numbits = newbits & 0x07;
						} else if((d = bitFile.ReadBits(3)) == 7)
						{
							if(bitFile.ReadBits(1))
								break;
							newval = 0xFF;
						} else
						{
							newval = 0xF8 + d;
						}
					} else
					{
						newval = d;
					}
					if(newval < sizeof(ptable))
					{
						int n = ptable[newval];
						if(blk.flags & MMCMP_DELTA)
						{
							n += oldval;
							oldval = n;
						}
						pDest[dwPos++] = static_cast<uint8>(n);
					}
					if(dwPos >= dwSize)
					{
						subblk++;
						dwPos = 0;
						if(!(subblk < blk.sub_blk))
							break;
						if(!psubblk[subblk].Validate(unpackedData, unpackedSize))
							return false;
						dwSize = psubblk[subblk].size;
						pDest = &(unpackedData[psubblk[subblk].position]);
					}
				}
			} catch(const BitReader::eof &)
			{
			}
		}
	}

	containerItems.back().file = FileReader(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(unpackedData)));

	return true;
}


#endif // !MPT_WITH_ANCIENT


OPENMPT_NAMESPACE_END
