/*
 * ContainerXPK.cpp
 * ----------------
 * Purpose: Handling of XPK compressed modules
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"

#include "../common/FileReader.h"
#include "Container.h"
#include "Sndfile.h"

#include <stdexcept>


OPENMPT_NAMESPACE_BEGIN


#if !defined(MPT_WITH_ANCIENT)


#ifdef MPT_ALL_LOGGING
#define MMCMP_LOG
#endif


struct XPKFILEHEADER
{
	char     XPKF[4];
	uint32be SrcLen;
	char     SQSH[4];
	uint32be DstLen;
	char     Name[16];
	uint32be Reserved;
};

MPT_BINARY_STRUCT(XPKFILEHEADER, 36)


struct XPK_error : public std::range_error
{
	XPK_error() : std::range_error("invalid XPK data") { }
};

struct XPK_BufferBounds
{
	const uint8 *pSrcBeg;
	std::size_t SrcSize;

	inline uint8 SrcRead(std::size_t index)
	{
		if(index >= SrcSize) throw XPK_error();
		return pSrcBeg[index];
	}
};

static int32 bfextu(std::size_t p, int32 bo, int32 bc, XPK_BufferBounds &bufs)
{
	uint32 r;

	p += bo / 8;
	r = bufs.SrcRead(p); p++;
	r <<= 8;
	r |= bufs.SrcRead(p); p++;
	r <<= 8;
	r |= bufs.SrcRead(p);
	r <<= bo % 8;
	r &= 0xffffff;
	r >>= 24 - bc;

	return r;
}

static int32 bfexts(std::size_t p, int32 bo, int32 bc, XPK_BufferBounds &bufs)
{
	uint32 r;

	p += bo / 8;
	r = bufs.SrcRead(p); p++;
	r <<= 8;
	r |= bufs.SrcRead(p); p++;
	r <<= 8;
	r |= bufs.SrcRead(p);
	r <<= (bo % 8) + 8;
	return mpt::rshift_signed(static_cast<int32>(r), 32 - bc);
}


static uint8 XPK_ReadTable(int32 index)
{
	static constexpr uint8 xpk_table[] = {
		2,3,4,5,6,7,8,0,3,2,4,5,6,7,8,0,4,3,5,2,6,7,8,0,5,4,6,2,3,7,8,0,6,5,7,2,3,4,8,0,7,6,8,2,3,4,5,0,8,7,6,2,3,4,5,0
	};
	if(index < 0) throw XPK_error();
	if(static_cast<std::size_t>(index) >= std::size(xpk_table)) throw XPK_error();
	// cppcheck false-positive
	// cppcheck-suppress arrayIndexOutOfBoundsCond
	return xpk_table[index];
}

static bool XPK_DoUnpack(const mpt::const_byte_span src_, std::vector<char> &unpackedData, int32 len)
{
	if(len <= 0) return false;
	int32 d0,d1,d2,d3,d4,d5,d6,a2,a5;
	int32 cp, cup1, type;
	std::size_t c;
	std::size_t src;
	std::size_t phist = 0;

	unpackedData.reserve(std::min(static_cast<uint32>(len), std::min(mpt::saturate_cast<uint32>(src_.size()), uint32_max / 20u) * 20u));

	XPK_BufferBounds bufs;
	bufs.pSrcBeg = mpt::byte_cast<const uint8*>(src_.data());
	bufs.SrcSize = src_.size();

	src = 0;
	c = src;
	while(len > 0)
	{
		type = bufs.SrcRead(c+0);
		cp = (bufs.SrcRead(c+4)<<8) | (bufs.SrcRead(c+5)); // packed
		cup1 = (bufs.SrcRead(c+6)<<8) | (bufs.SrcRead(c+7)); // unpacked
		//Log("  packed=%6d unpacked=%6d bytes left=%d dst=%08X(%d)\n", cp, cup1, len, dst, dst);
		c += 8;
		src = c+2;
		if (type == 0)
		{
			// RAW chunk
			if(cp < 0 || cp > len) throw XPK_error();
			for(int32 i = 0; i < cp; ++i)
			{
				unpackedData.push_back(bufs.SrcRead(c + i));
			}
			c+=cp;
			len -= cp;
			continue;
		}

		if (type != 1)
		{
			#ifdef MMCMP_LOG
				MPT_LOG_GLOBAL(LogDebug, "XPK", MPT_UFORMAT("Invalid XPK type! ({} bytes left)")(len));
			#endif
			break;
		}
		LimitMax(cup1, len);
		len -= cup1;
		cp = (cp + 3) & 0xfffc;
		c += cp;

		d0 = d1 = d2 = a2 = 0;
		d3 = bufs.SrcRead(src); src++;
		unpackedData.push_back(static_cast<char>(d3));
		cup1--;

		while (cup1 > 0)
		{
			if (d1 >= 8) goto l6dc;
			if (bfextu(src,d0,1,bufs)) goto l75a;
			d0 += 1;
			d5 = 0;
			d6 = 8;
			goto l734;

		l6dc:
			if (bfextu(src,d0,1,bufs)) goto l726;
			d0 += 1;
			if (! bfextu(src,d0,1,bufs)) goto l75a;
			d0 += 1;
			if (bfextu(src,d0,1,bufs)) goto l6f6;
			d6 = 2;
			goto l708;

		l6f6:
			d0 += 1;
			if (!bfextu(src,d0,1,bufs)) goto l706;
			d6 = bfextu(src,d0,3,bufs);
			d0 += 3;
			goto l70a;

		l706:
			d6 = 3;
		l708:
			d0 += 1;
		l70a:
			d6 = XPK_ReadTable((8*a2) + d6 -17);
			if (d6 != 8) goto l730;
		l718:
			if (d2 >= 20)
			{
				d5 = 1;
				goto l732;
			}
			d5 = 0;
			goto l734;

		l726:
			d0 += 1;
			d6 = 8;
			if (d6 == a2) goto l718;
			d6 = a2;
		l730:
			d5 = 4;
		l732:
			d2 += 8;
		l734:
			while ((d5 >= 0) && (cup1 > 0))
			{
				d4 = bfexts(src,d0,d6,bufs);
				d0 += d6;
				d3 -= d4;
				unpackedData.push_back(static_cast<char>(d3));
				cup1--;
				d5--;
			}
			if (d1 != 31) d1++;
			a2 = d6;
		l74c:
			d6 = d2;
			d6 >>= 3;
			d2 -= d6;
		}
	}
	return !unpackedData.empty();

l75a:
	d0 += 1;
	if (bfextu(src,d0,1,bufs)) goto l766;
	d4 = 2;
	goto l79e;

l766:
	d0 += 1;
	if (bfextu(src,d0,1,bufs)) goto l772;
	d4 = 4;
	goto l79e;

l772:
	d0 += 1;
	if (bfextu(src,d0,1,bufs)) goto l77e;
	d4 = 6;
	goto l79e;

l77e:
	d0 += 1;
	if (bfextu(src,d0,1,bufs)) goto l792;
	d0 += 1;
	d6 = bfextu(src,d0,3,bufs);
	d0 += 3;
	d6 += 8;
	goto l7a8;

l792:
	d0 += 1;
	d6 = bfextu(src,d0,5,bufs);
	d0 += 5;
	d4 = 16;
	goto l7a6;

l79e:
	d0 += 1;
	d6 = bfextu(src,d0,1,bufs);
	d0 += 1;
l7a6:
	d6 += d4;
l7a8:
	if(bfextu(src, d0, 1, bufs))
	{
		d5 = 12;
		a5 = -0x100;
	} else
	{
		d0 += 1;
		if(bfextu(src, d0, 1, bufs))
		{
			d5 = 14;
			a5 = -0x1100;
		} else
		{
			d5 = 8;
			a5 = 0;
		}
	}

	d0 += 1;
	d4 = bfextu(src,d0,d5,bufs);
	d0 += d5;
	d6 -= 3;
	if (d6 >= 0)
	{
		if (d6 > 0) d1 -= 1;
		d1 -= 1;
		if (d1 < 0) d1 = 0;
	}
	d6 += 2;
	phist = unpackedData.size() + a5 - d4 - 1;
	if(phist >= unpackedData.size())
		throw XPK_error();

	while ((d6 >= 0) && (cup1 > 0))
	{
		d3 = unpackedData[phist];
		phist++;
		unpackedData.push_back(static_cast<char>(d3));
		cup1--;
		d6--;
	}
	goto l74c;
}


static bool ValidateHeader(const XPKFILEHEADER &header)
{
	if(std::memcmp(header.XPKF, "XPKF", 4) != 0)
	{
		return false;
	}
	if(std::memcmp(header.SQSH, "SQSH", 4) != 0)
	{
		return false;
	}
	if(header.SrcLen == 0)
	{
		return false;
	}
	if(header.DstLen == 0)
	{
		return false;
	}
	static_assert(sizeof(XPKFILEHEADER) >= 8);
	if(header.SrcLen < (sizeof(XPKFILEHEADER) - 8))
	{
		return false;
	}
	return true;
}


static bool ValidateHeaderFileSize(const XPKFILEHEADER &header, uint64 filesize)
{
	if(filesize < header.SrcLen - 8)
	{
		return false;
	}
	return true;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderXPK(MemoryFileReader file, const uint64 *pfilesize)
{
	XPKFILEHEADER header;
	if(!file.ReadStruct(header))
	{
		return ProbeWantMoreData;
	}
	if(!ValidateHeader(header))
	{
		return ProbeFailure;
	}
	if(pfilesize)
	{
		if(!ValidateHeaderFileSize(header, *pfilesize))
		{
			return ProbeFailure;
		}
	}
	return ProbeSuccess;
}


bool UnpackXPK(std::vector<ContainerItem> &containerItems, FileReader &file, ContainerLoadingFlags loadFlags)
{
	file.Rewind();
	containerItems.clear();

	XPKFILEHEADER header;
	if(!file.ReadStruct(header))
	{
		return false;
	}
	if(!ValidateHeader(header))
	{
		return false;
	}
	if(loadFlags == ContainerOnlyVerifyHeader)
	{
		return true;
	}

	if(!file.CanRead(header.SrcLen - (sizeof(XPKFILEHEADER) - 8)))
	{
		return false;
	}

	containerItems.emplace_back();
	containerItems.back().data_cache = std::make_unique<std::vector<char> >();
	std::vector<char> & unpackedData = *(containerItems.back().data_cache);

	#ifdef MMCMP_LOG
		MPT_LOG_GLOBAL(LogDebug, "XPK", MPT_UFORMAT("XPK detected (SrcLen={} DstLen={}) filesize={}")(static_cast<uint32>(header.SrcLen), static_cast<uint32>(header.DstLen), file.GetLength()));
	#endif
	bool result = false;
	try
	{
		FileReader::PinnedView compressedData = file.GetPinnedView(header.SrcLen - (sizeof(XPKFILEHEADER) - 8));
		result = XPK_DoUnpack(compressedData.span(), unpackedData, header.DstLen);
	} catch(mpt::out_of_memory e)
	{
		mpt::delete_out_of_memory(e);
		return false;
	} catch(const XPK_error &)
	{
		return false;
	}

	if(result)
	{
		containerItems.back().file = FileReader(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(unpackedData)));
	}
	return result;
}


#endif // !MPT_WITH_ANCIENT


OPENMPT_NAMESPACE_END
