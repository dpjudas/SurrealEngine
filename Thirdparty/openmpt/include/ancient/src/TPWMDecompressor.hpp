/* Copyright (C) Teemu Suutari */

#ifndef TPWMDECOMPRESSOR_HPP
#define TPWMDECOMPRESSOR_HPP

#include "Decompressor.hpp"

namespace ancient::internal
{

class TPWMDecompressor : public Decompressor
{
public:
	TPWMDecompressor(const Buffer &packedData,bool verify);
	~TPWMDecompressor() noexcept=default;

	const std::string &getName() const noexcept final;
	size_t getPackedSize() const noexcept final;
	size_t getRawSize() const noexcept final;

	void decompressImpl(Buffer &rawData,bool verify) final;

	static bool detectHeader(uint32_t hdr,uint32_t footer) noexcept;
	static std::shared_ptr<Decompressor> create(const Buffer &packedData,bool exactSizeKnown,bool verify);

private:
	const Buffer	&_packedData;

	uint32_t	_rawSize{0};
	size_t		_decompressedPackedSize{0};
};

}

#endif
