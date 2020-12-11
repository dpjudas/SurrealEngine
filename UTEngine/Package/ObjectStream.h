#pragma once

#include "Package.h"

enum class ObjectFlags : uint32_t;
class UClass;

class ObjectStream
{
public:
	ObjectStream(Package* package, std::unique_ptr<uint64_t[]> buf, size_t startoffset, size_t size, ObjectFlags flags, const std::string& name, UClass* base) : package(package), buffer(std::move(buf)), data(reinterpret_cast<const uint8_t*>(buffer.get())), startoffset(startoffset), size(size), flags(flags), name(name), base(base) { }

	void ReadBytes(void* d, uint32_t s)
	{
		if (pos + s > size)
			throw std::runtime_error("Unexpected end of file");
		memcpy(d, data + pos, s);
		pos += s;
	}

	void ThrowIfNotEnd()
	{
		if (pos != size)
			throw std::runtime_error("Unexpected bytes at end of object stream");
	}

	bool IsEmptyStream() const { return size == 0; }

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
		if (offset < startoffset)
			throw std::runtime_error("Seeking outside object");
		offset -= (uint32_t)startoffset;
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
		return (uint32_t)(startoffset + pos);
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
		if (GetVersion() >= 64)
		{
			int len = ReadIndex();
			std::vector<char> s;
			s.resize(len);
			ReadBytes(s.data(), (int)s.size());
			s.push_back(0);
			return s.data();
		}
		else
		{
			return ReadAsciiZ();
		}
	}

	std::string ReadAsciiZ()
	{
		std::string s;
		while (true)
		{
			char c = ReadInt8();
			if (c == 0) break;
			s.push_back(c);
		}
		return s;
	}

	std::wstring ReadUnicodeZ()
	{
		std::wstring s;
		while (true)
		{
			wchar_t c = ReadUInt16();
			if (c == 0) break;
			s.push_back(c);
		}
		return s;
	}

	std::string ReadName()
	{
		return package->GetName(ReadIndex());
	}

	template<typename T>
	T* ReadObject()
	{
		return static_cast<T*>(package->GetUObject(ReadIndex(), [](UObject* obj) { UObject::Cast<T>(obj); }));
	}

	int GetVersion() const
	{
		return package->GetVersion();
	}

	ObjectFlags GetFlags() const
	{
		return flags;
	}

	UObject* GetUObject(int objref)
	{
		return package->GetUObject(objref);
	}

	const std::string& GetObjectName() const
	{
		return name;
	}

	UClass* GetObjectBase()
	{
		return base;
	}

	Package* GetPackage()
	{
		return package;
	}

private:
	Package* package = nullptr;
	std::unique_ptr<uint64_t[]> buffer;
	const uint8_t* data = nullptr;
	size_t startoffset = 0;
	size_t size = 0;
	size_t pos = 0;
	ObjectFlags flags = {};
	std::string name;
	UClass* base = nullptr;
};
