/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_IO_VIRTUAL_WRAPPER_HPP
#define MPT_IO_IO_VIRTUAL_WRAPPER_HPP



#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/io/base.hpp"

#include <type_traits>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



class IFileBase {
protected:
	IFileBase() = default;
	virtual ~IFileBase() = default;

public:
	virtual bool IsValid() = 0;
	virtual bool IsReadSeekable() = 0;
	virtual IO::Offset TellRead() = 0;
	virtual bool SeekBegin() = 0;
	virtual bool SeekEnd() = 0;
	virtual bool SeekAbsolute(IO::Offset pos) = 0;
	virtual bool SeekRelative(IO::Offset off) = 0;
	virtual mpt::byte_span ReadRawImpl(mpt::byte_span data) = 0;
	virtual bool IsEof() = 0;
};

template <typename Tfile>
class IFile
	: public IFileBase {
private:
	Tfile & f;

public:
	IFile(Tfile & f_)
		: f(f_) {
	}
	~IFile() override = default;

public:
	bool IsValid() override {
		return mpt::IO::IsValid(f);
	}
	bool IsReadSeekable() override {
		return mpt::IO::IsReadSeekable(f);
	}
	IO::Offset TellRead() override {
		return mpt::IO::TellRead(f);
	}
	bool SeekBegin() override {
		return mpt::IO::SeekBegin(f);
	}
	bool SeekEnd() override {
		return mpt::IO::SeekEnd(f);
	}
	bool SeekAbsolute(IO::Offset pos) override {
		return mpt::IO::SeekAbsolute(f, pos);
	}
	bool SeekRelative(IO::Offset off) override {
		return mpt::IO::SeekRelative(f, off);
	}
	mpt::byte_span ReadRawImpl(mpt::byte_span data) override {
		return mpt::IO::ReadRawImpl(f, data);
	}
	bool IsEof() override {
		return mpt::IO::IsEof(f);
	}
};



class OFileBase {
protected:
	OFileBase() = default;
	virtual ~OFileBase() = default;

public:
	virtual bool IsValid() = 0;
	virtual bool IsWriteSeekable() = 0;
	virtual IO::Offset TellWrite() = 0;
	virtual bool SeekBegin() = 0;
	virtual bool SeekEnd() = 0;
	virtual bool SeekAbsolute(IO::Offset pos) = 0;
	virtual bool SeekRelative(IO::Offset off) = 0;
	virtual bool WriteRawImpl(mpt::const_byte_span data) = 0;
	virtual bool Flush() = 0;
};

template <typename Tfile>
class OFile
	: public OFileBase {
private:
	Tfile & f;

public:
	OFile(Tfile & f_)
		: f(f_) {
	}
	~OFile() override = default;

public:
	bool IsValid() override {
		return mpt::IO::IsValid(f);
	}
	bool IsWriteSeekable() override {
		return mpt::IO::IsWriteSeekable(f);
	}
	IO::Offset TellWrite() override {
		return mpt::IO::TellWrite(f);
	}
	bool SeekBegin() override {
		return mpt::IO::SeekBegin(f);
	}
	bool SeekEnd() override {
		return mpt::IO::SeekEnd(f);
	}
	bool SeekAbsolute(IO::Offset pos) override {
		return mpt::IO::SeekAbsolute(f, pos);
	}
	bool SeekRelative(IO::Offset off) override {
		return mpt::IO::SeekRelative(f, off);
	}
	bool WriteRawImpl(mpt::const_byte_span data) override {
		return mpt::IO::WriteRawImpl(f, data);
	}
	bool Flush() override {
		return mpt::IO::Flush(f);
	}
};



class IOFileBase {
protected:
	IOFileBase() = default;
	virtual ~IOFileBase() = default;

public:
	virtual bool IsValid() = 0;
	virtual bool IsReadSeekable() = 0;
	virtual bool IsWriteSeekable() = 0;
	virtual IO::Offset TellRead() = 0;
	virtual IO::Offset TellWrite() = 0;
	virtual bool SeekBegin() = 0;
	virtual bool SeekEnd() = 0;
	virtual bool SeekAbsolute(IO::Offset pos) = 0;
	virtual bool SeekRelative(IO::Offset off) = 0;
	virtual mpt::byte_span ReadRawImpl(mpt::byte_span data) = 0;
	virtual bool WriteRawImpl(mpt::const_byte_span data) = 0;
	virtual bool IsEof() = 0;
	virtual bool Flush() = 0;
};

template <typename Tfile>
class IOFile
	: public IOFileBase {
private:
	Tfile & f;

public:
	IOFile(Tfile & f_)
		: f(f_) {
	}
	~IOFile() override = default;

public:
	bool IsValid() override {
		return mpt::IO::IsValid(f);
	}
	bool IsReadSeekable() override {
		return mpt::IO::IsReadSeekable(f);
	}
	bool IsWriteSeekable() override {
		return mpt::IO::IsWriteSeekable(f);
	}
	IO::Offset TellRead() override {
		return mpt::IO::TellRead(f);
	}
	IO::Offset TellWrite() override {
		return mpt::IO::TellWrite(f);
	}
	bool SeekBegin() override {
		return mpt::IO::SeekBegin(f);
	}
	bool SeekEnd() override {
		return mpt::IO::SeekEnd(f);
	}
	bool SeekAbsolute(IO::Offset pos) override {
		return mpt::IO::SeekAbsolute(f, pos);
	}
	bool SeekRelative(IO::Offset off) override {
		return mpt::IO::SeekRelative(f, off);
	}
	mpt::byte_span ReadRawImpl(mpt::byte_span data) override {
		return mpt::IO::ReadRawImpl(f, data);
	}
	bool WriteRawImpl(mpt::const_byte_span data) override {
		return mpt::IO::WriteRawImpl(f, data);
	}
	bool IsEof() override {
		return mpt::IO::IsEof(f);
	}
	bool Flush() override {
		return mpt::IO::Flush(f);
	}
};



template <typename Tfile>
struct FileOperations<Tfile, typename std::enable_if_t<std::is_base_of<IFileBase, Tfile>::value>> {

private:
	IFileBase & f;

public:
	FileOperations(IFileBase & f_)
		: f(f_) {
		return;
	}

public:
	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool IsValid() {
		return f.IsValid();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool IsReadSeekable() {
		return f.IsReadSeekable();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline IO::Offset TellRead() {
		return f.TellRead();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekBegin() {
		return f.SeekBegin();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekEnd() {
		return f.SeekEnd();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekAbsolute(IO::Offset pos) {
		return f.SeekAbsolute(pos);
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekRelative(IO::Offset off) {
		return f.SeekRelative(off);
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline mpt::byte_span ReadRawImpl(mpt::byte_span data) {
		return f.ReadRawImpl(data);
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool IsEof() {
		return f.IsEof();
	}
};



template <typename Tfile>
struct FileOperations<Tfile, typename std::enable_if_t<std::is_base_of<OFileBase, Tfile>::value>> {

private:
	OFileBase & f;

public:
	FileOperations(OFileBase & f_)
		: f(f_) {
		return;
	}

public:
	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool IsValid() {
		return f.IsValid();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool IsWriteSeekable() {
		return f.IsWriteSeekable();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline IO::Offset TellWrite() {
		return f.TellWrite();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekBegin() {
		return f.SeekBegin();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekEnd() {
		return f.SeekEnd();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekAbsolute(IO::Offset pos) {
		return f.SeekAbsolute(pos);
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekRelative(IO::Offset off) {
		return f.SeekRelative(off);
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool WriteRawImpl(mpt::const_byte_span data) {
		return f.WriteRawImpl(data);
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool Flush() {
		return f.Flush();
	}
};



template <typename Tfile>
struct FileOperations<Tfile, typename std::enable_if_t<std::is_base_of<IOFileBase, Tfile>::value>> {

private:
	IOFileBase & f;

public:
	FileOperations(IOFileBase & f_)
		: f(f_) {
		return;
	}

public:
	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool IsValid() {
		return f.IsValid();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool IsReadSeekable() {
		return f.IsReadSeekable();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool IsWriteSeekable() {
		return f.IsWriteSeekable();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline IO::Offset TellRead() {
		return f.TellRead();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline IO::Offset TellWrite() {
		return f.TellWrite();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekBegin() {
		return f.SeekBegin();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekEnd() {
		return f.SeekEnd();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekAbsolute(IO::Offset pos) {
		return f.SeekAbsolute(pos);
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekRelative(IO::Offset off) {
		return f.SeekRelative(off);
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline mpt::byte_span ReadRawImpl(mpt::byte_span data) {
		return f.ReadRawImpl(data);
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool WriteRawImpl(mpt::const_byte_span data) {
		return f.WriteRawImpl(data);
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool IsEof() {
		return f.IsEof();
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool Flush() {
		return f.Flush();
	}
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_IO_VIRTUAL_WRAPPER_HPP
