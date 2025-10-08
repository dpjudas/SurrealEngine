/* Copyright (C) Teemu Suutari */

#include <cstring>

#include <algorithm>

#include "OutputStream.hpp"
// for exceptions
#include "Decompressor.hpp"
#include "common/Common.hpp"
#include "common/OverflowCheck.hpp"


namespace ancient::internal
{

ForwardOutputStreamBase::ForwardOutputStreamBase(Buffer &buffer,size_t startOffset) :
	_buffer{buffer},
	_startOffset{startOffset},
	_currentOffset{startOffset}
{
	// should call ensureSize but can't
}

void ForwardOutputStreamBase::writeByte(uint8_t value)
{
	ensureSize(_currentOffset+1);
	_buffer[_currentOffset++]=value;
}

uint8_t ForwardOutputStreamBase::copy(size_t distance,size_t count)
{
	ensureSize(OverflowCheck::sum(_currentOffset,count));
	if (!distance || OverflowCheck::sum(_startOffset,distance)>_currentOffset)
		throw Decompressor::DecompressionError();
	uint8_t ret{0};
	for (size_t i=0;i<count;i++,_currentOffset++)
		ret=_buffer[_currentOffset]=_buffer[_currentOffset-distance];
	return ret;
}

uint8_t ForwardOutputStreamBase::copy(size_t distance,size_t count,const Buffer &prevBuffer)
{
	ensureSize(OverflowCheck::sum(_currentOffset,count));
	if (!distance)
		throw Decompressor::DecompressionError();
	size_t prevCount{0};
	uint8_t ret{0};
	if (OverflowCheck::sum(_startOffset,distance)>_currentOffset)
	{
		size_t prevSize{prevBuffer.size()};
		if (_startOffset+distance>_currentOffset+prevSize)
			throw Decompressor::DecompressionError(); 
		size_t prevDist{_startOffset+distance-_currentOffset};
		prevCount=std::min(count,prevDist);
		const uint8_t *prev{&prevBuffer[prevSize-prevDist]};
		for (size_t i=0;i<prevCount;i++,_currentOffset++)
			ret=_buffer[_currentOffset]=prev[i];
	}
	for (size_t i=prevCount;i<count;i++,_currentOffset++)
		ret=_buffer[_currentOffset]=_buffer[_currentOffset-distance];
	return ret;
}

uint8_t ForwardOutputStreamBase::copy(size_t distance,size_t count,uint8_t defaultChar)
{
	ensureSize(OverflowCheck::sum(_currentOffset,count));
	if (!distance)
		throw Decompressor::DecompressionError();
	size_t prevCount{0};
	uint8_t ret{0};
	if (OverflowCheck::sum(_startOffset,distance)>_currentOffset)
	{
		prevCount=std::min(count,_startOffset+distance-_currentOffset);
		for (size_t i=0;i<prevCount;i++,_currentOffset++)
			ret=_buffer[_currentOffset]=defaultChar;
	}
	for (size_t i=prevCount;i<count;i++,_currentOffset++)
		ret=_buffer[_currentOffset]=_buffer[_currentOffset-distance];
	return ret;
}

const uint8_t *ForwardOutputStreamBase::history(size_t distance) const
{
	if (OverflowCheck::sum(distance,_startOffset)>_currentOffset)
		throw Decompressor::DecompressionError();
	return &_buffer[_currentOffset-distance];
}

uint8_t *ForwardOutputStreamBase::history(size_t distance)
{
	if (OverflowCheck::sum(distance,_startOffset)>_currentOffset)
		throw Decompressor::DecompressionError();
	return &_buffer[_currentOffset-distance];
}

void ForwardOutputStreamBase::produce(const Buffer &src)
{
	ensureSize(OverflowCheck::sum(_currentOffset,src.size()));
	std::memcpy(&_buffer[_currentOffset],src.data(),src.size());
	_currentOffset+=src.size();
}

// ---

ForwardOutputStream::ForwardOutputStream(Buffer &buffer,size_t startOffset,size_t endOffset) :
	ForwardOutputStreamBase{buffer,startOffset},
	_endOffset{endOffset}
{
	if (_startOffset>_endOffset || _endOffset>_buffer.size())
		throw Decompressor::DecompressionError();
}

void ForwardOutputStream::reset(size_t startOffset,size_t endOffset)
{
	_currentOffset=_startOffset=startOffset;
	_endOffset=endOffset;
	if (_startOffset>_endOffset || _endOffset>_buffer.size())
		throw Decompressor::DecompressionError();
}

void ForwardOutputStream::ensureSize(size_t offset)
{
	if (offset>_endOffset)
		throw Decompressor::DecompressionError();
}

// ---

AutoExpandingForwardOutputStream::AutoExpandingForwardOutputStream(Buffer &buffer) :
	ForwardOutputStreamBase{buffer,0}
{
	// nothing needed
}

AutoExpandingForwardOutputStream::~AutoExpandingForwardOutputStream() noexcept
{
	// trim
	if (_hasExpanded && _currentOffset!=_buffer.size())
		_buffer.resize(_currentOffset);
}

void AutoExpandingForwardOutputStream::ensureSize(size_t offset)
{
	if (offset>Decompressor::getMaxRawSize())
		throw Decompressor::DecompressionError();
	if (offset>_buffer.size())
	{
		_buffer.resize(offset+_advance);
		_hasExpanded=true;
	}
}

// ---

BackwardOutputStream::BackwardOutputStream(Buffer &buffer,size_t startOffset,size_t endOffset) :
	_buffer{buffer},
	_startOffset{startOffset},
	_currentOffset{endOffset},
	_endOffset{endOffset}
{
	if (_startOffset>_endOffset || _currentOffset>buffer.size() || _endOffset>buffer.size())
		throw Decompressor::DecompressionError();
}

void BackwardOutputStream::writeByte(uint8_t value)
{
	if (_currentOffset<=_startOffset)
		throw Decompressor::DecompressionError();
	_buffer[--_currentOffset]=value;
}

uint8_t BackwardOutputStream::copy(size_t distance,size_t count)
{
	if (!distance || OverflowCheck::sum(_startOffset,count)>_currentOffset || OverflowCheck::sum(_currentOffset,distance)>_endOffset)
		throw Decompressor::DecompressionError();
	uint8_t ret{0};
	for (size_t i=0;i<count;i++,--_currentOffset)
		ret=_buffer[_currentOffset-1]=_buffer[_currentOffset+distance-1];
	return ret;
}

}
