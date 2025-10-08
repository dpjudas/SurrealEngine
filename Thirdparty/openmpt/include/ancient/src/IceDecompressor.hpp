/* Copyright (C) Teemu Suutari */

#ifndef ICEDECOMPRESSOR_HPP
#define ICEDECOMPRESSOR_HPP

#include "Decompressor.hpp"

namespace ancient::internal
{

class IceDecompressor : public Decompressor
{
public:
	IceDecompressor(const Buffer &packedData,bool exactSizeKnown,bool verify);
	~IceDecompressor() noexcept=default;

	const std::string &getName() const noexcept final;
	size_t getPackedSize() const noexcept final;
	size_t getRawSize() const noexcept final;

	void decompressImpl(Buffer &rawData,bool verify) final;

	static bool detectHeader(uint32_t hdr,uint32_t footer) noexcept;
	static std::shared_ptr<Decompressor> create(const Buffer &packedData,bool exactSizeKnown,bool verify);

private:
	void decompressInternal(Buffer &rawData,bool useBytes);

	const Buffer	&_packedData;

	uint32_t	_rawSize{0};
	size_t		_packedSize{0};
	uint32_t	_ver;
};

}

#endif
