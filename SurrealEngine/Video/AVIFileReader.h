#pragma once

#include "Utils/Array.h"
#include <string>
#include <cstdint>
#include <stdexcept>
#include <cstring>

class AVIFileReader
{
public:
	AVIFileReader(Array<uint8_t> buffer) : buffer(std::move(buffer)) { }

	void PushChunk(std::string tagname)
	{
		std::string tag = PushChunk();
		if (tag != tagname)
			throw std::runtime_error("Expected " + tagname + ", got " + tag);
	}

	std::string PushChunk()
	{
		std::string tag = ReadTag();
		uint32_t chunksize = ReadUint32();
		chunks.push_back({ Tell(), chunksize });
		return tag;
	}

	void SkipJunk()
	{
		while (!IsEndOfChunk())
		{
			size_t oldpos = pos;
			if (ReadTag() != "JUNK")
			{
				pos = oldpos;
				break;
			}
			uint32_t chunksize = ReadUint32();
			chunks.push_back({ Tell(), chunksize });
			PopChunk(false);
		}
	}

	uint32_t GetChunkSize() { return chunks.back().second; }

	bool IsEndOfChunk()
	{
		return Tell() - chunks.back().first == chunks.back().second;
	}

	void PopChunk(bool mustReadAll = false)
	{
		if (mustReadAll && !IsEndOfChunk())
			throw std::runtime_error("Unexpected end of chunk");

		// Chunks always 16 bit align
		Seek(chunks.back().first + chunks.back().second + (chunks.back().second & 1));
		chunks.pop_back();
	}

	size_t Read(void* dest, size_t size)
	{
		size_t available = buffer.size() - pos;
		size_t count = std::min(size, available);
		memcpy(dest, buffer.data() + pos, count);
		pos += count;
		return count;
	}

	uint8_t ReadUint8()
	{
		return pos < buffer.size() ? buffer[pos++] : 0;
	}

	void ReadTag(std::string tagname)
	{
		std::string tag = ReadTag();
		if (tag != tagname)
			throw std::runtime_error("Expected " + tagname + ", got " + tag);
	}

	std::string ReadString(uint32_t count)
	{
		std::string s;
		for (uint32_t i = 0; i < count; i++)
			s.push_back(ReadUint8());
		for (size_t j = 0; j < s.size(); j++)
		{
			if (s[j] == 0)
			{
				s.resize(j);
				break;
			}
		}
		return s;
	}

	Array<uint8_t> ReadArray8(uint32_t count)
	{
		Array<uint8_t> buffer(count);
		Read(buffer.data(), buffer.size());
		return buffer;
	}

	std::string ReadTag()
	{
		char tag[4];
		for (int i = 0; i < 4; i++)
			tag[i] = ReadUint8();
		return std::string(tag, 4);
	}

	uint32_t ReadUint32BE()
	{
		uint32_t v0 = ReadUint8();
		uint32_t v1 = ReadUint8();
		uint32_t v2 = ReadUint8();
		uint32_t v3 = ReadUint8();
		return (v0 << 24) | (v1 << 16) | (v2 << 8) | v3;
	}

	int16_t ReadInt16()
	{
		return ReadUint16();
	}

	uint16_t ReadUint16()
	{
		uint32_t v0 = ReadUint8();
		uint32_t v1 = ReadUint8();
		return (v1 << 8) | v0;
	}

	uint32_t ReadUint24()
	{
		uint32_t v0 = ReadUint8();
		uint32_t v1 = ReadUint8();
		uint32_t v2 = ReadUint8();
		return (v2 << 16) | (v1 << 8) | v0;
	}

	int32_t ReadInt24()
	{
		return ((int32_t)(ReadUint24() << 8)) >> 8;
	}

	uint32_t ReadUint32()
	{
		uint32_t v0 = ReadUint8();
		uint32_t v1 = ReadUint8();
		uint32_t v2 = ReadUint8();
		uint32_t v3 = ReadUint8();
		return (v3 << 24) | (v2 << 16) | (v1 << 8) | v0;
	}

	int32_t ReadInt32()
	{
		return ReadUint32();
	}

	const uint8_t* CurrentPosData() const { return &buffer[pos]; }

	size_t Tell() { return pos; }
	size_t Size() { return buffer.size(); }
	void Seek(size_t newpos) { pos = newpos; }

private:
	Array<uint8_t> buffer;
	size_t pos = 0;
	Array<std::pair<size_t, uint32_t>> chunks;
};
