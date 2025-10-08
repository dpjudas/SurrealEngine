/* Copyright (C) Teemu Suutari */

#ifndef PPDECOMPRESSOR_HPP
#define PPDECOMPRESSOR_HPP

#include "Decompressor.hpp"
#include "XPKDecompressor.hpp"
#include "InputStream.hpp"

#include <array>

namespace ancient::internal
{

class PPDecompressor : public Decompressor, public XPKDecompressor
{
private:
	class PPState : public XPKDecompressor::State
	{
	public:
		PPState(uint32_t mode) noexcept;
		~PPState() noexcept=default;

		uint32_t _cachedMode;
	};

public:
	PPDecompressor(const Buffer &packedData,bool exactSizeKnown,bool verify);
	PPDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);
	~PPDecompressor() noexcept=default;

	const std::string &getName() const noexcept final;
	const std::string &getSubName() const noexcept final;

	size_t getPackedSize() const noexcept final;
	size_t getRawSize() const noexcept final;

	void decompressImpl(Buffer &rawData,bool verify) final;
	void decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify) final;

	static bool detectHeader(uint32_t hdr,uint32_t footer) noexcept;
	static bool detectHeaderXPK(uint32_t hdr) noexcept;

	static std::shared_ptr<Decompressor> create(const Buffer &packedData,bool exactSizeKnown,bool verify);
	static std::shared_ptr<XPKDecompressor> create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify);

private:
	class DoneException : public std::exception
	{
	public:
		DoneException(uint32_t key) noexcept : _key(key) {}
		~DoneException() noexcept=default;

		uint32_t getKey() const noexcept { return _key; }

	private:
		uint32_t	_key;
	};

	void findKeyRound(BackwardInputStream &inputStream,LSBBitReader<BackwardInputStream> &bitReader,uint32_t keyBits,uint32_t keyMask,uint32_t outputPosition,uint32_t &iterCount);
	void findKey(uint32_t keyBits,uint32_t keyMask);

	const Buffer		&_packedData;

	size_t			_dataStart{0};
	size_t			_rawSize{0};
	uint8_t			_startShift{0};
	std::array<uint8_t,4>	_modeTable;
	bool			_isObsfuscated{false};
	bool			_isXPK{false};
};

}

#endif
