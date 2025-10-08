/* Copyright (C) Teemu Suutari */

#ifndef FREEZEDECOMPRESSOR_HPP
#define FREEZEDECOMPRESSOR_HPP

#include "Decompressor.hpp"

#include <array>

namespace ancient::internal
{

class FreezeDecompressor : public Decompressor
{
public:
	FreezeDecompressor(const Buffer &packedData,bool exactSizeKnown,bool verify);
	~FreezeDecompressor() noexcept=default;

	size_t getRawSize() const noexcept final;
	size_t getPackedSize() const noexcept final;

	const std::string &getName() const noexcept final;

	void decompressImpl(Buffer &rawData,bool verify) final;

	static bool detectHeader(uint32_t hdr,uint32_t footer) noexcept;

	static std::shared_ptr<Decompressor> create(const Buffer &packedData,bool exactSizeKnown,bool verify);

private:
	const Buffer	&_packedData;

	size_t		_packedSize{0};
	size_t		_rawSize{0};
	bool		_isOldVersion;
	bool		_exactSizeKnown;

	std::array<uint8_t,8> _hufTable;
};

}

#endif
