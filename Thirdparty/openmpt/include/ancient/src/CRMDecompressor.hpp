/* Copyright (C) Teemu Suutari */

#ifndef CRMDECOMPRESSOR_HPP
#define CRMDECOMPRESSOR_HPP

#include "Decompressor.hpp"
#include "XPKDecompressor.hpp"

namespace ancient::internal
{

class CRMDecompressor : public Decompressor, public XPKDecompressor
{
public:
	CRMDecompressor(const Buffer &packedData,uint32_t recursionLevel,bool verify);
	CRMDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);
	~CRMDecompressor() noexcept=default;

	const std::string &getName() const noexcept final;
	const std::string &getSubName() const noexcept final;

	size_t getPackedSize() const noexcept final;
	size_t getRawSize() const noexcept final;

	void decompressImpl(Buffer &rawData,bool verify) final;
	void decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify) final;

	static bool detectHeader(uint32_t hdr,uint32_t footer) noexcept;
	static bool detectHeaderXPK(uint32_t hdr) noexcept;

	static std::shared_ptr<Decompressor> create(const Buffer &packedData,bool exactSizeKnown,bool verify);
	static std::shared_ptr<XPKDecompressor> create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);

private:
	const Buffer	&_packedData;

	uint32_t	_packedSize{0};
	uint32_t	_rawSize{0};
	bool		_isLZH{false};		// "normal" compression or LZH compression
	bool		_isSampled{false};	// normal or "sampled" i.e. obsfuscated
	bool		_isXPKDelta{false};	// If delta encoding defined in XPK
};

}

#endif
