
#include "Precomp.h"
#include "PackageStream.h"
#include "Package.h"
#include "PackageWriter.h"
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
	file->seek(file->tell() + bytes);
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
		Array<char> s;
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

/////////////////////////////////////////////////////////////////////////////

PackageStreamWriter::PackageStreamWriter(PackageWriter* package, std::shared_ptr<File> file) : package(package), file(file)
{
}

void PackageStreamWriter::WriteBytes(const void* d, uint32_t s)
{
	file->write(d, s);
}

void PackageStreamWriter::WriteInt8(int8_t v)
{
	WriteBytes(&v, 1);
}

void PackageStreamWriter::WriteInt16(int16_t v)
{
	WriteBytes(&v, 2);
}

void PackageStreamWriter::WriteInt32(int32_t v)
{
	WriteBytes(&v, 4);
}

void PackageStreamWriter::WriteInt64(int64_t v)
{
	WriteBytes(&v, 8);
}

void PackageStreamWriter::WriteFloat(float v)
{
	WriteBytes(&v, 4);
}

void PackageStreamWriter::WriteUInt8(uint8_t v)
{
	WriteBytes(&v, 1);
}

void PackageStreamWriter::WriteUInt16(uint16_t v)
{
	WriteBytes(&v, 2);
}

void PackageStreamWriter::WriteUInt32(uint32_t v)
{
	WriteBytes(&v, 4);
}

void PackageStreamWriter::WriteUInt64(uint64_t v)
{
	WriteBytes(&v, 8);
}

void PackageStreamWriter::WriteIndex(int32_t v)
{
}

void PackageStreamWriter::WriteString(const std::string& v)
{
}

void PackageStreamWriter::WriteAsciiZ(const std::string& v)
{
	WriteBytes(v.c_str(), (uint32_t)(v.size() + 1));
}

void PackageStreamWriter::WriteUnicodeZ(const std::wstring& v)
{
	WriteBytes(v.c_str(), (uint32_t)((v.size() + 1) * 2));
}

void PackageStreamWriter::WriteName(NameString name)
{
	WriteIndex(package->GetNameIndex(name));
}

void PackageStreamWriter::WriteObject(UObject* obj)
{
	WriteIndex(package->GetObjectReference(obj));
}

void PackageStreamWriter::Seek(uint32_t offset)
{
	file->seek(offset);
}

uint32_t PackageStreamWriter::Tell()
{
	return (uint32_t)file->tell();
}

PackageWriter* PackageStreamWriter::GetPackage() const
{
	return package;
}

int PackageStreamWriter::GetVersion() const
{
	return package->GetVersion();
}
