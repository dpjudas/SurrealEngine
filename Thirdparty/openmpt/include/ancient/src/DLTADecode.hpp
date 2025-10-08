/* Copyright (C) Teemu Suutari */

#ifndef DLTADECODE_HPP
#define DLTADECODE_HPP

#include "XPKDecompressor.hpp"

namespace ancient::internal
{


class DLTADecode : public XPKDecompressor
{
public:
	DLTADecode(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);
	~DLTADecode() noexcept=default;

	const std::string &getSubName() const noexcept final;

	void decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify) final;

	static bool detectHeaderXPK(uint32_t hdr) noexcept;
	static std::shared_ptr<XPKDecompressor> create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);

	// static method for easy external usage. Buffers can be the same for in-place replacement
	static void decode(Buffer &bufferDest,const Buffer &bufferSrc,size_t offset,size_t size);

private:
	const Buffer	&_packedData;
};

}

#endif
