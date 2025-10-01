#pragma once

#include <memory>
#include <string>
#include <filesystem>

enum class SeekPoint
{
	begin,
	current,
	end
};

class File
{
public:
	static std::shared_ptr<File> create_always(const std::string &filename);
	static std::shared_ptr<File> open_existing(const std::string &filename);

	static std::shared_ptr<File> try_open_existing(const std::string& filename)
	{
		try
		{
			return open_existing(filename);
		}
		catch (...)
		{
			return {};
		}
	}

	static void rename(const std::string& sourceName, const std::string& destinationName);

	static void write_all_bytes(const std::string& filename, const void* data, size_t size);
	static void write_all_text(const std::string& filename, const std::string& text);
	static Array<uint8_t> read_all_bytes(const std::string& filename);
	static std::string read_all_text(const std::string& filename);
	static Array<std::string> read_all_lines(const std::string& filename);

	static bool try_delete(const std::string& filename);
	static void delete_always(const std::string& filename);

	uint8_t read_uint8() { uint8_t v; read(&v, sizeof(uint8_t)); return v; }
	int8_t read_int8() { int8_t v; read(&v, sizeof(int8_t)); return v; }
	uint16_t read_uint16() { uint16_t v; read(&v, sizeof(uint16_t)); return v; }
	int16_t read_int16() { int16_t v; read(&v, sizeof(int16_t)); return v; }
	uint32_t read_uint32() { uint32_t v; read(&v, sizeof(uint32_t)); return v; }
	int32_t read_int32() { int32_t v; read(&v, sizeof(int32_t)); return v; }
	uint64_t read_uint64() { uint64_t v; read(&v, sizeof(uint64_t)); return v; }
	int64_t read_int64() { int64_t v; read(&v, sizeof(int64_t)); return v; }

	virtual ~File() = default;
	virtual int64_t size() = 0;
	virtual void read(void *data, size_t size) = 0;
	virtual void write(const void *data, size_t size) = 0;
	virtual void seek(int64_t offset, SeekPoint origin = SeekPoint::begin) = 0;
	virtual uint64_t tell() = 0;
};

class Directory
{
public:
	static Array<std::string> files(const std::string& filename);
	static void create(const std::string& path);
	static std::string localAppData();
};

class OS
{
public:
	static std::string executable_path();
	static std::string get_default_font_name();
	static std::string find_truetype_font(const std::string& font_name);
};

class FilePath
{
public:
	static bool exists(const std::string& filepath);
	static bool has_extension(const std::string &filename, const char *extension);
	static std::string extension(const std::string &filename);
	static std::string remove_extension(const std::string &filename);
	static std::string first_component(const std::string& path);
	static std::string remove_first_component(const std::string& path);
	static std::string last_component(const std::string &path);
	static std::string remove_last_component(const std::string &path);
	static std::string combine(const std::string &path1, const std::string &path2);
	static std::string convert_path_delimiters(const std::string &path);
	// Converts a relative path to an absolute one, starting from the system path of the current game.
	static std::string relative_to_absolute_from_system(std::string game_system_path, std::string relative_path);
};
