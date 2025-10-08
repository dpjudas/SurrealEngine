/* Copyright (C) Teemu Suutari */

#ifndef ANCIENT_ANCIENT_HPP
#define ANCIENT_ANCIENT_HPP

#ifndef ANCIENT_API
#if defined(ANCIENT_API_DECLSPEC_DLLEXPORT)
#define ANCIENT_API __declspec(dllexport)
#endif
#if defined(ANCIENT_API_DECLSPEC_DLLIMPORT)
#define ANCIENT_API __declspec(dllimport)
#endif
#if defined(ANCIENT_API_VISIBILITY_DEFAULT_USED)
#define ANCIENT_API __attribute__((visibility("default"))) __attribute__((used))
#endif
#if defined(ANCIENT_API_VISIBILITY_DEFAULT)
#define ANCIENT_API __attribute__((visibility("default")))
#endif
#if defined(ANCIENT_API_VISIBILITY_HIDDEN)
#define ANCIENT_API __attribute__((visibility("hidden")))
#endif
#ifndef ANCIENT_API
#define ANCIENT_API
#endif
#endif

#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdint>

namespace ancient
{

namespace internal
{
namespace APIv2
{
class DecompressorImpl;
}
}

inline namespace APIv2
{

// just a base class to easily catch all the errors
class ANCIENT_API Error : public std::exception
{
public:
	Error() noexcept;
	virtual ~Error() noexcept;
};

class ANCIENT_API InvalidFormatError : public Error
{
public:
	InvalidFormatError() noexcept;
	~InvalidFormatError() noexcept;
};

class ANCIENT_API DecompressionError : public Error
{
public:
	DecompressionError() noexcept;
	~DecompressionError() noexcept;
};

class ANCIENT_API VerificationError : public Error
{
public:
	VerificationError() noexcept;
	~VerificationError() noexcept;
};

class ANCIENT_API Decompressor final
{

public:

	// Detect signature whether it matches to any known compressor
	// This does not guarantee the data is decompressable though, only signature is read
	static bool detect(const std::vector<uint8_t> &packedData) noexcept;
	static bool detect(const uint8_t *packedData,size_t packedSize) noexcept;

	// Main entrypoint
	// if verify=true then check the packedData for errors: CRC or other checksum if available
	// check exactSizeKnown from size documentation
	// can throw InvalidFormatError if stream is not recognized or it is invalid
	// can throw VerificationError if verify enabled and checksum does not match
	explicit Decompressor(const std::vector<uint8_t> &packedData,bool exactSizeKnown,bool verify);
	explicit Decompressor(const uint8_t *packedData,size_t packedSize,bool exactSizeKnown,bool verify);

	// Name returned is human readable long name
	const std::string &getName() const noexcept;

	// the functions are there to protect against "accidental" large files when parsing headers
	// a.k.a. 16M should be enough for everybody (sizes do not have to accurate i.e.
	// compressors can exclude header content for simplification)
	// This entirely ok for the context of "old computers" and their files,
	// for other usages these need to be tuned up
	static size_t getMaxPackedSize() noexcept;
	static size_t getMaxRawSize() noexcept;

	// PackedSize or RawSize are taken from the stream if available, std::nullopt otherwise.
	// for those compressors having no known sizes, running decompression will update
	// the values.
	// There are exceptions: Some decompressors need to exact size of the packed data
	// in order to decompress. For those providing a indefinitely size packed stream
	// will not work
	// use the "exactSizeKnown" flag for create to tell whether you know the size or not
	std::optional<size_t> getPackedSize() const noexcept;
	std::optional<size_t> getRawSize() const noexcept;

	// in case of disk image based formats the data does not necessarily start
	// from logical beginnig of the image but it is offsetted inside the logical image (f.e. DMS).
	// getDataOffset will return the offset which can also be 0, or std::nullopt if not image-based format.
	// getImageSize will return the size of the the logical image, or std::nullopt if not image-based format.
	std::optional<size_t> getImageSize() const noexcept;
	std::optional<size_t> getImageOffset() const noexcept;

	// Actual decompression.
	// verify checksum if verify==true
	// can throw DecompressionError if stream cant be unpacked
	// can throw VerificationError if verify enabled and checksum does not match
	std::vector<uint8_t> decompress(bool verify);

	~Decompressor() noexcept;

private:

	std::unique_ptr<internal::APIv2::DecompressorImpl> m_impl;

private:

	Decompressor(const Decompressor&)=delete;
	Decompressor& operator=(const Decompressor&)=delete;

};

}

}

#endif
