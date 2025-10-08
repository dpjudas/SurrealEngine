
#ifndef ASIO_ASIOVERSION_HPP
#define ASIO_ASIOVERSION_HPP



namespace ASIO {



#define ASIO_VERSION_MAJOR 2
#define ASIO_VERSION_MINOR 3
#define ASIO_VERSION_PATCH 3
#define ASIO_VERSION_BUILD 20190614



#define ASIO_MODERNSDK_VERSION_MAJOR 0
#define ASIO_MODERNSDK_VERSION_MINOR 12
#define ASIO_MODERNSDK_VERSION_PATCH 10
#define ASIO_MODERNSDK_VERSION_BUILD 0



#define ASIO_VERSION_BUILD_NAMESPACE_IMPL(a, b, c, d) v##a##_##b##_##c
#define ASIO_VERSION_BUILD_NAMESPACE(a, b, c, d)      ASIO_VERSION_BUILD_NAMESPACE_IMPL(a, b, c, d)



#define ASIO_VERSION_NAMESPACE ASIO_VERSION_BUILD_NAMESPACE(ASIO_MODERNSDK_VERSION_MAJOR, ASIO_MODERNSDK_VERSION_MINOR, ASIO_MODERNSDK_VERSION_PATCH, ASIO_MODERNSDK_VERSION_BUILD)



inline namespace ASIO_VERSION_NAMESPACE {



inline namespace Version {



struct SemanticVersion {
	unsigned long long                                                               Major = 0;
	unsigned long long                                                               Minor = 0;
	unsigned long long                                                               Patch = 0;
	constexpr std::tuple<unsigned long long, unsigned long long, unsigned long long> as_tuple() const noexcept {
		return std::make_tuple(Major, Minor, Patch);
	}
};

constexpr bool operator==(SemanticVersion a, SemanticVersion b) noexcept {
	return a.as_tuple() == b.as_tuple();
}
constexpr bool operator!=(SemanticVersion a, SemanticVersion b) noexcept {
	return a.as_tuple() != b.as_tuple();
}
constexpr bool operator<(SemanticVersion a, SemanticVersion b) noexcept {
	return a.as_tuple() < b.as_tuple();
}
constexpr bool operator>(SemanticVersion a, SemanticVersion b) noexcept {
	return a.as_tuple() > b.as_tuple();
}
constexpr bool operator<=(SemanticVersion a, SemanticVersion b) noexcept {
	return a.as_tuple() <= b.as_tuple();
}
constexpr bool operator>=(SemanticVersion a, SemanticVersion b) noexcept {
	return a.as_tuple() >= b.as_tuple();
}

struct VersionInfo {
	SemanticVersion                                                                                                  SemVer;
	unsigned long long                                                                                               Build = 0;
	constexpr std::tuple<std::tuple<unsigned long long, unsigned long long, unsigned long long>, unsigned long long> as_tuple() const noexcept {
		return std::make_tuple(SemVer.as_tuple(), Build);
	}
	template <typename Tostream>
	friend Tostream & operator<<(Tostream & os, VersionInfo vi) {
		if (vi.Build > 0) {
			os << vi.SemVer.Major << "." << vi.SemVer.Minor << "." << vi.SemVer.Patch << "+build." << vi.Build;
		} else {
			os << vi.SemVer.Major << "." << vi.SemVer.Minor << "." << vi.SemVer.Patch;
		}
		return os;
	}
};

constexpr bool operator==(VersionInfo a, VersionInfo b) noexcept {
	return a.as_tuple() == b.as_tuple();
}
constexpr bool operator!=(VersionInfo a, VersionInfo b) noexcept {
	return a.as_tuple() != b.as_tuple();
}
constexpr bool operator<(VersionInfo a, VersionInfo b) noexcept {
	return a.as_tuple() < b.as_tuple();
}
constexpr bool operator>(VersionInfo a, VersionInfo b) noexcept {
	return a.as_tuple() > b.as_tuple();
}
constexpr bool operator<=(VersionInfo a, VersionInfo b) noexcept {
	return a.as_tuple() <= b.as_tuple();
}
constexpr bool operator>=(VersionInfo a, VersionInfo b) noexcept {
	return a.as_tuple() >= b.as_tuple();
}



constexpr inline VersionInfo Version = {
	{ASIO_VERSION_MAJOR, ASIO_VERSION_MINOR, ASIO_VERSION_PATCH},
	ASIO_VERSION_BUILD
};

constexpr inline VersionInfo ModernSDKVersion = {
	{ASIO_MODERNSDK_VERSION_MAJOR, ASIO_MODERNSDK_VERSION_MINOR, ASIO_MODERNSDK_VERSION_PATCH},
	ASIO_MODERNSDK_VERSION_BUILD
};



} // namespace Version



} // namespace ASIO_VERSION_NAMESPACE



} // namespace ASIO



#endif // ASIO_ASIOVERSION_HPP
