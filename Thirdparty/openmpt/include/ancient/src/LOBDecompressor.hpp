/* Copyright (C) Teemu Suutari */

#ifndef LOBDECOMPRESSOR_HPP
#define LOBDECOMPRESSOR_HPP

#include "Decompressor.hpp"

namespace ancient::internal
{

class LOBDecompressor : public Decompressor
{
public:
	LOBDecompressor(const Buffer &packedData,bool verify);
	~LOBDecompressor() noexcept=default;

	const std::string &getName() const noexcept final;
	size_t getPackedSize() const noexcept final;
	size_t getRawSize() const noexcept final;

	void decompressImpl(Buffer &rawData,bool verify) final;

	static bool detectHeader(uint32_t hdr,uint32_t footer) noexcept;
	static std::shared_ptr<Decompressor> create(const Buffer &packedData,bool exactSizeKnown,bool verify);

private:
	static void decompressRound(Buffer &rawData,const Buffer &packedData);

	const Buffer	&_packedData;

	uint32_t	_rawSize{0};
	uint32_t	_packedSize{0};

	uint32_t	_methodCount;
};

}

#endif
