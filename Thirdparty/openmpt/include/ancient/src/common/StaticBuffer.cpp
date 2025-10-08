/* Copyright (C) Teemu Suutari */

#include "StaticBuffer.hpp"


namespace ancient::internal
{

ConstStaticBuffer::ConstStaticBuffer(const uint8_t *data,size_t length) noexcept :
	_data{data},
	_length{length}
{
	// nothing needed
}

const uint8_t *ConstStaticBuffer::data() const noexcept
{
	return _data;
}

uint8_t *ConstStaticBuffer::data()
{
	throw InvalidOperationError();
}

size_t ConstStaticBuffer::size() const noexcept
{
	return _length;
}

bool ConstStaticBuffer::isResizable() const noexcept
{
	return false;
}

}
