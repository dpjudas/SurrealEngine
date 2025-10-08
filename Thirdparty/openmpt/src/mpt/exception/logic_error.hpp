/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_EXCEPTION_LOGIC_ERROR_HPP
#define MPT_EXCEPTION_LOGIC_ERROR_HPP



#include "mpt/base/namespace.hpp"
#include "mpt/exception/exception.hpp"

#include <stdexcept>



namespace mpt {
inline namespace MPT_INLINE_NS {



using logic_error = mpt::exception_ustring_wrapper<std::logic_error>;



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_EXCEPTION_LOGIC_ERROR_HPP
