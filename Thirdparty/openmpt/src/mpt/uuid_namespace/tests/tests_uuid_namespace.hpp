/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_TESTS_UUID_NAMESPACE_HPP
#define MPT_BASE_TESTS_UUID_NAMESPACE_HPP



#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/string/types.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"
#include "mpt/uuid/uuid.hpp"
#include "mpt/uuid_namespace/uuid_namespace.hpp"



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace uuid_namespace {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/uuid_namespace")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
	using namespace mpt::uuid_literals;
	constexpr mpt::UUID uuid_ns_dns = "6ba7b810-9dad-11d1-80b4-00c04fd430c8"_uuid;
	constexpr mpt::UUID expected = "74738ff5-5367-5958-9aee-98fffdcd1876"_uuid;
	mpt::UUID gotten = mpt::UUIDRFC4122NamespaceV5(uuid_ns_dns, MPT_USTRING("www.example.org"));
	MPT_TEST_EXPECT_EQUAL(gotten, expected);
}

} // namespace uuid_namespace
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_TESTS_UUID_NAMESPACE_HPP
