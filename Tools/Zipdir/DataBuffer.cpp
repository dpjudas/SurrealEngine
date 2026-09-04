
#include "DataBuffer.h"
#include <cstring>
#include <vector>

class DataBufferImpl : public DataBuffer
{
public:
	DataBufferImpl(size_t init_size) { setSize(init_size); }
	DataBufferImpl(const void* init_data, size_t init_size) { setSize(init_size); memcpy(data(), init_data, size()); }

	char* data() override { return buffer.data(); }
	const char* data() const override { return buffer.data(); }
	size_t size() const override { return buffer.size(); }
	size_t capacity() const override { return buffer.capacity(); }
	void setSize(size_t size) override { buffer.resize(size); }
	void setCapacity(size_t capacity) override { buffer.reserve(capacity); }

	std::shared_ptr<DataBuffer> copy(size_t pos, size_t size) override { return DataBuffer::create(data() + pos, size); }

private:
	std::vector<char> buffer;
};

std::shared_ptr<DataBuffer> DataBuffer::create(size_t size)
{
	return std::make_shared<DataBufferImpl>(size);
}

std::shared_ptr<DataBuffer> DataBuffer::create(const void* data, size_t size)
{
	return std::make_shared<DataBufferImpl>(data, size);
}
