/* Copyright (C) Teemu Suutari */

#ifndef XPKUNIMPLEMENTED_HPP
#define XPKUNIMPLEMENTED_HPP

#include "XPKDecompressor.hpp"

namespace ancient::internal
{

class XPKUnimplemented : public XPKDecompressor
{
public:
	XPKUnimplemented(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);
	~XPKUnimplemented() noexcept=default;

	const std::string &getSubName() const noexcept final;

	void decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify) final;

	static bool detectHeaderXPK(uint32_t hdr) noexcept;
	static std::shared_ptr<XPKDecompressor> create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);

private:
	struct Mode
	{
		uint32_t	fourcc;
		std::string	name;
	};

	static std::vector<Mode> &getModes();

	uint32_t		_modeIndex;
};

}

#endif
