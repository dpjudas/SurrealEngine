/* Copyright (C) Teemu Suutari */

#ifndef RNCDECOMPRESSOR_HPP
#define RNCDECOMPRESSOR_HPP

#include "Decompressor.hpp"

namespace ancient::internal
{

class RNCDecompressor : public Decompressor
{
public:
	RNCDecompressor(const Buffer &packedData,bool verify);
	~RNCDecompressor() noexcept=default;

	const std::string &getName() const noexcept final;
	size_t getPackedSize() const noexcept final;
	size_t getRawSize() const noexcept final;

	void decompressImpl(Buffer &rawData,bool verify) final;

	static bool detectHeader(uint32_t hdr,uint32_t footer) noexcept;

	static std::shared_ptr<Decompressor> create(const Buffer &packedData,bool exactSizeKnown,bool verify);

private:
	enum class Version
	{
		RNC1Old=0,
		RNC1New,
		RNC2Old,
		RNC2New
	};

	void RNCDecompressOld(Buffer &rawData,bool verify,bool rnc2);
	void RNC1DecompressNew(Buffer &rawData,bool verify);
	void RNC2DecompressNew(Buffer &rawData,bool verify);

	const Buffer	&_packedData;

	uint32_t	_rawSize{0};
	uint32_t	_packedSize{0};
	uint16_t	_rawCRC{0};
	uint8_t		_chunks{0};
	Version		_ver;
};

}

#endif
