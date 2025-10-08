/* Copyright (C) Teemu Suutari */

#include "Buffer.hpp"
#include "OverflowCheck.hpp"


namespace ancient::internal
{

void Buffer::resize(size_t newSize)
{
	throw InvalidOperationError();
}

uint8_t &Buffer::operator[](size_t i)
{
	if (i>=size()) throw OutOfBoundsError();
	return data()[i];
}

const uint8_t &Buffer::operator[](size_t i) const
{
	if (i>=size()) throw OutOfBoundsError();
	return data()[i];
}

uint32_t Buffer::readBE32(size_t offset) const
{
	if (OverflowCheck::sum(offset,4U)>size()) throw OutOfBoundsError();
	const uint8_t *ptr=data()+offset;
	return (uint32_t(ptr[0])<<24)|(uint32_t(ptr[1])<<16)|(uint32_t(ptr[2])<<8)|uint32_t(ptr[3]);
}

uint16_t Buffer::readBE16(size_t offset) const
{
	if (OverflowCheck::sum(offset,2U)>size()) throw OutOfBoundsError();
	const uint8_t *ptr=data()+offset;
	return (uint16_t(ptr[0])<<8)|uint16_t(ptr[1]);
}

uint64_t Buffer::readLE64(size_t offset) const
{
	if (OverflowCheck::sum(offset,8U)>size()) throw OutOfBoundsError();
	const uint8_t *ptr=data()+offset;
	return (uint64_t(ptr[7])<<56)|(uint64_t(ptr[6])<<48)|(uint64_t(ptr[5])<<40)|(uint64_t(ptr[4])<<32)|
		(uint64_t(ptr[3])<<24)|(uint64_t(ptr[2])<<16)|(uint64_t(ptr[1])<<8)|uint64_t(ptr[0]);
}

uint32_t Buffer::readLE32(size_t offset) const
{
	if (OverflowCheck::sum(offset,4U)>size()) throw OutOfBoundsError();
	const uint8_t *ptr=data()+offset;
	return (uint32_t(ptr[3])<<24)|(uint32_t(ptr[2])<<16)|(uint32_t(ptr[1])<<8)|uint32_t(ptr[0]);
}

uint16_t Buffer::readLE16(size_t offset) const
{
	if (OverflowCheck::sum(offset,2U)>size()) throw OutOfBoundsError();
	const uint8_t *ptr=data()+offset;
	return (uint16_t(ptr[1])<<8)|uint16_t(ptr[0]);
}

uint8_t Buffer::read8(size_t offset) const
{
	if (offset>=size()) throw OutOfBoundsError();
	const uint8_t *ptr=reinterpret_cast<const uint8_t*>(data())+offset;
	return ptr[0];
}

}
