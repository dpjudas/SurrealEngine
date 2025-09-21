#pragma once

class File;
class Package;
class PackageWriter;
class UObject;
class NameString;

class PackageStream
{
public:
	PackageStream(Package* package, std::shared_ptr<File> file);

	void ReadBytes(void* d, uint32_t s);

	int8_t ReadInt8();
	int16_t ReadInt16();
	int32_t ReadInt32();
	int64_t ReadInt64();
	float ReadFloat();
	uint8_t ReadUInt8();
	uint16_t ReadUInt16();
	uint32_t ReadUInt32();
	uint64_t ReadUInt64();
	int32_t ReadIndex();
	std::string ReadString();

	void Seek(uint32_t offset);
	void Skip(uint32_t bytes);
	uint32_t Tell();

	Package* GetPackage() const;
	int GetVersion() const;

private:
	Package* package;
	std::shared_ptr<File> file;
};

class PackageStreamWriter
{
public:
	PackageStreamWriter(PackageWriter* package, std::shared_ptr<File> file);

	void WriteBytes(const void* d, uint32_t s);

	void WriteInt8(int8_t v);
	void WriteInt16(int16_t v);
	void WriteInt32(int32_t v);
	void WriteInt64(int64_t v);
	void WriteFloat(float v);
	void WriteUInt8(uint8_t v);
	void WriteUInt16(uint16_t v);
	void WriteUInt32(uint32_t v);
	void WriteUInt64(uint64_t v);
	void WriteIndex(int32_t v);
	void WriteString(const std::string& v);
	void WriteAsciiZ(const std::string& v);
	void WriteUnicodeZ(const std::wstring& v);
	void WriteName(NameString name);
	void WriteObject(UObject* obj);

	void BeginSkipOffset();
	void EndSkipOffset();

	void Seek(uint32_t offset);
	uint32_t Tell();

	PackageWriter* GetPackage() const;
	int GetVersion() const;

private:
	PackageWriter* package;
	std::shared_ptr<File> file;
	uint32_t skipOffsetLocation = 0;
};
