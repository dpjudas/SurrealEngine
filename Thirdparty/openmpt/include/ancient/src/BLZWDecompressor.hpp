/* Copyright (C) Teemu Suutari */

#ifndef BLZWDECOMPRESSOR_HPP
#define BLZWDECOMPRESSOR_HPP

#include "XPKDecompressor.hpp"

namespace ancient::internal
{

class BLZWDecompressor : public XPKDecompressor
{
public:
	BLZWDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);

	~BLZWDecompressor() noexcept=default;

	const std::string &getSubName() const noexcept final;

	void decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify) final;

	static bool detectHeaderXPK(uint32_t hdr);
	static std::shared_ptr<XPKDecompressor> create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);

private:
	const Buffer	&_packedData;

	uint32_t	_maxBits{0};
	uint32_t	_stackLength{0};
};

}

#endif
