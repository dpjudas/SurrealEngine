/* Copyright (C) Teemu Suutari */

#ifndef MMCMPDECOMPRESSOR_HPP
#define MMCMPDECOMPRESSOR_HPP

#include "Decompressor.hpp"

namespace ancient::internal
{

class MMCMPDecompressor : public Decompressor
{
public:
	MMCMPDecompressor(const Buffer &packedData,bool exactSizeKnown,bool verify);
	~MMCMPDecompressor() noexcept=default;

	const std::string &getName() const noexcept final;
	size_t getPackedSize() const noexcept final;
	size_t getRawSize() const noexcept final;

	void decompressImpl(Buffer &rawData,bool verify) final;

	static bool detectHeader(uint32_t hdr,uint32_t footer) noexcept;
	static std::shared_ptr<Decompressor> create(const Buffer &packedData,bool exactSizeKnown,bool verify);

private:
	const Buffer	&_packedData;

	uint32_t	_packedSize{0};
	uint32_t	_rawSize{0};
	uint32_t	_blocksOffset{0};
	uint32_t	_blocks{0};
	uint16_t	_version{0};
};

}

#endif
