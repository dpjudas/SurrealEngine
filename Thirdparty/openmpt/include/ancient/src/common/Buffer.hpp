/* Copyright (C) Teemu Suutari */

#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <cstddef>
#include <cstdint>

#include <exception>

namespace ancient::internal
{

class Buffer
{
protected:
	Buffer() noexcept=default;

public:
	class Error : public std::exception
	{
	public:
		Error() noexcept=default;
		virtual ~Error() noexcept=default;
	};

	class OutOfBoundsError : public Error
	{
	public:
		OutOfBoundsError() noexcept=default;
		virtual ~OutOfBoundsError() noexcept=default;
	};

	class InvalidOperationError : public Error
	{
	public:
		InvalidOperationError() noexcept=default;
		virtual ~InvalidOperationError() noexcept=default;
	};

	Buffer(const Buffer&)=delete;
	Buffer& operator=(const Buffer&)=delete;

	virtual ~Buffer() noexcept=default;

	virtual const uint8_t *data() const noexcept=0;
	virtual uint8_t *data()=0;
	virtual size_t size() const noexcept=0;

	template<typename T>
	const T *cast() const noexcept
	{
		return reinterpret_cast<const T*>(data());
	}

	template<typename T>
	T *cast()
	{
		return reinterpret_cast<T*>(data());
	}

	virtual bool isResizable() const noexcept=0;
	virtual void resize(size_t newSize);

	uint8_t &operator[](size_t i);
	const uint8_t &operator[](size_t i) const;

	uint32_t readBE32(size_t offset) const;
	uint16_t readBE16(size_t offset) const;

	uint64_t readLE64(size_t offset) const;
	uint32_t readLE32(size_t offset) const;
	uint16_t readLE16(size_t offset) const;

	uint8_t read8(size_t offset) const;
};

}

#endif
