/* Copyright (C) Teemu Suutari */

#ifndef DMSDECOMPRESSOR_HPP
#define DMSDECOMPRESSOR_HPP

#include "Decompressor.hpp"

namespace ancient::internal
{


class DMSDecompressor : public Decompressor
{
public:
	DMSDecompressor(const Buffer &packedData,bool verify);
	~DMSDecompressor() noexcept=default;

	const std::string &getName() const noexcept final;
	size_t getPackedSize() const noexcept final;
	size_t getRawSize() const noexcept final;

	size_t getImageSize() const noexcept final;
	size_t getImageOffset() const noexcept final;

	void decompressImpl(Buffer &rawData,bool verify) final;

	static bool detectHeader(uint32_t hdr,uint32_t footer) noexcept;
	static std::shared_ptr<Decompressor> create(const Buffer &packedData,bool exactSizeKnown,bool verify);

private:
	void decompressImpl(Buffer &rawData,bool verify,uint32_t &restartPosition);

	const Buffer	&_packedData;

	uint32_t	_packedSize{0};
	uint32_t	_rawSize{0};
	uint32_t	_contextBufferSize{0};
	uint32_t	_tmpBufferSize{0};
	uint32_t	_imageSize;
	uint32_t	_rawOffset;
	uint32_t	_minTrack;
	bool		_isHD;
	bool		_isObsfuscated;
};

}

#endif
