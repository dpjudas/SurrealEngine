/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_SEMANTIC_VERSION_HPP
#define MPT_BASE_SEMANTIC_VERSION_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/version.hpp"



#include <tuple>



namespace mpt {
inline namespace MPT_INLINE_NS {


struct semantic_version {
	unsigned long long major = 0;
	unsigned long long minor = 0;
	unsigned long long patch = 0;
	constexpr std::tuple<unsigned long long, unsigned long long, unsigned long long> as_tuple() const noexcept {
		return std::make_tuple(major, minor, patch);
	}
};

constexpr bool operator==(const semantic_version a, const semantic_version b) noexcept {
	return a.as_tuple() == b.as_tuple();
}
constexpr bool operator!=(const semantic_version a, const semantic_version b) noexcept {
	return a.as_tuple() != b.as_tuple();
}
constexpr bool operator<(const semantic_version a, const semantic_version b) noexcept {
	return a.as_tuple() < b.as_tuple();
}
constexpr bool operator>(const semantic_version a, const semantic_version b) noexcept {
	return a.as_tuple() > b.as_tuple();
}
constexpr bool operator<=(const semantic_version a, const semantic_version b) noexcept {
	return a.as_tuple() <= b.as_tuple();
}
constexpr bool operator>=(const semantic_version a, const semantic_version b) noexcept {
	return a.as_tuple() >= b.as_tuple();
}

struct version_info {
	semantic_version semver{};
	unsigned long long build = 0;
	constexpr std::tuple<std::tuple<unsigned long long, unsigned long long, unsigned long long>, unsigned long long> as_tuple() const noexcept {
		return std::make_tuple(semver.as_tuple(), build);
	}
	template <typename Tostream>
	friend Tostream & operator<<(Tostream & os, const version_info vi) {
		if (vi.build > 0) {
			os << vi.semver.major << "." << vi.semver.minor << "." << vi.semver.patch << "+build." << vi.build;
		} else {
			os << vi.semver.major << "." << vi.semver.minor << "." << vi.semver.patch;
		}
		return os;
	}
};

constexpr bool operator==(const version_info a, const version_info b) noexcept {
	return a.as_tuple() == b.as_tuple();
}
constexpr bool operator!=(const version_info a, const version_info b) noexcept {
	return a.as_tuple() != b.as_tuple();
}
constexpr bool operator<(const version_info a, const version_info b) noexcept {
	return a.as_tuple() < b.as_tuple();
}
constexpr bool operator>(const version_info a, const version_info b) noexcept {
	return a.as_tuple() > b.as_tuple();
}
constexpr bool operator<=(const version_info a, const version_info b) noexcept {
	return a.as_tuple() <= b.as_tuple();
}
constexpr bool operator>=(const version_info a, const version_info b) noexcept {
	return a.as_tuple() >= b.as_tuple();
}

constexpr inline version_info Version = {
	{MPT_VERSION_MAJOR, MPT_VERSION_MINOR, MPT_VERSION_PATCH},
	MPT_VERSION_BUILD
};


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_SEMANTIC_VERSION_HPP
