/* Copyright (C) Teemu Suutari */

#include "XPKUnimplemented.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool XPKUnimplemented::detectHeaderXPK(uint32_t hdr) noexcept
{
	auto &modes{getModes()};
	for (auto &mode : modes)
		if (mode.fourcc==hdr) return true;
	return false;
}

std::shared_ptr<XPKDecompressor> XPKUnimplemented::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<XPKUnimplemented>(hdr,recursionLevel,packedData,state,verify);
}

XPKUnimplemented::XPKUnimplemented(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_modeIndex{0}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
	auto &modes{getModes()};
	for (uint32_t i=0;i<modes.size();i++)
		if (modes[i].fourcc==hdr)
		{
			_modeIndex=i;
			break;
		}
}

const std::string &XPKUnimplemented::getSubName() const noexcept
{
	return getModes()[_modeIndex].name;
}

void XPKUnimplemented::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	// Only identify!
	throw Decompressor::DecompressionError();
}

std::vector<XPKUnimplemented::Mode> &XPKUnimplemented::getModes()
{
	// So here are the remaining XPK-libraries info. All there is, all there ever was.
	// (And I'm sure after writing that someone points me to 7 more formats)
	// Unimplemented reasons are as follows:
	// 1. Missing - there is no compressor available anywhere
	// 2. PowerPC only - Amiga OS 4 (or 3.5+) libraries that are nothing but modern compression methods wrapped in AmigaOS
	// 3. Floating point based formats - Fragile formats that require exact 68881/68882 semantics
	// 4. Encryption formats - Encryption formats requiring a proper passphrase
	static std::vector<Mode> modes={
		Mode{FourCC("BLFH"),"XPK-BLFH: Blowfish encryption (unimplemented)"},	// Encryption format
		Mode{FourCC("BZIP"),"XPK-BZIP: Bzip v1 (unimplemented)"},		// PowerPC only
		Mode{FourCC("CAST"),"XPK-CAST: CAST encryption (unimplemented)"},	// Encryption format
		Mode{FourCC("ENCO"),"XPK-ENCO: Unsafe encryption (unimplemented)"},	// Encryption format
		Mode{FourCC("DHUF"),"XPK-DHUF: Huffman compressor (unimplemented)"},	// Missing (All the libraries that exist are broken)
		Mode{FourCC("DMCB"),"XPK-DMCB: Arithmetic compressor (unimplemented)"},	// Floating point based format
		Mode{FourCC("DMCD"),"XPK-DMCD: Arithmetic compressor (unimplemented)"},	// Floating point based format
		Mode{FourCC("DMCI"),"XPK-DMCI: Arithmetic compressor (unimplemented)"},	// Missing
		Mode{FourCC("DMCU"),"XPK-DMCU: Arithmetic compressor (unimplemented)"},	// Floating point based format
		Mode{FourCC("FEAL"),"XPK-FEAL: FEAL-N encryption (unimplemented)"},	// Encryption format
		Mode{FourCC("IDEA"),"XPK-IDEA: IDEA encryption (unimplemented)"},	// Encryption format
		Mode{FourCC("L2XZ"),"XPK-L2XZ: LZMA2 compressor (unimplemented)"},	// PowerPC only
		Mode{FourCC("LZ40"),"XPK-LZ40: LZ4 compressor (unimplemented)"},	// PowerPC only
		Mode{FourCC("LZMA"),"XPK-LZMA: LZMA2 compressor (unimplemented)"},	// PowerPC only
		Mode{FourCC("NUID"),"XPK-NUID: IDEA encryption + NUKE (unimplemented)"},// Encryption format
		Mode{FourCC("SHID"),"XPK-SHID: IDEA encryption + SHRI (unimplemented)"},// Encryption format
		Mode{FourCC("TLTA"),"XPK-TLTA: TLTA encoder (unimplemented)"}};		// Missing, no idea what this is

	return modes;
}

}
