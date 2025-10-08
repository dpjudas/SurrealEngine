/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_FORMAT_SIMPLE_SPEC_HPP
#define MPT_FORMAT_SIMPLE_SPEC_HPP



#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/string_transcode/transcode.hpp"

#include <utility>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {


struct format_simple_base {

	enum FormatFlagsEnum {
		BaseDec = 0x0001, // base 10 (integers only)        // int+float
		BaseHex = 0x0002, // base 16 (integers only)        // int+float
		CaseLow = 0x0010, // lower case hex digits          // int+float
		CaseUpp = 0x0020, // upper case hex digits          // int+float
		FillOff = 0x0100, // do not fill up width           // int+float
		FillNul = 0x0400, // fill up width with zeros       // int+float
		NotaNrm = 0x1000, // float: normal/default notation // float
		NotaFix = 0x2000, // float: fixed point notation    // float
		NotaSci = 0x4000, // float: scientific notation     // float
	};

}; // struct format_simple_base

using format_simple_flags = unsigned int;

static_assert(sizeof(format_simple_flags) >= sizeof(format_simple_base::FormatFlagsEnum));


template <typename Tstring>
class format_simple_spec {
private:
	format_simple_flags flags;
	std::size_t width;  // int+float
	int precision;      // float
	unsigned int group; // int
	Tstring group_sep;  // int
public:
	MPT_CONSTEXPRINLINE format_simple_spec() noexcept
		: flags(0)
		, width(0)
		, precision(-1)
		, group(0)
		, group_sep() { }
	MPT_CONSTEXPRINLINE format_simple_flags GetFlags() const noexcept {
		return flags;
	}
	MPT_CONSTEXPRINLINE std::size_t GetWidth() const noexcept {
		return width;
	}
	MPT_CONSTEXPRINLINE int GetPrecision() const noexcept {
		return precision;
	}
	MPT_CONSTEXPRINLINE unsigned int GetGroup() const noexcept {
		return group;
	}
	MPT_CONSTEXPRINLINE Tstring GetGroupSep() const {
		return group_sep;
	}
	MPT_CONSTEXPRINLINE format_simple_spec & SetFlags(format_simple_flags f) noexcept {
		flags = f;
		return *this;
	}
	MPT_CONSTEXPRINLINE format_simple_spec & SetWidth(std::size_t w) noexcept {
		width = w;
		return *this;
	}
	MPT_CONSTEXPRINLINE format_simple_spec & SetPrecision(int p) noexcept {
		precision = p;
		return *this;
	}
	MPT_CONSTEXPRINLINE format_simple_spec & SetGroup(unsigned int g) noexcept {
		group = g;
		return *this;
	}
	MPT_CONSTEXPRINLINE format_simple_spec & SetGroupSep(Tstring s) {
		group_sep = std::move(s);
		return *this;
	}

public:
	MPT_CONSTEXPRINLINE format_simple_spec & BaseDec() noexcept {
		flags &= ~(format_simple_base::BaseDec | format_simple_base::BaseHex);
		flags |= format_simple_base::BaseDec;
		return *this;
	}
	MPT_CONSTEXPRINLINE format_simple_spec & BaseHex() noexcept {
		flags &= ~(format_simple_base::BaseDec | format_simple_base::BaseHex);
		flags |= format_simple_base::BaseHex;
		return *this;
	}
	MPT_CONSTEXPRINLINE format_simple_spec & CaseLow() noexcept {
		flags &= ~(format_simple_base::CaseLow | format_simple_base::CaseUpp);
		flags |= format_simple_base::CaseLow;
		return *this;
	}
	MPT_CONSTEXPRINLINE format_simple_spec & CaseUpp() noexcept {
		flags &= ~(format_simple_base::CaseLow | format_simple_base::CaseUpp);
		flags |= format_simple_base::CaseUpp;
		return *this;
	}
	MPT_CONSTEXPRINLINE format_simple_spec & FillOff() noexcept {
		flags &= ~(format_simple_base::FillOff | format_simple_base::FillNul);
		flags |= format_simple_base::FillOff;
		return *this;
	}
	MPT_CONSTEXPRINLINE format_simple_spec & FillNul() noexcept {
		flags &= ~(format_simple_base::FillOff | format_simple_base::FillNul);
		flags |= format_simple_base::FillNul;
		return *this;
	}
	MPT_CONSTEXPRINLINE format_simple_spec & NotaNrm() noexcept {
		flags &= ~(format_simple_base::NotaNrm | format_simple_base::NotaFix | format_simple_base::NotaSci);
		flags |= format_simple_base::NotaNrm;
		return *this;
	}
	MPT_CONSTEXPRINLINE format_simple_spec & NotaFix() noexcept {
		flags &= ~(format_simple_base::NotaNrm | format_simple_base::NotaFix | format_simple_base::NotaSci);
		flags |= format_simple_base::NotaFix;
		return *this;
	}
	MPT_CONSTEXPRINLINE format_simple_spec & NotaSci() noexcept {
		flags &= ~(format_simple_base::NotaNrm | format_simple_base::NotaFix | format_simple_base::NotaSci);
		flags |= format_simple_base::NotaSci;
		return *this;
	}
	MPT_CONSTEXPRINLINE format_simple_spec & Width(std::size_t w) noexcept {
		width = w;
		return *this;
	}
	MPT_CONSTEXPRINLINE format_simple_spec & Prec(int p) noexcept {
		precision = p;
		return *this;
	}
	MPT_CONSTEXPRINLINE format_simple_spec & Group(unsigned int g) noexcept {
		group = g;
		return *this;
	}
	MPT_CONSTEXPRINLINE format_simple_spec & GroupSep(Tstring s) {
		group_sep = std::move(s);
		return *this;
	}

public:
	MPT_CONSTEXPRINLINE format_simple_spec & Dec() noexcept {
		return BaseDec();
	}
	MPT_CONSTEXPRINLINE format_simple_spec & Hex() noexcept {
		return BaseHex();
	}
	MPT_CONSTEXPRINLINE format_simple_spec & Low() noexcept {
		return CaseLow();
	}
	MPT_CONSTEXPRINLINE format_simple_spec & Upp() noexcept {
		return CaseUpp();
	}
	MPT_CONSTEXPRINLINE format_simple_spec & Off() noexcept {
		return FillOff();
	}
	MPT_CONSTEXPRINLINE format_simple_spec & Nul() noexcept {
		return FillNul();
	}
	MPT_CONSTEXPRINLINE format_simple_spec & Nrm() noexcept {
		return NotaNrm();
	}
	MPT_CONSTEXPRINLINE format_simple_spec & Fix() noexcept {
		return NotaFix();
	}
	MPT_CONSTEXPRINLINE format_simple_spec & Sci() noexcept {
		return NotaSci();
	}

public:
	MPT_CONSTEXPRINLINE format_simple_spec & Decimal() noexcept {
		return BaseDec();
	}
	MPT_CONSTEXPRINLINE format_simple_spec & Hexadecimal() noexcept {
		return BaseHex();
	}
	MPT_CONSTEXPRINLINE format_simple_spec & Lower() noexcept {
		return CaseLow();
	}
	MPT_CONSTEXPRINLINE format_simple_spec & Upper() noexcept {
		return CaseUpp();
	}
	MPT_CONSTEXPRINLINE format_simple_spec & FillNone() noexcept {
		return FillOff();
	}
	MPT_CONSTEXPRINLINE format_simple_spec & FillZero() noexcept {
		return FillNul();
	}
	MPT_CONSTEXPRINLINE format_simple_spec & FloatNormal() noexcept {
		return NotaNrm();
	}
	MPT_CONSTEXPRINLINE format_simple_spec & FloatFixed() noexcept {
		return NotaFix();
	}
	MPT_CONSTEXPRINLINE format_simple_spec & FloatScientific() noexcept {
		return NotaSci();
	}
	MPT_CONSTEXPRINLINE format_simple_spec & Precision(int p) noexcept {
		return Prec(p);
	}
};


template <typename Tdststring, typename Tsrcstring>
inline format_simple_spec<Tdststring> transcode_format_simple_spec(const format_simple_spec<Tsrcstring> & src) {
	format_simple_spec<Tdststring> dst;
	dst.SetFlags(src.GetFlags());
	dst.SetWidth(src.GetWidth());
	dst.SetPrecision(src.GetPrecision());
	dst.SetGroup(src.GetGroup());
	dst.SetGroupSep(mpt::transcode<Tdststring>(src.GetGroupSep()));
	return dst;
}

template <typename Tdststring, typename Tsrcstring, typename Tencoding, std::enable_if_t<std::is_same<Tdststring, std::string>::value, bool> = true>
inline format_simple_spec<Tdststring> transcode_format_simple_spec(Tencoding to, const format_simple_spec<Tsrcstring> & src) {
	format_simple_spec<Tdststring> dst;
	dst.SetFlags(src.GetFlags());
	dst.SetWidth(src.GetWidth());
	dst.SetPrecision(src.GetPrecision());
	dst.SetGroup(src.GetGroup());
	dst.SetGroupSep(mpt::transcode<Tdststring>(to, src.GetGroupSep()));
	return dst;
}

template <typename Tdststring, typename Tsrcstring, typename Tencoding, std::enable_if_t<std::is_same<typename mpt::make_string_type<Tsrcstring>::type, std::string>::value, bool> = true>
inline format_simple_spec<Tdststring> transcode_format_simple_spec(Tencoding from, const format_simple_spec<Tsrcstring> & src) {
	format_simple_spec<Tdststring> dst;
	dst.SetFlags(src.GetFlags());
	dst.SetWidth(src.GetWidth());
	dst.SetPrecision(src.GetPrecision());
	dst.SetGroup(src.GetGroup());
	dst.SetGroupSep(mpt::transcode<Tdststring>(from, src.GetGroupSep()));
	return dst;
}

template <typename Tdststring, typename Tsrcstring, typename Tto, typename Tfrom>
inline format_simple_spec<Tdststring> transcode_format_simple_spec(Tto to, Tfrom from, const format_simple_spec<Tsrcstring> & src) {
	format_simple_spec<Tdststring> dst;
	dst.SetFlags(src.GetFlags());
	dst.SetWidth(src.GetWidth());
	dst.SetPrecision(src.GetPrecision());
	dst.SetGroup(src.GetGroup());
	dst.SetGroupSep(mpt::transcode<Tdststring>(to, from, src.GetGroupSep()));
	return dst;
}




} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_FORMAT_SIMPLE_SPEC_HPP
