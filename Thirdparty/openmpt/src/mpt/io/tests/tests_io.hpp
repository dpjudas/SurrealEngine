/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_TESTS_IO_HPP
#define MPT_IO_TESTS_IO_HPP



#include "mpt/base/alloc.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/utility.hpp"
#include "mpt/endian/integer.hpp"
#include "mpt/io/base.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"

#include <ios>
#include <sstream>
#include <string>
#include <vector>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace io {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/io")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{

	// check that empty stringstream behaves correctly with our MSVC workarounds when using iostream interface directly

	{
		std::ostringstream ss;
		MPT_TEST_EXPECT_EQUAL(static_cast<std::streamoff>(ss.tellp()), std::streamoff(0));
	}
	{
		std::ostringstream ss;
		ss.seekp(0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekAbsolute(ss, 0), true);
	}
	{
		std::ostringstream ss;
		ss.seekp(0, std::ios_base::beg);
		MPT_TEST_EXPECT_EQUAL(!ss.fail(), true);
	}
	{
		std::ostringstream ss;
		ss.seekp(0, std::ios_base::cur);
		MPT_TEST_EXPECT_EQUAL(!ss.fail(), true);
	}
	{
		std::istringstream ss;
		MPT_TEST_EXPECT_EQUAL(static_cast<std::streamoff>(ss.tellg()), std::streamoff(0));
	}
	{
		std::istringstream ss;
		ss.seekg(0, std::ios::beg);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekAbsolute(ss, 0), true);
	}
	{
		std::istringstream ss;
		ss.seekg(0, std::ios_base::beg);
		MPT_TEST_EXPECT_EQUAL(!ss.fail(), true);
	}
	{
		std::istringstream ss;
		ss.seekg(0, std::ios_base::cur);
		MPT_TEST_EXPECT_EQUAL(!ss.fail(), true);
	}

	{
		std::ostringstream s;
		char b = 23;
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		MPT_TEST_EXPECT_EQUAL(static_cast<std::streamoff>(s.tellp()), std::streamoff(0));
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		s.seekp(0, std::ios_base::beg);
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		MPT_TEST_EXPECT_EQUAL(static_cast<std::streamoff>(s.tellp()), std::streamoff(0));
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		s.write(&b, 1);
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		MPT_TEST_EXPECT_EQUAL(static_cast<std::streamoff>(s.tellp()), std::streamoff(1));
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		s.seekp(0, std::ios_base::beg);
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		MPT_TEST_EXPECT_EQUAL(static_cast<std::streamoff>(s.tellp()), std::streamoff(0));
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		s.seekp(0, std::ios_base::end);
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		MPT_TEST_EXPECT_EQUAL(static_cast<std::streamoff>(s.tellp()), std::streamoff(1));
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		MPT_TEST_EXPECT_EQUAL(s.str(), std::string(1, b));
	}

	{
		std::istringstream s;
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		MPT_TEST_EXPECT_EQUAL(static_cast<std::streamoff>(s.tellg()), std::streamoff(0));
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		s.seekg(0, std::ios_base::beg);
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		MPT_TEST_EXPECT_EQUAL(static_cast<std::streamoff>(s.tellg()), std::streamoff(0));
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		s.seekg(0, std::ios_base::end);
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		MPT_TEST_EXPECT_EQUAL(static_cast<std::streamoff>(s.tellg()), std::streamoff(0));
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
	}

	{
		std::istringstream s("a");
		char a = 0;
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		MPT_TEST_EXPECT_EQUAL(static_cast<std::streamoff>(s.tellg()), std::streamoff(0));
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		s.seekg(0, std::ios_base::beg);
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		MPT_TEST_EXPECT_EQUAL(static_cast<std::streamoff>(s.tellg()), std::streamoff(0));
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		s.read(&a, 1);
		MPT_TEST_EXPECT_EQUAL(a, 'a');
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		MPT_TEST_EXPECT_EQUAL(static_cast<std::streamoff>(s.tellg()), std::streamoff(1));
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		s.seekg(0, std::ios_base::beg);
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		MPT_TEST_EXPECT_EQUAL(static_cast<std::streamoff>(s.tellg()), std::streamoff(0));
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		s.seekg(0, std::ios_base::end);
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		MPT_TEST_EXPECT_EQUAL(static_cast<std::streamoff>(s.tellg()), std::streamoff(1));
		MPT_TEST_EXPECT_EQUAL(!s.fail(), true);
		MPT_TEST_EXPECT_EQUAL(std::string(1, a), std::string(1, 'a'));
	}

	// check that empty native and fixed stringstream both behaves correctly with out IO functions

	{
		std::ostringstream ss;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellWrite(ss), 0);
	}
	{
		std::ostringstream ss;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekBegin(ss), true);
	}
	{
		std::ostringstream ss;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekAbsolute(ss, 0), true);
	}
	{
		std::ostringstream ss;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekRelative(ss, 0), true);
	}
	{
		std::istringstream ss;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(ss), 0);
	}
	{
		std::istringstream ss;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekBegin(ss), true);
	}
	{
		std::istringstream ss;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekAbsolute(ss, 0), true);
	}
	{
		std::istringstream ss;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekRelative(ss, 0), true);
	}

	{
		std::ostringstream ss;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellWrite(ss), 0);
	}
	{
		std::ostringstream ss;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekBegin(ss), true);
	}
	{
		std::ostringstream ss;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekAbsolute(ss, 0), true);
	}
	{
		std::ostringstream ss;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekRelative(ss, 0), true);
	}
	{
		std::istringstream ss;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(ss), 0);
	}
	{
		std::istringstream ss;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekBegin(ss), true);
	}
	{
		std::istringstream ss;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekAbsolute(ss, 0), true);
	}
	{
		std::istringstream ss;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekRelative(ss, 0), true);
	}

	{
		std::ostringstream s;
		char b = 23;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellWrite(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekBegin(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellWrite(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::WriteRaw(s, &b, 1), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellWrite(s), 1);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekBegin(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellWrite(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekEnd(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellWrite(s), 1);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(s.str(), std::string(1, b));
	}

	{
		std::istringstream s;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekBegin(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekEnd(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
	}

	{
		std::istringstream s("a");
		char a = 0;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekBegin(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::ReadRaw(s, &a, 1).size(), 1u);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(s), 1);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekBegin(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekEnd(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(s), 1);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(std::string(1, a), std::string(1, 'a'));
	}

	{
		std::ostringstream s;
		char b = 23;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellWrite(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekBegin(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellWrite(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::WriteRaw(s, &b, 1), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellWrite(s), 1);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekBegin(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellWrite(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekEnd(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellWrite(s), 1);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(s.str(), std::string(1, b));
	}

	{
		std::istringstream s;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekBegin(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekEnd(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
	}

	{
		std::istringstream s("a");
		char a = 0;
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekBegin(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::ReadRaw(s, &a, 1).size(), 1u);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(s), 1);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekBegin(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(s), 0);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::SeekEnd(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::TellRead(s), 1);
		MPT_TEST_EXPECT_EQUAL(mpt::IO::IsValid(s), true);
		MPT_TEST_EXPECT_EQUAL(std::string(1, a), std::string(1, 'a'));
	}

	// General file I/O tests
	{
		// Verify that writing arrays does not confuse the compiler.
		// This is both, compile-time and run-time cheking.
		// Run-time in case some weird compiler gets confused by our templates
		// and only writes the first array element.
		std::ostringstream f;
		mpt::uint16be data[2];
		mpt::reset(data);
		data[0] = 0x1234;
		data[1] = 0x5678;
		mpt::IO::Write(f, data);
		MPT_TEST_EXPECT_EQUAL(f.str(), std::string("\x12\x34\x56\x78"));
	}
	{
		std::ostringstream f;
		std::vector<mpt::int16be> data;
		data.resize(3);
		data[0] = 0x1234;
		data[1] = 0x5678;
		data[2] = 0x1234;
		mpt::IO::Write(f, data);
		MPT_TEST_EXPECT_EQUAL(f.str(), std::string("\x12\x34\x56\x78\x12\x34"));
	}
	{
		std::ostringstream f;
		mpt::int16be data[3];
		mpt::reset(data);
		data[0] = 0x1234;
		data[1] = 0x5678;
		data[2] = 0x1234;
		mpt::IO::Write(f, data);
		MPT_TEST_EXPECT_EQUAL(f.str(), std::string("\x12\x34\x56\x78\x12\x34"));
	}

	{
		auto TestAdaptive16 = [&](uint16 value, mpt::IO::Offset expected_size, std::size_t fixedSize, const char * bytes) {
			std::stringstream f;
			MPT_TEST_EXPECT_EQUAL(mpt::IO::WriteAdaptiveInt16LE(f, value, fixedSize), true);
			MPT_TEST_EXPECT_EQUAL(mpt::IO::TellWrite(f), expected_size);
			if (bytes) {
				mpt::IO::SeekBegin(f);
				for (mpt::IO::Offset i = 0; i < expected_size; ++i) {
					uint8 val = 0;
					mpt::IO::ReadIntLE<uint8>(f, val);
					MPT_TEST_EXPECT_EQUAL(val, static_cast<uint8>(bytes[i]));
				}
			}
			mpt::IO::SeekBegin(f);
			uint16 result = 0;
			MPT_TEST_EXPECT_EQUAL(mpt::IO::ReadAdaptiveInt16LE(f, result), true);
			MPT_TEST_EXPECT_EQUAL(result, value);
		};
		auto TestAdaptive32 = [&](uint32 value, mpt::IO::Offset expected_size, std::size_t fixedSize, const char * bytes) {
			std::stringstream f;
			MPT_TEST_EXPECT_EQUAL(mpt::IO::WriteAdaptiveInt32LE(f, value, fixedSize), true);
			MPT_TEST_EXPECT_EQUAL(mpt::IO::TellWrite(f), expected_size);
			if (bytes) {
				mpt::IO::SeekBegin(f);
				for (mpt::IO::Offset i = 0; i < expected_size; ++i) {
					uint8 val = 0;
					mpt::IO::ReadIntLE<uint8>(f, val);
					MPT_TEST_EXPECT_EQUAL(val, static_cast<uint8>(bytes[i]));
				}
			}
			mpt::IO::SeekBegin(f);
			uint32 result = 0;
			MPT_TEST_EXPECT_EQUAL(mpt::IO::ReadAdaptiveInt32LE(f, result), true);
			MPT_TEST_EXPECT_EQUAL(result, value);
		};
		auto TestAdaptive64 = [&](uint64 value, mpt::IO::Offset expected_size, std::size_t fixedSize, const char * bytes) {
			std::stringstream f;
			MPT_TEST_EXPECT_EQUAL(mpt::IO::WriteAdaptiveInt64LE(f, value, fixedSize), true);
			MPT_TEST_EXPECT_EQUAL(mpt::IO::TellWrite(f), expected_size);
			if (bytes) {
				mpt::IO::SeekBegin(f);
				for (mpt::IO::Offset i = 0; i < expected_size; ++i) {
					uint8 val = 0;
					mpt::IO::ReadIntLE<uint8>(f, val);
					MPT_TEST_EXPECT_EQUAL(val, static_cast<uint8>(bytes[i]));
				}
			}
			mpt::IO::SeekBegin(f);
			uint64 result = 0;
			MPT_TEST_EXPECT_EQUAL(mpt::IO::ReadAdaptiveInt64LE(f, result), true);
			MPT_TEST_EXPECT_EQUAL(result, value);
		};
		TestAdaptive16(0, 1, 0, "\x00");
		TestAdaptive16(1, 1, 0, "\x02");
		TestAdaptive16(2, 1, 0, nullptr);
		TestAdaptive16(0x7f, 1, 0, nullptr);
		TestAdaptive16(0x80, 2, 0, "\x01\x01");
		TestAdaptive16(0x81, 2, 0, "\x03\x01");
		TestAdaptive16(0x7fff, 2, 0, "\xff\xff");
		TestAdaptive16(0, 1, 1, nullptr);
		TestAdaptive16(1, 1, 1, nullptr);
		TestAdaptive16(2, 1, 1, nullptr);
		TestAdaptive16(0x7f, 1, 1, nullptr);
		TestAdaptive16(0x80, 2, 0, nullptr);
		TestAdaptive16(0x81, 2, 0, nullptr);
		TestAdaptive16(0x7fff, 2, 0, nullptr);
		TestAdaptive16(0, 2, 2, "\x01\x00");
		TestAdaptive16(1, 2, 2, "\x03\x00");
		TestAdaptive16(2, 2, 2, nullptr);
		TestAdaptive16(0x7f, 2, 2, nullptr);
		TestAdaptive16(0x80, 2, 2, nullptr);
		TestAdaptive16(0x81, 2, 2, nullptr);
		TestAdaptive16(0x7fff, 2, 2, nullptr);

		TestAdaptive32(0, 1, 0, "\x00");
		TestAdaptive32(1, 1, 0, nullptr);
		TestAdaptive32(2, 1, 0, nullptr);
		TestAdaptive32(0x3f, 1, 0, nullptr);
		TestAdaptive32(0x40, 2, 0, "\x01\x01");
		TestAdaptive32(0x41, 2, 0, "\x05\x01");
		TestAdaptive32(0x7f, 2, 0, nullptr);
		TestAdaptive32(0x80, 2, 0, nullptr);
		TestAdaptive32(0x3fff, 2, 0, nullptr);
		TestAdaptive32(0x4000, 3, 0, "\x02\x00\x01");
		TestAdaptive32(0x4001, 3, 0, nullptr);
		TestAdaptive32(0x3fffff, 3, 0, nullptr);
		TestAdaptive32(0x400000, 4, 0, "\x03\x00\x00\x01");
		TestAdaptive32(0x400001, 4, 0, nullptr);
		TestAdaptive32(0x3fffffff, 4, 0, "\xff\xff\xff\xff");
		TestAdaptive32(0, 2, 2, nullptr);
		TestAdaptive32(1, 2, 2, nullptr);
		TestAdaptive32(2, 2, 2, nullptr);
		TestAdaptive32(0x3f, 2, 2, nullptr);
		TestAdaptive32(0x40, 2, 2, nullptr);
		TestAdaptive32(0x41, 2, 2, nullptr);
		TestAdaptive32(0x7f, 2, 2, nullptr);
		TestAdaptive32(0x80, 2, 2, nullptr);
		TestAdaptive32(0x3fff, 2, 2, nullptr);
		TestAdaptive32(0, 3, 3, nullptr);
		TestAdaptive32(1, 3, 3, nullptr);
		TestAdaptive32(2, 3, 3, nullptr);
		TestAdaptive32(0x3f, 3, 3, nullptr);
		TestAdaptive32(0x40, 3, 3, nullptr);
		TestAdaptive32(0x41, 3, 3, nullptr);
		TestAdaptive32(0x7f, 3, 3, nullptr);
		TestAdaptive32(0x80, 3, 3, nullptr);
		TestAdaptive32(0x3fff, 3, 3, nullptr);
		TestAdaptive32(0x4000, 3, 3, nullptr);
		TestAdaptive32(0x4001, 3, 3, nullptr);
		TestAdaptive32(0x3fffff, 3, 3, nullptr);
		TestAdaptive32(0, 4, 4, nullptr);
		TestAdaptive32(1, 4, 4, nullptr);
		TestAdaptive32(2, 4, 4, nullptr);
		TestAdaptive32(0x3f, 4, 4, nullptr);
		TestAdaptive32(0x40, 4, 4, nullptr);
		TestAdaptive32(0x41, 4, 4, nullptr);
		TestAdaptive32(0x7f, 4, 4, nullptr);
		TestAdaptive32(0x80, 4, 4, nullptr);
		TestAdaptive32(0x3fff, 4, 4, nullptr);
		TestAdaptive32(0x4000, 4, 4, nullptr);
		TestAdaptive32(0x4001, 4, 4, nullptr);
		TestAdaptive32(0x3fffff, 4, 4, nullptr);
		TestAdaptive32(0x400000, 4, 4, nullptr);
		TestAdaptive32(0x400001, 4, 4, nullptr);
		TestAdaptive32(0x3fffffff, 4, 4, nullptr);

		TestAdaptive64(0, 1, 0, nullptr);
		TestAdaptive64(1, 1, 0, nullptr);
		TestAdaptive64(2, 1, 0, nullptr);
		TestAdaptive64(0x3f, 1, 0, nullptr);
		TestAdaptive64(0x40, 2, 0, nullptr);
		TestAdaptive64(0x41, 2, 0, nullptr);
		TestAdaptive64(0x7f, 2, 0, nullptr);
		TestAdaptive64(0x80, 2, 0, nullptr);
		TestAdaptive64(0x3fff, 2, 0, nullptr);
		TestAdaptive64(0x4000, 4, 0, nullptr);
		TestAdaptive64(0x4001, 4, 0, nullptr);
		TestAdaptive64(0x3fffff, 4, 0, nullptr);
		TestAdaptive64(0x400000, 4, 0, nullptr);
		TestAdaptive64(0x400001, 4, 0, nullptr);
		TestAdaptive64(0x3fffffff, 4, 0, nullptr);
		TestAdaptive64(0x40000000, 8, 0, nullptr);
		TestAdaptive64(0x40000001, 8, 0, nullptr);
		TestAdaptive64(0x3fffffffffffffffull, 8, 0, nullptr);
		TestAdaptive64(0, 2, 2, nullptr);
		TestAdaptive64(1, 2, 2, nullptr);
		TestAdaptive64(2, 2, 2, nullptr);
		TestAdaptive64(0x3f, 2, 2, nullptr);
		TestAdaptive64(0, 4, 4, nullptr);
		TestAdaptive64(1, 4, 4, nullptr);
		TestAdaptive64(2, 4, 4, nullptr);
		TestAdaptive64(0x3f, 4, 4, nullptr);
		TestAdaptive64(0x40, 4, 4, nullptr);
		TestAdaptive64(0x41, 4, 4, nullptr);
		TestAdaptive64(0x7f, 4, 4, nullptr);
		TestAdaptive64(0x80, 4, 4, nullptr);
		TestAdaptive64(0x3fff, 4, 4, nullptr);
		TestAdaptive64(0, 8, 8, nullptr);
		TestAdaptive64(1, 8, 8, nullptr);
		TestAdaptive64(2, 8, 8, nullptr);
		TestAdaptive64(0x3f, 8, 8, nullptr);
		TestAdaptive64(0x40, 8, 8, nullptr);
		TestAdaptive64(0x41, 8, 8, nullptr);
		TestAdaptive64(0x7f, 8, 8, nullptr);
		TestAdaptive64(0x80, 8, 8, nullptr);
		TestAdaptive64(0x3fff, 8, 8, nullptr);
		TestAdaptive64(0x4000, 8, 8, nullptr);
		TestAdaptive64(0x4001, 8, 8, nullptr);
		TestAdaptive64(0x3fffff, 8, 8, nullptr);
		TestAdaptive64(0x400000, 8, 8, nullptr);
		TestAdaptive64(0x400001, 8, 8, nullptr);
		TestAdaptive64(0x3fffffff, 8, 8, nullptr);
		TestAdaptive64(0x40000000, 8, 8, nullptr);
		TestAdaptive64(0x40000001, 8, 8, nullptr);
		TestAdaptive64(0x3fffffffffffffffull, 8, 8, nullptr);
	}
}

} // namespace io
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_TESTS_IO_HPP
