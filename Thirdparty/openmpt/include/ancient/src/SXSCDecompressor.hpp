/* Copyright (C) Teemu Suutari */

#ifndef SXSCDECOMPRESSOR_HPP
#define SXSCDECOMPRESSOR_HPP

#include <cstdint>

#include "XPKDecompressor.hpp"
#include "InputStream.hpp"
#include "RangeDecoder.hpp"

namespace ancient::internal
{

class SXSCDecompressor : public XPKDecompressor
{
public:
	SXSCDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);
	~SXSCDecompressor() noexcept=default;

	const std::string &getSubName() const noexcept final;

	void decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify) final;

	static bool detectHeaderXPK(uint32_t hdr) noexcept;
	static std::shared_ptr<XPKDecompressor> create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);

private:
	class SXSCReader : public RangeDecoder::BitReader
	{
	public:
		SXSCReader(ForwardInputStream &stream);
		~SXSCReader() noexcept=default;

		uint32_t readBit() final;

	private:
		MSBBitReader<ForwardInputStream>	_reader;
	};

	void decompressASC(Buffer &rawData,ForwardInputStream &inputStream);
	void decompressHSC(Buffer &rawData,ForwardInputStream &inputStream);

	const Buffer					&_packedData;
	bool						_isHSC;
};

}

#endif
