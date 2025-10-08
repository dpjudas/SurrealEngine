/* Copyright (C) Teemu Suutari */

#ifndef PMCDECOMPRESSOR_HPP
#define PMCDECOMPRESSOR_HPP

#include "Decompressor.hpp"

namespace ancient::internal
{

class PMCDecompressor : public Decompressor
{
public:
	PMCDecompressor(const Buffer &packedData,bool verify);
	~PMCDecompressor() noexcept=default;

	const std::string &getName() const noexcept final;
	size_t getPackedSize() const noexcept final;
	size_t getRawSize() const noexcept final;

	void decompressImpl(Buffer &rawData,bool verify) final;

	static bool detectHeader(uint32_t hdr,uint32_t footer) noexcept;
	static std::shared_ptr<Decompressor> create(const Buffer &packedData,bool exactSizeKnown,bool verify);

private:
	const Buffer	&_packedData;

	uint32_t	_rawSize{0};
	size_t		_packedSize{0};
	uint32_t	_ver;
};

}

#endif
