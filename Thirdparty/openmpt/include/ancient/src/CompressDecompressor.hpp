/* Copyright (C) Teemu Suutari */

#ifndef COMPRESSDECOMPRESSOR_HPP
#define COMPRESSDECOMPRESSOR_HPP

#include "Decompressor.hpp"

namespace ancient::internal
{

class CompressDecompressor : public Decompressor
{
public:
	CompressDecompressor(const Buffer &packedData,bool exactSizeKnown,bool verify);
	~CompressDecompressor() noexcept=default;

	size_t getRawSize() const noexcept final;
	size_t getPackedSize() const noexcept final;

	const std::string &getName() const noexcept final;

	void decompressImpl(Buffer &rawData,bool verify) final;

	static bool detectHeader(uint32_t hdr,uint32_t footer) noexcept;

	static std::shared_ptr<Decompressor> create(const Buffer &packedData,bool exactSizeKnown,bool verify);

private:
	const Buffer	&_packedData;

	size_t		_rawSize{0};

	bool		_hasBlocks;
	uint32_t	_maxBits;
};

}

#endif
