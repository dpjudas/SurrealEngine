/* Copyright (C) Teemu Suutari */

#ifndef BZIP2DECOMPRESSOR_HPP
#define BZIP2DECOMPRESSOR_HPP

#include "Decompressor.hpp"
#include "XPKDecompressor.hpp"

namespace ancient::internal
{

class BZIP2Decompressor : public Decompressor, public XPKDecompressor
{
public:
	BZIP2Decompressor(const Buffer &packedData,bool exactSizeKnown,bool verify);
	BZIP2Decompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);
	~BZIP2Decompressor() noexcept=default;

	size_t getRawSize() const noexcept final;
	size_t getPackedSize() const noexcept final;

	const std::string &getName() const noexcept final;
	const std::string &getSubName() const noexcept final;

	void decompressImpl(Buffer &rawData,bool verify) final;
	void decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify) final;

	static bool detectHeader(uint32_t hdr,uint32_t footer) noexcept;
	static bool detectHeaderXPK(uint32_t hdr) noexcept;

	static std::shared_ptr<Decompressor> create(const Buffer &packedData,bool exactSizeKnown,bool verify);
	static std::shared_ptr<XPKDecompressor> create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);

private:
	const Buffer		&_packedData;

	size_t			_blockSize{0};
	size_t			_packedSize{0};
	size_t			_rawSize{0};
};

}

#endif
