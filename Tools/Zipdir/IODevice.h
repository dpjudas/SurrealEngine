
#pragma once

#include <cstdint>
#include <stdexcept>

class IODevice
{
public:
	virtual ~IODevice() = default;

	virtual int64_t size() const = 0;
	int64_t position() const { return const_cast<IODevice*>(this)->seek_from_current(0); }

	virtual int64_t seek(int64_t position) = 0;
	virtual int64_t seek_from_current(int64_t offset) = 0;
	virtual int64_t seek_from_end(int64_t offset) = 0;

	virtual size_t try_read(void *data, size_t size) = 0;
	void read(void *data, size_t size) { size_t bytes = try_read(data, size); if (bytes != size) throw std::runtime_error("Could not read all bytes"); }
	virtual void write(const void *data, size_t size) = 0;

	virtual void close() { }

	uint8_t readUint8() { return readType<uint8_t>(); }
	int8_t readInt8() { return readType<int8_t>(); }
	uint16_t readUint16() { return readType<uint16_t>(); }
	int16_t readInt16() { return readType<int16_t>(); }
	uint32_t readUint32() { return readType<uint32_t>(); }
	int32_t readInt32() { return readType<int32_t>(); }
	float readFloat() { return readType<float>(); }
	double readDouble() { return readType<double>(); }

	void writeUint8(uint8_t v) { writeType(v); }
	void writeInt8(int8_t v) { writeType(v); }
	void writeUint16(uint16_t v) { writeType(v); }
	void writeInt16(int16_t v) { writeType(v); }
	void writeUint32(uint32_t v) { writeType(v); }
	void writeInt32(int32_t v) { writeType(v); }
	void writeUint64(uint64_t v) { writeType(v); }
	void writeInt64(int64_t v) { writeType(v); }
	void writeFloat(float v) { writeType(v); }
	void writeDouble(double v) { writeType(v); }

	template<typename T> T readType() { T v; read(&v, sizeof(T)); return v; }
	template<typename T> void writeType(T v) { write(&v, sizeof(T)); }
};
