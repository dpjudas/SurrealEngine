#include "MemoryStreamWriter.h"

#define MAX_STREAM_SIZE UINT32_MAX

MemoryStreamWriter::MemoryStreamWriter(size_t len)
{
	if (!len)
	{
		Exception::Throw("Size is 0");
	}

	if (len > MAX_STREAM_SIZE)
	{
		Exception::Throw("size " + std::to_string(len) + " too big");
	}

	data.reserve(len);
	pos = 0;
}

MemoryStreamWriter::MemoryStreamWriter(MemoryStreamWriter&& msw) noexcept
{
	data = std::move(msw.data);
	pos = msw.pos;
}

MemoryStreamWriter::~MemoryStreamWriter()
{
	data.clear();
}

void MemoryStreamWriter::Reserve(size_t capacity)
{
	data.reserve(capacity);
}

void MemoryStreamWriter::Write(const void* buf, size_t size)
{
	const uint8_t* buf8 = reinterpret_cast<const uint8_t*>(buf);
	for (size_t i = 0; i < size; i++)
	{
		if (pos == data.size())
		{
			data.push_back(buf8[i]);
		}
		else
		{
			data[pos] = buf8[i];
		}
		pos++;
	}
}

void MemoryStreamWriter::Seek(size_t newpos, int base)
{
	if (base == SEEK_CUR)
	{
		pos += newpos;
	}
	else if (base == SEEK_END)
	{
		pos = data.size() - newpos - 1;
	}
	else
	{
		pos = newpos;
	}
}

size_t MemoryStreamWriter::Tell() const
{
	return pos;
}

const uint8_t* MemoryStreamWriter::Data() const
{
	return data.data();
}

size_t MemoryStreamWriter::Size() const
{
	return data.size();
}