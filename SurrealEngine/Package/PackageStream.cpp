
#include "Precomp.h"
#include "PackageStream.h"
#include "Package.h"
#include "Utils/File.h"

PackageStream::PackageStream(Package* package, std::shared_ptr<File> file) : package(package), file(file)
{
}

void PackageStream::ReadBytes(void* d, uint32_t s)
{
	file->read(d, s);
}

int8_t PackageStream::ReadInt8()
{
	int8_t t;
	ReadBytes(&t, 1);
	return t;
}

int16_t PackageStream::ReadInt16()
{
	int16_t t;
	ReadBytes(&t, 2);
	return t;
}

int32_t PackageStream::ReadInt32()
{
	int32_t t;
	ReadBytes(&t, 4);
	return t;
}

int64_t PackageStream::ReadInt64()
{
	int64_t t;
	ReadBytes(&t, 8);
	return t;
}

float PackageStream::ReadFloat()
{
	float t;
	ReadBytes(&t, 4);
	return t;
}

uint8_t PackageStream::ReadUInt8()
{
	return ReadInt8();
}

uint16_t PackageStream::ReadUInt16()
{
	return ReadInt16();
}

uint32_t PackageStream::ReadUInt32()
{
	return ReadInt32();
}

uint64_t PackageStream::ReadUInt64()
{
	return ReadInt64();
}

void PackageStream::Seek(uint32_t offset)
{
	file->seek(offset);
}

void PackageStream::Skip(uint32_t bytes)
{
	file->seek(file->tell());
}

uint32_t PackageStream::Tell()
{
	return (uint32_t)file->tell();
}

int32_t PackageStream::ReadIndex()
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

std::string PackageStream::ReadString()
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
		std::string s;
		while (true)
		{
			char c = ReadInt8();
			if (c == 0) break;
			s.push_back(c);
		}
		return s;
	}
}

Package* PackageStream::GetPackage() const
{
	return package;
}

int PackageStream::GetVersion() const
{
	return package->GetVersion();
}
