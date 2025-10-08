/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_READ_CALLBACKSTREAM_HPP
#define MPT_IO_READ_CALLBACKSTREAM_HPP



#include "mpt/base/integer.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



template <typename Tstream>
struct CallbackStreamTemplate {
	enum : int {
		SeekSet = 0,
		SeekCur = 1,
		SeekEnd = 2
	};
	Tstream stream;
	std::size_t (*read)(Tstream stream, void * dst, std::size_t bytes);
	int (*seek)(Tstream stream, int64 offset, int whence);
	int64 (*tell)(Tstream stream);
};

using CallbackStream = CallbackStreamTemplate<void *>;



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_READ_CALLBACKSTREAM_HPP
