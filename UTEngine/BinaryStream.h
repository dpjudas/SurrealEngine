#pragma once

class BinaryStream
{
public:
	BinaryStream(const void* data, size_t size) : data(static_cast<const uint8_t*>(data)), size(size) { }

	void ReadBytes(void* d, uint32_t s)
	{
		if (pos + s > size)
			throw std::runtime_error("Unexpected end of file");
		memcpy(d, data + pos, s);
		pos += s;
	}

	int8_t ReadInt8() { int8_t t; ReadBytes(&t, 1); return t; }
	int16_t ReadInt16() { int16_t t; ReadBytes(&t, 2); return t; }
	int32_t ReadInt32() { int32_t t; ReadBytes(&t, 4); return t; }
	int64_t ReadInt64() { int64_t t; ReadBytes(&t, 8); return t; }
	float ReadFloat() { float t; ReadBytes(&t, 4); return t; }

	uint8_t ReadUInt8() { return ReadInt8(); }
	uint16_t ReadUInt16() { return ReadInt16(); }
	uint32_t ReadUInt32() { return ReadInt32(); }
	uint64_t ReadUInt64() { return ReadInt64(); }

	void Seek(uint32_t offset)
	{
		if (offset > size)
			throw std::runtime_error("Unexpected end of file");
		pos = offset;
	}

	void Skip(uint32_t bytes)
	{
		if (pos + bytes > size)
			throw std::runtime_error("Unexpected end of file");
		pos += bytes;
	}

	uint32_t Tell()
	{
		return (uint32_t)pos;
	}

	int32_t ReadIndex()
	{
		uint8_t value = ReadInt8();
		bool signbit = value & (1 << 7);
		bool nextbyte = value & (1 << 6);
		int32_t index = value & 0x3f;
		if (nextbyte)
		{
			int shift = 6;
			do
			{
				value = ReadInt8();
				index |= static_cast<int32_t>(value & 0x7f) << shift;
				shift += 7;
			} while ((value & (1 << 7)) && shift < 32);
		}
		if (signbit)
			index = -index;
		return index;
	}

	std::string ReadString()
	{
		int len = ReadIndex();
		std::vector<char> s;
		s.resize(len);
		ReadBytes(s.data(), (int)s.size());
		s.push_back(0);
		return s.data();
	}

private:
	const uint8_t* data = nullptr;
	size_t size = 0;
	size_t pos = 0;
};
