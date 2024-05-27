#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

class MemoryStreamWriter
{
public:
	MemoryStreamWriter(size_t len = 64);
	MemoryStreamWriter(MemoryStreamWriter&& msw) noexcept;
	~MemoryStreamWriter();

	void Reserve(size_t capacity);
	void Write(const void* buf, size_t size);
	void Seek(size_t pos, int base);
	size_t Tell() const;

	const uint8_t* Data() const;
	size_t Size() const;

	MemoryStreamWriter& operator<<(uint8_t u)
	{
		Write(&u, sizeof(u));
		return *this;
	}
	MemoryStreamWriter& operator<<(uint16_t u)
	{
		u = BSWAP16(u);
		Write(&u, sizeof(u));
		return *this;
	}
	MemoryStreamWriter& operator<<(uint32_t u)
	{
		u = BSWAP32(u);
		Write(&u, sizeof(u));
		return *this;
	}
	MemoryStreamWriter& operator<<(uint64_t u)
	{
		u = BSWAP64(u);
		Write(&u, sizeof(u));
		return *this;
	}
	MemoryStreamWriter& operator<<(int8_t i)
	{
		Write(&i, sizeof(i));
		return *this;
	}
	MemoryStreamWriter& operator<<(int16_t i)
	{
		i = BSWAP16(i);
		Write(&i, sizeof(i));
		return *this;
	}
	MemoryStreamWriter& operator<<(int32_t i)
	{
		i = BSWAP32(i);
		Write(&i, sizeof(i));
		return *this;
	}
	MemoryStreamWriter& operator<<(int64_t i)
	{
		i = BSWAP64(i);
		Write(&i, sizeof(i));
		return *this;
	}
	MemoryStreamWriter& operator<<(float f)
	{
		f = BSWAP32(f);
		Write(&f, sizeof(f));
		return *this;
	}
	MemoryStreamWriter& operator<<(double d)
	{
		d = BSWAP64(d);
		Write(&d, sizeof(d));
		return *this;
	}
	MemoryStreamWriter& operator<<(const char* s)
	{
		Write(s, strlen(s));
		return *this;
	}
	MemoryStreamWriter& operator<<(std::string s)
	{
		Write(s.data(), s.size());
		return *this;
	}

private:
	std::vector<uint8_t> data;
	size_t pos;
};