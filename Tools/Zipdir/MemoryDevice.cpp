
#include "MemoryDevice.h"
#include "DataBuffer.h"
#include <stdexcept>
#include <algorithm>
#include <cstring>

class MemoryDeviceImpl : public MemoryDevice
{
public:
	MemoryDeviceImpl() : _buffer(DataBuffer::create(0)) { }
	MemoryDeviceImpl(const std::shared_ptr<DataBuffer> &buffer) : _buffer(buffer) { }

	const std::shared_ptr<DataBuffer> &buffer() const override { return _buffer; }
	void setBuffer(const std::shared_ptr<DataBuffer> &buffer) override { _buffer = buffer; _pos = 0; }

	int64_t size() const override { return _buffer->size(); }

	int64_t seek(int64_t pos) override { if (pos >= 0 && pos < size()) _pos = pos; return _pos; }
	int64_t seek_from_current(int64_t offset) override { return seek(_pos + offset); }
	int64_t seek_from_end(int64_t offset) override { return seek(size() + offset); }

	size_t try_read(void *data, size_t size) override
	{
		if (size < 0)
			throw std::runtime_error("Read failed");

		size = std::min(size, (size_t)(_buffer->size() - _pos));
		memcpy(data, _buffer->data() + _pos, size);
		_pos += size;
		return size;
	}

	void write(const void *data, size_t size) override
	{
		if (size < 0)
			throw std::runtime_error("Write failed");

		if (_pos + size > 0x7ffffff)
			throw std::runtime_error("Memory buffer max size exceeeded");

		if ((size_t)(_pos + size) > _buffer->capacity())
			_buffer->setCapacity(std::max(_pos + size, static_cast<size_t>(_buffer->capacity()) * 2));

		if ((size_t)(_pos + size) > _buffer->size())
			_buffer->setSize(_pos + size);

		memcpy(_buffer->data() + _pos, data, size);
		_pos += size;
	}

	MemoryDeviceImpl(const MemoryDeviceImpl &) = delete;
	MemoryDeviceImpl &operator=(const MemoryDeviceImpl &) = delete;

	std::shared_ptr<DataBuffer> _buffer;
	size_t _pos = 0;
};

std::shared_ptr<MemoryDevice> MemoryDevice::create()
{
	return std::make_shared<MemoryDeviceImpl>();
}

std::shared_ptr<MemoryDevice> MemoryDevice::open(const std::shared_ptr<DataBuffer> &buffer)
{
	return std::make_shared<MemoryDeviceImpl>(buffer);
}
