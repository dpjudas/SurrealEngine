#pragma once

#include <memory>
#include <cstdint>

class DataBuffer
{
public:
	static std::shared_ptr<DataBuffer> create(size_t size);
	static std::shared_ptr<DataBuffer> create(const void* data, size_t size);

	virtual char* data() = 0;
	virtual const char* data() const = 0;
	virtual size_t size() const = 0;
	virtual size_t capacity() const = 0;
	virtual void setSize(size_t size) = 0;
	virtual void setCapacity(size_t capacity) = 0;
	virtual std::shared_ptr<DataBuffer> copy(size_t pos, size_t size) = 0;

	template<typename Type> Type* data() { return reinterpret_cast<Type*>(data()); }
	template<typename Type> const Type* data() const { return reinterpret_cast<const Type*>(data()); }
};
