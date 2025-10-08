/* Copyright (C) Teemu Suutari */

#ifndef LHDECOMPRESSOR_HPP
#define LHDECOMPRESSOR_HPP

#include "XPKDecompressor.hpp"

namespace ancient::internal
{

class LHDecompressor : public XPKDecompressor
{
public:
	LHDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);
	~LHDecompressor() noexcept=default;

	const std::string &getSubName() const noexcept final;

	void decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify) final;

	static bool detectHeaderXPK(uint32_t hdr) noexcept;
	static std::shared_ptr<XPKDecompressor> create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);

	static void decompressLhLib(Buffer &rawData,const Buffer &packedData);

private:
	const Buffer	&_packedData;
};

}

#endif
