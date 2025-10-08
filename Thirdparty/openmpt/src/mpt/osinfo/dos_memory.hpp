/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_OSINFO_DOS_MEMORY_HPP
#define MPT_OSINFO_DOS_MEMORY_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"

#if MPT_OS_DJGPP
#include "mpt/string/buffer.hpp"
#endif // MPT_OS_DJGPP

#if MPT_OS_DJGPP
#include <array>
#include <limits>
#include <optional>
#include <string_view>
#endif // MPT_OS_DJGPP

#if MPT_OS_DJGPP
#include <cstddef>
#endif // MPT_OS_DJGPP

#if MPT_OS_DJGPP
#include <dpmi.h>
#endif // MPT_OS_DJGPP



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace osinfo {

namespace dos {



#if MPT_OS_DJGPP

struct memory_info {

	std::optional<std::size_t> total_virtual;
	std::optional<std::size_t> free_virtual;
	std::optional<std::size_t> free_contiguous_virtual;

	std::optional<std::size_t> total_physical;
	std::optional<std::size_t> free_physical;
	std::optional<std::size_t> free_contiguous_physical;

	std::optional<std::size_t> unlocked_physical;

	std::optional<std::size_t> total_swap;

	inline std::optional<std::size_t> get_virtual_total() const noexcept {
		if (!total_virtual) {
			if (total_physical && total_swap) {
				return *total_physical + *total_swap;
			} else if (total_physical) {
				return total_physical;
			} else {
				return std::nullopt;
			}
		}
		return total_virtual;
	}
	inline std::optional<std::size_t> get_virtual_used() const noexcept {
		if (!get_virtual_total() || !get_virtual_free()) {
			return std::nullopt;
		}
		if (*get_virtual_total() < *get_virtual_free()) {
			return 0;
		}
		return *get_virtual_total() - *get_virtual_free();
	}
	inline std::optional<std::size_t> get_virtual_free() const noexcept {
		if (free_virtual) {
			return free_virtual;
		} else if (free_contiguous_virtual) {
			return free_contiguous_virtual;
		} else {
			return std::nullopt;
		}
	}
	inline std::optional<std::size_t> get_virtual_external_fragmentation() const noexcept {
		if (!get_virtual_free() || !free_contiguous_virtual) {
			return std::nullopt;
		}
		if (*get_virtual_free() < *free_contiguous_virtual) {
			return 0;
		}
		return *get_virtual_free() - *free_contiguous_virtual;
	}
	inline std::optional<std::size_t> get_virtual_free_contiguous() const noexcept {
		return free_contiguous_virtual;
	}

	inline std::optional<std::size_t> get_physical_total() const noexcept {
		return total_physical;
	}
	inline std::optional<std::size_t> get_physical_used() const noexcept {
		if (!total_physical || !get_physical_free()) {
			return std::nullopt;
		}
		if (*total_physical < *get_physical_free()) {
			return 0;
		}
		return *total_physical - *get_physical_free();
	}
	inline std::optional<std::size_t> get_physical_free() const noexcept {
		if (free_physical) {
			return free_physical;
		} else if (free_contiguous_physical) {
			return free_contiguous_physical;
		} else {
			return std::nullopt;
		}
	}
	inline std::optional<std::size_t> get_physical_external_fragmentation() const noexcept {
		if (!get_physical_free() || !free_contiguous_physical) {
			return std::nullopt;
		}
		if (*get_physical_free() < *free_contiguous_physical) {
			return 0;
		}
		return *get_physical_free() - *free_contiguous_physical;
	}
	inline std::optional<std::size_t> get_physical_free_contiguous() const noexcept {
		return free_contiguous_physical;
	}

	inline std::optional<std::size_t> get_physical_used_locked() const noexcept {
		if (!total_physical || !unlocked_physical) {
			return std::nullopt;
		}
		if (*total_physical < *unlocked_physical) {
			return 0;
		}
		return *total_physical - *unlocked_physical;
	}
	inline std::optional<std::size_t> get_physical_freeable() const noexcept {
		if (!unlocked_physical) {
			return get_physical_free();
		}
		return unlocked_physical;
	}

	inline std::optional<std::size_t> get_swap_total() const noexcept {
		return total_swap;
	}
};

inline memory_info get_memory_info() {
	memory_info result;
	__dpmi_free_mem_info dpmi_free_mem_info{};
	if (__dpmi_get_free_memory_information(&dpmi_free_mem_info) == 0) {
		unsigned long page_size = 0;
		if (__dpmi_get_page_size(&page_size) != 0) {
			page_size = 0;
		}
		if (dpmi_free_mem_info.largest_available_free_block_in_bytes != 0xffffffffu) {
			result.free_contiguous_virtual = dpmi_free_mem_info.largest_available_free_block_in_bytes;
		} else if ((dpmi_free_mem_info.maximum_unlocked_page_allocation_in_pages != 0xffffffffu) && (page_size > 0)) {
			result.free_contiguous_virtual = dpmi_free_mem_info.maximum_unlocked_page_allocation_in_pages * page_size;
		}
		if (page_size > 0) {
			if (dpmi_free_mem_info.maximum_locked_page_allocation_in_pages != 0xffffffffu) {
				result.free_contiguous_physical = dpmi_free_mem_info.maximum_locked_page_allocation_in_pages * page_size;
			}
			if (dpmi_free_mem_info.linear_address_space_size_in_pages != 0xffffffffu) {
				result.total_virtual = dpmi_free_mem_info.linear_address_space_size_in_pages * page_size;
			}
			if (dpmi_free_mem_info.total_number_of_unlocked_pages != 0xffffffffu) {
				result.unlocked_physical = dpmi_free_mem_info.total_number_of_unlocked_pages * page_size;
				int dpmi_flags = 0;
				std::array<char, 128> dpmi_vendor = {};
				if (__dpmi_get_capabilities(&dpmi_flags, dpmi_vendor.data()) == 0) {
					char buf[126] = {};
					std::memcpy(buf, dpmi_vendor.data() + 2, 126);
					// CWSDPMI does not track locked memory, thus all physical is reported unlocked
					if (std::string_view(mpt::ReadAutoBuf(buf)) == std::string_view("CWSDPMI")) {
						result.unlocked_physical = std::nullopt;
					}
				}
			}
			if (dpmi_free_mem_info.total_number_of_free_pages != 0xffffffffu) {
				result.free_physical = dpmi_free_mem_info.total_number_of_free_pages * page_size;
			}
			if (dpmi_free_mem_info.total_number_of_physical_pages != 0xffffffffu) {
				result.total_physical = dpmi_free_mem_info.total_number_of_physical_pages * page_size;
			}
			if (dpmi_free_mem_info.free_linear_address_space_in_pages != 0xffffffffu) {
				result.free_virtual = dpmi_free_mem_info.free_linear_address_space_in_pages * page_size;
			}
			if (dpmi_free_mem_info.size_of_paging_file_partition_in_pages != 0xffffffffu) {
				result.total_swap = dpmi_free_mem_info.size_of_paging_file_partition_in_pages * page_size;
			}
		}
	}
	return result;
}

#endif // MPT_OS_DJGPP



} // namespace dos

} // namespace osinfo



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_OSINFO_DOS_MEMORY_HPP
