
#include "ZipWriter.h"
#include "miniz.h"
#include <algorithm>

class ZipWriterImpl : public ZipWriter
{
public:
	ZipWriterImpl(std::shared_ptr<IODevice> output) : output(output)
	{
		mz_zip_zero_struct(&zip);
		zip.m_pWrite = &ZipWriterImpl::write_func;
		zip.m_pIO_opaque = this;
		mz_bool result = mz_zip_writer_init(&zip, 0);
		if (result == MZ_FALSE)
			throw std::runtime_error("mz_zip_writer_init failed");
	}

	~ZipWriterImpl()
	{
		mz_zip_writer_end(&zip);
	}

	void addFile(const std::string& filename, bool compress_file, const void* data, uint64_t size) override
	{
		mz_uint flags = compress_file ? MZ_BEST_SPEED : MZ_NO_COMPRESSION;
		mz_bool result = mz_zip_writer_add_mem(&zip, filename.c_str(), data, size, flags);
		if (result == MZ_FALSE)
			throw std::runtime_error("mz_zip_writer_add_mem failed");
	}

	void addFile(const std::string& filename, bool compress_file, const void* data, uint64_t size, int64_t lastModified) override
	{
		mz_uint flags = compress_file ? MZ_BEST_SPEED : MZ_NO_COMPRESSION;
		mz_bool result = mz_zip_writer_add_mem_ex_v2(&zip, filename.c_str(), data, size, nullptr, 0, flags, 0, 0, (MZ_TIME_T*)&lastModified, nullptr, 0, nullptr, 0);
		if (result == MZ_FALSE)
			throw std::runtime_error("mz_zip_writer_add_mem_ex_v2 failed");
	}

	void writeToc() override
	{
		mz_bool result = mz_zip_writer_finalize_archive(&zip);
		if (result == MZ_FALSE)
			throw std::runtime_error("mz_zip_writer_finalize_archive failed");
	}

	static size_t write_func(void* opaque, mz_uint64 file_offset, const void* data, size_t size)
	{
		ZipWriterImpl* self = static_cast<ZipWriterImpl*>(opaque);
		if (file_offset != self->pos)
		{
			if (self->output->seek(file_offset) != file_offset)
				throw std::runtime_error("ZipWriter seek failed");
			self->pos = file_offset;
		}

		size_t pos = 0;
		while (pos < size)
		{
			int intsize = (int)std::min(size, (size_t)0x70000000);
			self->output->write(data, intsize);
			pos += intsize;
		}
		self->pos += size;
		return size;
	}

	std::shared_ptr<IODevice> output;
	bool storeFilenamesAsUTF8 = true;
	mz_zip_archive zip = {};
	mz_uint64 pos = 0;
};

std::unique_ptr<ZipWriter> ZipWriter::create(std::shared_ptr<IODevice> output)
{
	return std::make_unique<ZipWriterImpl>(std::move(output));
}
