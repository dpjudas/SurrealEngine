/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_READ_FILECURSOR_FILENAME_TRAITS_HPP
#define MPT_IO_READ_FILECURSOR_FILENAME_TRAITS_HPP



#include "mpt/base/namespace.hpp"

#include <memory>
#include <optional>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



class FileCursorFilenameTraitsNone {

public:
	struct empty_type { };

	using filename_type = empty_type;
	using shared_filename_type = empty_type;

	static std::optional<filename_type> get_optional_filename(shared_filename_type /* filename */) {
		return std::nullopt;
	}
};

template <typename Tpath>
class FileCursorFilenameTraits {

public:
	using filename_type = Tpath;
	using shared_filename_type = std::shared_ptr<Tpath>;

	static std::optional<filename_type> get_optional_filename(const shared_filename_type & filename) {
		if (!filename) {
			return std::nullopt;
		}
		if ((*filename) == Tpath{}) {
			return std::nullopt;
		}
		return *filename;
	}
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_READ_FILECURSOR_FILENAME_TRAITS_HPP
