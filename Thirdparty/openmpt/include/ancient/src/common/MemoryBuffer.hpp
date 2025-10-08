/* Copyright (C) Teemu Suutari */

#ifndef MEMORYBUFFER_HPP
#define MEMORYBUFFER_HPP

#include <memory>

#include "Buffer.hpp"

namespace ancient::internal
{

class MemoryBuffer : public Buffer
{
public:
	MemoryBuffer(size_t size);
	MemoryBuffer(const Buffer &src,size_t offset,size_t size);
	~MemoryBuffer() noexcept;

	const uint8_t *data() const noexcept final;
	uint8_t *data() final;
	size_t size() const noexcept final;

	bool isResizable() const noexcept final;
	void resize(size_t newSize) final;

private:
	uint8_t*			_data;
	size_t				_size;
};

}

#endif
