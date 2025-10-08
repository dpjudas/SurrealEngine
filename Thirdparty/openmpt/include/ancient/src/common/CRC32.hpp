/* Copyright (C) Teemu Suutari */

#ifndef CRC32_HPP
#define CRC32_HPP

#include <cstdint>

#include "Buffer.hpp"

namespace ancient::internal
{

// The most common CRC32

uint32_t CRC32(const Buffer &buffer,size_t offset,size_t len,uint32_t accumulator);

uint32_t CRC32Byte(uint8_t ch,uint32_t accumulator) noexcept;

// Same polynomial, but in reverse...

uint32_t CRC32Rev(const Buffer &buffer,size_t offset,size_t len,uint32_t accumulator);

uint32_t CRC32RevByte(uint8_t ch,uint32_t accumulator) noexcept;

}

#endif
