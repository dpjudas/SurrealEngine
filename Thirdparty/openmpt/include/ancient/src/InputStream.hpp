/* Copyright (C) Teemu Suutari */

#ifndef INPUTSTREAM_HPP
#define INPUTSTREAM_HPP

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <memory>

#include "common/Buffer.hpp"

// for exceptions
#include "Decompressor.hpp"

namespace ancient::internal
{

class BackwardInputStream;

class ForwardInputStream
{
	friend class BackwardInputStream;

public:
	ForwardInputStream(const Buffer &buffer,size_t startOffset,size_t endOffset,size_t overrunAllowance=0);
	~ForwardInputStream() noexcept=default;

	void reset(size_t startOffset,size_t endOffset);

	uint8_t readByte();
	uint16_t readBE16();
	uint32_t readBE32();
	uint16_t readLE16();
	uint32_t readLE32();
	std::shared_ptr<const Buffer> consume(size_t bytes);

	bool eof() const noexcept { return _currentOffset==_endOffset; }
	size_t getOffset() const noexcept { return _currentOffset; }
	size_t getEndOffset() const noexcept { return _endOffset; }
	void link(BackwardInputStream &stream) noexcept { _linkedInputStream=&stream; }

	size_t available() const noexcept { return _endOffset-_currentOffset; }
	void setOffset(size_t offset);

private:
	void setEndOffset(size_t offset) noexcept { _endOffset=offset; }

	const Buffer		&_buffer;
	size_t			_currentOffset;
	size_t			_endOffset;
	size_t			_overrunAllowance;

	BackwardInputStream	*_linkedInputStream{nullptr};
};


class BackwardInputStream
{
	friend class ForwardInputStream;
public:
	BackwardInputStream(const Buffer &buffer,size_t startOffset,size_t endOffset);
	~BackwardInputStream() noexcept=default;

	uint8_t readByte();
	uint16_t readBE16();
	uint32_t readBE32();
	uint16_t readLE16();
	uint32_t readLE32();

	bool eof() const noexcept { return _currentOffset==_endOffset; }
	size_t getOffset() const noexcept { return _currentOffset; }
	void link(ForwardInputStream &stream) noexcept { _linkedInputStream=&stream; }

	size_t available() const noexcept { return _currentOffset-_endOffset; }
	void setOffset(size_t offset);

private:
	void setEndOffset(size_t offset) noexcept { _endOffset=offset; }

	const Buffer		&_buffer;
	size_t			_currentOffset;
	size_t			_endOffset;

	ForwardInputStream	*_linkedInputStream{nullptr};
};


template<typename T>
class LSBBitReader
{
public:
	LSBBitReader(T &inputStream) noexcept :
		_inputStream{inputStream}
	{
		// nothing needed
	}
	~LSBBitReader() noexcept=default;

	uint32_t readBits8(uint32_t count)
	{
		return readBitsGeneric(count,[&](){
			return std::make_pair(_inputStream.readByte(),uint8_t(8));
		});
	}

	uint32_t readBitsBE16(uint32_t count)
	{
		return readBitsGeneric(count,[&](){
			return std::make_pair(_inputStream.readBE16(),uint8_t(16));
		});
	}

	uint32_t readBitsBE32(uint32_t count)
	{
		return readBitsGeneric(count,[&](){
			return std::make_pair(_inputStream.readBE32(),uint8_t(32));
		});
	}

	uint32_t readBitsLE16(uint32_t count)
	{
		return readBitsGeneric(count,[&](){
			return std::make_pair(_inputStream.readLE16(),uint8_t(16));
		});
	}

	uint32_t getBufContent() const noexcept { return _bufContent; }
	uint8_t getBufLength() const noexcept { return _bufLength; }

	void reset(uint32_t bufContent=0,uint8_t bufLength=0) noexcept
	{
		_bufContent=bufContent;
		_bufLength=bufLength;
	}

	template<typename F>
	uint32_t readBitsGeneric(uint32_t count,F readWord)
	{
		uint32_t ret{0};
		uint32_t pos{0};
		if (count>32U)
			throw Decompressor::DecompressionError();
		while (count)
		{
			if (!_bufLength)
				std::tie(_bufContent,_bufLength)=readWord();
			uint8_t maxCount{std::min(uint8_t(count),_bufLength)};
			ret|=(_bufContent&((1<<maxCount)-1))<<pos;
			_bufContent>>=maxCount;
			_bufLength-=maxCount;
			count-=maxCount;
			pos+=maxCount;
		}
		return ret;
	}

	size_t available() const noexcept { return _inputStream.available()*8U+_bufLength; }

private:
	T			&_inputStream;
	uint32_t		_bufContent{0};
	uint8_t			_bufLength{0};
};


template<typename T>
class MSBBitReader
{
public:
	MSBBitReader(T &inputStream) noexcept :
		_inputStream{inputStream}
	{
		// nothing needed
	}
	~MSBBitReader() noexcept=default;

	uint32_t readBits8(uint32_t count)
	{
		return readBitsGeneric(count,[&](){
			return std::make_pair(_inputStream.readByte(),uint8_t(8));
		});
	}

	uint32_t readBitsBE16(uint32_t count)
	{
		return readBitsGeneric(count,[&](){
			return std::make_pair(_inputStream.readBE16(),uint8_t(16));
		});
	}

	uint32_t readBitsBE32(uint32_t count)
	{
		return readBitsGeneric(count,[&](){
			return std::make_pair(_inputStream.readBE32(),uint8_t(32));
		});
	}

	uint32_t readBitsLE16(uint32_t count)
	{
		return readBitsGeneric(count,[&](){
			return std::make_pair(_inputStream.readLE16(),uint8_t(16));
		});
	}

	uint32_t getBufContent() const noexcept { return _bufContent; }
	uint8_t getBufLength() const noexcept { return _bufLength; }

	void reset(uint32_t bufContent=0,uint8_t bufLength=0) noexcept
	{
		_bufContent=bufContent;
		_bufLength=bufLength;
	}

	template<typename F>
	uint32_t readBitsGeneric(uint32_t count,F readWord)
	{
		uint32_t ret{0};
		if (count>32U)
			throw Decompressor::DecompressionError();
		while (count)
		{
			if (!_bufLength)
				std::tie(_bufContent,_bufLength)=readWord();
			uint8_t maxCount{std::min(uint8_t(count),_bufLength)};
			_bufLength-=maxCount;
			ret=(ret<<maxCount)|((_bufContent>>_bufLength)&((1<<maxCount)-1));
			count-=maxCount;
		}
		return ret;
	}

	size_t available() const noexcept { return _inputStream.available()*8U+_bufLength; }

private:
	T			&_inputStream;
	uint32_t		_bufContent{0};
	uint8_t			_bufLength{0};
};

}

#endif
