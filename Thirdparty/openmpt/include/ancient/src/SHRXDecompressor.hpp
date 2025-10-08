/* Copyright (C) Teemu Suutari */

#ifndef SHRXDECOMPRESSOR_HPP
#define SHRXDECOMPRESSOR_HPP

#include "XPKDecompressor.hpp"

#include <array>

namespace ancient::internal
{

class SHRXDecompressor : public XPKDecompressor
{
private:
	class SHRXState : public XPKDecompressor::State
	{
	public:
		SHRXState() noexcept;
		virtual ~SHRXState() noexcept=default;

		uint32_t vlen{0};
		uint32_t vnext{0};
		uint32_t shift{0};
		std::array<uint32_t,999> ar;
	};

public:
	SHRXDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);
	~SHRXDecompressor() noexcept=default;

	const std::string &getSubName() const noexcept final;

	void decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify) final;

	static bool detectHeaderXPK(uint32_t hdr) noexcept;
	static std::shared_ptr<XPKDecompressor> create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);

private:
	const Buffer				&_packedData;

	uint32_t				_ver{0};
	size_t					_startOffset{0};
	size_t					_rawSize{0};
	bool					_isSHR3{false};

	std::shared_ptr<XPKDecompressor::State>	&_state;	// reference!!!
};

}

#endif
