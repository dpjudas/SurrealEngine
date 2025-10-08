/* Copyright (C) Teemu Suutari */

#ifndef LZW4DECOMPRESSOR_HPP
#define LZW4DECOMPRESSOR_HPP

#include "XPKDecompressor.hpp"

namespace ancient::internal
{

class LZW4Decompressor : public XPKDecompressor
{
public:
	LZW4Decompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);
	~LZW4Decompressor() noexcept=default;

	const std::string &getSubName() const noexcept final;

	void decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify) final;

	static bool detectHeaderXPK(uint32_t hdr) noexcept;
	static std::shared_ptr<XPKDecompressor> create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);

private:
	const Buffer	&_packedData;
};

}

#endif
