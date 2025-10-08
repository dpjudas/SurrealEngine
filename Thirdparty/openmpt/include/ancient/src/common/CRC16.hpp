/* Copyright (C) Teemu Suutari */

#ifndef CRC16_HPP
#define CRC16_HPP

#include <cstdint>

#include "Buffer.hpp"

namespace ancient::internal
{

// The most common CRC16

uint16_t CRC16(const Buffer &buffer,size_t offset,size_t len,uint16_t accumulator);

uint16_t CRC16Byte(uint8_t ch,uint16_t accumulator) noexcept;

}

#endif
