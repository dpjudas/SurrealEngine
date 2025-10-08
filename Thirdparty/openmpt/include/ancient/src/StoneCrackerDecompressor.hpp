/* Copyright (C) Teemu Suutari */

#ifndef STONECRACKERDECOMPRESSOR_HPP
#define STONECRACKERDECOMPRESSOR_HPP

#include "Decompressor.hpp"

#include <array>

namespace ancient::internal
{

class StoneCrackerDecompressor : public Decompressor
{
public:
	StoneCrackerDecompressor(const Buffer &packedData,bool exactSizeKnown,bool verify);
	~StoneCrackerDecompressor() noexcept=default;

	const std::string &getName() const noexcept final;

	size_t getPackedSize() const noexcept final;
	size_t getRawSize() const noexcept final;

	void decompressImpl(Buffer &rawData,bool verify) final;

	static bool detectHeader(uint32_t hdr,uint32_t footer) noexcept;

	static std::shared_ptr<Decompressor> create(const Buffer &packedData,bool exactSizeKnown,bool verify);

private:
	static bool detectHeaderAndGeneration(uint32_t hdr,uint32_t &generation) noexcept;

	void initialize(const Buffer &packedData,uint32_t hdr);
	void decompressGen1(Buffer &rawData);
	void decompressGen23(Buffer &rawData);
	void decompressGen456(Buffer &rawData);
	void decompressGen7(Buffer &rawData);
	void decompressGen8(Buffer &rawData);

	const Buffer	&_packedData;

	uint32_t		_rawSize{0};
	uint32_t		_packedSize{0};
	uint32_t		_rleSize{0};
	std::array<uint8_t,4>	_modes;
	std::array<uint8_t,3>	_rle;
	uint32_t		_generation;
	uint32_t		_dataOffset;
};

}

#endif
