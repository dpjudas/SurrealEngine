/* Copyright (C) Teemu Suutari */

#ifndef WRAPPEDVECTORBUFFER_HPP
#define WRAPPEDVECTORBUFFER_HPP

#include <vector>

#include <cstddef>
#include <cstdint>

#include "Buffer.hpp"

namespace ancient::internal
{

class WrappedVectorBuffer : public Buffer
{
public:
	WrappedVectorBuffer(std::vector<uint8_t> &refdata) noexcept;
	~WrappedVectorBuffer() noexcept=default;

	const uint8_t *data() const noexcept final;
	uint8_t *data() final;
	size_t size() const noexcept final;

	bool isResizable() const noexcept final;
	void resize(size_t newSize) final;

private:
	std::vector<uint8_t> & _refdata;
};

}

#endif
