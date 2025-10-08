/* Copyright (C) Teemu Suutari */

#ifndef STATICBUFFER_HPP
#define STATICBUFFER_HPP

#include <cstddef>
#include <cstdint>

#include "Buffer.hpp"

namespace ancient::internal
{

template<size_t N>
class StaticBuffer : public Buffer
{
public:
	StaticBuffer(const StaticBuffer&)=delete;
	StaticBuffer& operator=(const StaticBuffer&)=delete;

	StaticBuffer() noexcept=default;
	~StaticBuffer() noexcept=default;

	const uint8_t *data() const noexcept final
	{
		return _data;
	}

	uint8_t *data() final
	{
		return _data;
	}

	size_t size() const noexcept final
	{
		return N;
	}

	bool isResizable() const noexcept final
	{
		return false;
	}

private:
	uint8_t 	_data[N];
};


class ConstStaticBuffer : public Buffer
{
public:
	ConstStaticBuffer(const ConstStaticBuffer&)=delete;
	ConstStaticBuffer& operator=(const ConstStaticBuffer&)=delete;

	ConstStaticBuffer(const uint8_t *data,size_t length) noexcept;
	~ConstStaticBuffer() noexcept=default;

	const uint8_t *data() const noexcept final;
	uint8_t *data() final;

	size_t size() const noexcept final;
	bool isResizable() const noexcept final;

private:
	const uint8_t 	*_data;
	size_t		_length;
};

}

#endif
