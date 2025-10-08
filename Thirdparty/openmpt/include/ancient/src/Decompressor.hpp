/* Copyright (C) Teemu Suutari */

#ifndef DECOMPRESSOR_HPP
#define DECOMPRESSOR_HPP

#include <cstddef>
#include <cstdint>

#include <memory>
#include <string>

#include "common/Buffer.hpp"
#include "ancient.hpp"

namespace ancient::internal
{

class Decompressor
{
protected:
	Decompressor() noexcept=default;

public:

	using Error = ancient::Error;
	using InvalidFormatError = ancient::InvalidFormatError;
	using DecompressionError = ancient::DecompressionError;
	using VerificationError = ancient::VerificationError;

	Decompressor(const Decompressor&)=delete;
	Decompressor& operator=(const Decompressor&)=delete;

	virtual ~Decompressor() noexcept=default;

	// Name returned is human readable long name
	virtual const std::string &getName() const noexcept=0;

	// PackedSize or RawSize are taken from the stream if available, 0 otherwise.
	// for those compressors having 0 sizes, running decompression will update
	// the values. (make sure to allocate big-enough buffer for decompression)
	// There are exceptions: Some decompressors need to exact size of the packed data
	// in order to decompress. For those providing a indefinitely size packed stream
	// will not work
	// use the "exactSizeKnown" flag for create to tell whether you know the size or not
	virtual size_t getPackedSize() const noexcept=0;
	virtual size_t getRawSize() const noexcept=0;

	// Actual decompression.
	// verify checksum if verify==true
	// can throw DecompressionError if stream cant be unpacked
	// can throw VerificationError if verify enabled and checksum does not match
	void decompress(Buffer &rawData,bool verify);

	// in case of disk image based formats the data does not necessarily start
	// from logical beginnig of the image but it is offsetted inside the logical image
	// (f.e. DMS). getDataOffset will return the offset (or 0 if not relevant or if offset does not exist)
	// getImageSize will return the size of the the logical image, or 0 if not image-based format
	virtual size_t getImageSize() const noexcept;
	virtual size_t getImageOffset() const noexcept;

	// the functions are there to protect against "accidental" large files when parsing headers
	// a.k.a. 16M should be enough for everybody (sizes do not have to accurate i.e.
	// compressors can exclude header content for simplification)
	// This entirely ok for the context of "old computers" and their files,
	// for other usages these need to be tuned up
	static size_t getMaxPackedSize() noexcept;
	static size_t getMaxRawSize() noexcept;

	// Main entrypoint
	// if verify=true then check the packedData for errors: CRC or other checksum if available
	// check exactSizeKnown from size documentation
	// can throw InvalidFormatError if stream is not recognized or it is invalid
	// can throw VerificationError if verify enabled and checksum does not match
	static std::shared_ptr<Decompressor> create(const Buffer &packedData,bool exactSizeKnown,bool verify);

	// Detect signature whether it matches to any known compressor
	// This does not guarantee the data is decompressable though, only signature(s) is read
	static bool detect(const Buffer &packedData,bool exactSizeKnown) noexcept;

protected:
	virtual void decompressImpl(Buffer &rawData,bool verify)=0;
};

}

#endif
