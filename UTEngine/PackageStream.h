#pragma once

class File;
class Package;

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
