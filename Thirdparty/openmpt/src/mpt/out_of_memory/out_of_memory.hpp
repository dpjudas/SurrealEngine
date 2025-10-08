/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_OUT_OF_MEMORY_OUT_OF_MEMORY_HPP
#define MPT_OUT_OF_MEMORY_OUT_OF_MEMORY_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/detect/mfc.hpp"

#include <exception>
#if !MPT_DETECTED_MFC
#include <new>
#endif // !MPT_DETECTED_MFC

#if MPT_DETECTED_MFC
// cppcheck-suppress missingInclude
#include <afx.h>
#endif // MPT_DETECTED_MFC



namespace mpt {
inline namespace MPT_INLINE_NS {


// Exception handling helpers, because MFC requires explicit deletion of the exception object,
// Thus, always call exactly one of mpt::rethrow_out_of_memory(e) or mpt::delete_out_of_memory(e).

#if MPT_DETECTED_MFC

using out_of_memory = CMemoryException *;

[[noreturn]] inline void throw_out_of_memory() {
	AfxThrowMemoryException();
}

[[noreturn]] inline void rethrow_out_of_memory(out_of_memory e) {
	MPT_UNUSED(e);
	// cppcheck false-positive
	// cppcheck-suppress rethrowNoCurrentException
	throw;
}

inline void delete_out_of_memory(out_of_memory & e) {
	if (e) {
		e->Delete();
		e = nullptr;
	}
}

#else // !MPT_DETECTED_MFC

using out_of_memory = const std::bad_alloc &;

[[noreturn]] inline void throw_out_of_memory() {
	throw std::bad_alloc();
}

[[noreturn]] inline void rethrow_out_of_memory(out_of_memory e) {
	MPT_UNUSED(e);
	// cppcheck false-positive
	// cppcheck-suppress rethrowNoCurrentException
	throw;
}

inline void delete_out_of_memory(out_of_memory e) {
	MPT_UNUSED(e);
}

#endif // MPT_DETECTED_MFC


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_OUT_OF_MEMORY_OUT_OF_MEMORY_HPP
