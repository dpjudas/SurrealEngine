/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_TEST_TEST_MACROS_HPP
#define MPT_TEST_TEST_MACROS_HPP



#include "mpt/base/namespace.hpp"
#include "mpt/base/preprocessor.hpp"
#include "mpt/test/test.hpp"

#include <functional>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace test {



#define MPT_TEST_GROUP_BEGIN(name) \
	inline mpt::test::group MPT_PP_UNIQUE_IDENTIFIER(mpt_test_group_name) { \
		name, [](mpt::test::context & mpt_test_context) {
#define MPT_TEST_GROUP_END() \
	} \
	} \
	;

#define MPT_TEST_GROUP_INLINE_IDENTIFIER(identifier, name) \
	inline void MPT_PP_JOIN(mpt_test_group_func_, identifier)(mpt::test::context & mpt_test_context); \
	inline mpt::test::group MPT_PP_JOIN(mpt_test_group_name_, identifier){ \
		name, [](mpt::test::context & mpt_test_context) { \
			MPT_PP_JOIN(mpt_test_group_func_, identifier) \
			(mpt_test_context); \
		}}; \
	inline void MPT_PP_UNIQUE_IDENTIFIER(mpt_test_group_func)(mpt::test::context & mpt_test_context)

#define MPT_TEST_GROUP_INLINE(name) \
	inline void MPT_PP_UNIQUE_IDENTIFIER(mpt_test_group_func)(mpt::test::context & mpt_test_context); \
	inline mpt::test::group MPT_PP_UNIQUE_IDENTIFIER(mpt_test_group_name){ \
		name, [](mpt::test::context & mpt_test_context) { \
			MPT_PP_UNIQUE_IDENTIFIER(mpt_test_group_func) \
			(mpt_test_context); \
		}}; \
	inline void MPT_PP_UNIQUE_IDENTIFIER(mpt_test_group_func)(mpt::test::context & mpt_test_context)

#define MPT_TEST_GROUP_STATIC(name) \
	static void MPT_PP_UNIQUE_IDENTIFIER(mpt_test_group_func)(mpt::test::context & mpt_test_context); \
	static mpt::test::group MPT_PP_UNIQUE_IDENTIFIER(mpt_test_group_name){ \
		name, [](mpt::test::context & mpt_test_context) { \
			MPT_PP_UNIQUE_IDENTIFIER(mpt_test_group_func) \
			(mpt_test_context); \
		}}; \
	static void MPT_PP_UNIQUE_IDENTIFIER(mpt_test_group_func)(mpt::test::context & mpt_test_context)

#define MPT_TEST_DELAYED(x) [&] { \
	return x; \
}

#define MPT_TEST_EXPECT mpt::test::test{context}.expect

#define MPT_TEST_EXPECT_EXPR(e)                 mpt::test::test{mpt_test_context}.expect([&] { return e; }, #e)
#define MPT_TEST_EXPECT_CMP(a, cmp, b)          mpt::test::test{mpt_test_context}.expect([&] { return a; }, [](const auto & a_, const auto & b_) { return a_ cmp b_; }, [&] { return b; }, #a, #cmp, #b)
#define MPT_TEST_EXPECT_THROWS_ANY(expr)        mpt::test::test{mpt_test_context}.expect_throws_any([&] { expr; }, #expr)
#define MPT_TEST_EXPECT_THROWS(exception, expr) mpt::test::test{mpt_test_context}.expect_throws<exception>([&] { expr; }, #exception, #expr)

#define MPT_TEST_EXPECT_EQUAL(a, b) mpt::test::test{mpt_test_context}.expect([&] { return a; }, std::equal_to<>{}, [&] { return b; }, #a, "==", #b)



} // namespace test



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_TEST_TEST_MACROS_HPP
