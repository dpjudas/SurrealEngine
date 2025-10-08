/* Copyright (C) Teemu Suutari */

#ifndef XPKMAIN_HPP
#define XPKMAIN_HPP

#include "Decompressor.hpp"
#include "XPKDecompressor.hpp"

#include <vector>

namespace ancient::internal
{

class XPKMain : public Decompressor
{
private:
	XPKMain(const Buffer &packedData,bool verify,uint32_t recursionLevel);

public:
	~XPKMain() noexcept=default;

	const std::string &getName() const noexcept final;
	size_t getPackedSize() const noexcept final;
	size_t getRawSize() const noexcept final;

	void decompressImpl(Buffer &rawData,bool verify) final;

	static bool detectHeader(uint32_t hdr,uint32_t footer) noexcept;

	static std::shared_ptr<Decompressor> create(const Buffer &packedData,bool exactSizeKnown,bool verify);

	// Can be used for direct recursion
	static std::shared_ptr<Decompressor> createDecompressor(uint32_t recursionLevel,const Buffer &buffer,bool verify);
	// Can be used to create directly decoder for chunk (needed by CYB2)
	static std::shared_ptr<XPKDecompressor> createDecompressor(uint32_t type,uint32_t recursionLevel,const Buffer &buffer,std::shared_ptr<XPKDecompressor::State> &state,bool verify);

private:
	static void registerDecompressor(bool(*detect)(uint32_t),std::shared_ptr<XPKDecompressor>(*create)(uint32_t,uint32_t,const Buffer&,std::shared_ptr<XPKDecompressor::State>&,bool));
	static constexpr uint32_t getMaxRecursionLevel() noexcept { return 4; }

	template <typename F>
	void forEachChunk(F func) const;

	const Buffer	&_packedData;

	uint32_t	_packedSize{0};
	uint32_t	_rawSize{0};
	uint32_t	_headerSize{0};
	uint32_t	_type{0};
	bool		_longHeaders{false};
	uint32_t	_recursionLevel{0};
	bool		_hasPassword{false};
};

}

#endif
