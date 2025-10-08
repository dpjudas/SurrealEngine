/* Copyright (C) Teemu Suutari */

#ifndef LZXDECOMPRESSOR_HPP
#define LZXDECOMPRESSOR_HPP

#include "XPKDecompressor.hpp"

namespace ancient::internal
{

class LZXDecompressor : public XPKDecompressor
{
public:
	LZXDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);
	~LZXDecompressor() noexcept=default;

	const std::string &getSubName() const noexcept final;

	void decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify) final;

	static bool detectHeaderXPK(uint32_t hdr) noexcept;
	static std::shared_ptr<XPKDecompressor> create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);

private:
	const Buffer	&_packedData;

	bool		_isSampled{false};
	bool		_isCompressed{false};
	size_t		_packedSize{0};
	size_t		_packedOffset{0};
	size_t		_rawSize{0};
	uint32_t	_rawCRC{0};
};

}

#endif
