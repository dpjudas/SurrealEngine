/* Copyright (C) Teemu Suutari */

#include "WrappedVectorBuffer.hpp"


namespace ancient::internal
{

WrappedVectorBuffer::WrappedVectorBuffer(std::vector<uint8_t> &refdata) noexcept :
	_refdata{refdata}
{
	// nothing needed
}

const uint8_t *WrappedVectorBuffer::data() const noexcept
{
	return _refdata.data();
}

uint8_t *WrappedVectorBuffer::data()
{
	return _refdata.data();
}

size_t WrappedVectorBuffer::size() const noexcept
{
	return _refdata.size();
}

bool WrappedVectorBuffer::isResizable() const noexcept
{
	return true;
}

void WrappedVectorBuffer::resize(size_t newSize)
{
	_refdata.resize(newSize);
}

}
