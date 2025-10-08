/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_ARCH_FEATURE_FLAGS_HPP
#define MPT_ARCH_FEATURE_FLAGS_HPP


#include "mpt/base/namespace.hpp"



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace arch {



template <typename bitset>
struct [[nodiscard]] basic_feature_flags {
private:
	bitset m_flags{};
public:
	constexpr basic_feature_flags() noexcept = default;
	explicit constexpr basic_feature_flags(bitset flags) noexcept
		: m_flags(flags) {
		return;
	}
	[[nodiscard]] constexpr basic_feature_flags operator~() noexcept {
		return basic_feature_flags{~m_flags};
	}
	constexpr basic_feature_flags & operator&=(basic_feature_flags o) noexcept {
		m_flags &= o.m_flags;
		return *this;
	}
	constexpr basic_feature_flags & operator|=(basic_feature_flags o) noexcept {
		m_flags |= o.m_flags;
		return *this;
	}
	constexpr basic_feature_flags & operator^=(basic_feature_flags o) noexcept {
		m_flags ^= o.m_flags;
		return *this;
	}
	[[nodiscard]] friend constexpr basic_feature_flags operator&(basic_feature_flags a, basic_feature_flags b) noexcept {
		return basic_feature_flags{static_cast<bitset>(a.m_flags & b.m_flags)};
	}
	[[nodiscard]] friend constexpr basic_feature_flags operator|(basic_feature_flags a, basic_feature_flags b) noexcept {
		return basic_feature_flags{static_cast<bitset>(a.m_flags | b.m_flags)};
	}
	[[nodiscard]] friend constexpr basic_feature_flags operator^(basic_feature_flags a, basic_feature_flags b) noexcept {
		return basic_feature_flags{static_cast<bitset>(a.m_flags ^ b.m_flags)};
	}
	[[nodiscard]] friend constexpr bool operator==(basic_feature_flags a, basic_feature_flags b) noexcept {
		return a.m_flags == b.m_flags;
	}
	[[nodiscard]] friend constexpr bool operator!=(basic_feature_flags a, basic_feature_flags b) noexcept {
		return a.m_flags != b.m_flags;
	}
	[[nodiscard]] constexpr bool supports(basic_feature_flags query) noexcept {
		return (m_flags & query.m_flags) == query.m_flags;
	}
	[[nodiscard]] explicit constexpr operator bool() const noexcept {
		return m_flags ? true : false;
	}
	[[nodiscard]] constexpr bool operator!() const noexcept {
		return m_flags ? false : true;
	}
};



} // namespace arch



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_ARCH_FEATURE_FLAGS_HPP
