/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_EXCEPTION_EXCEPTION_HPP
#define MPT_EXCEPTION_EXCEPTION_HPP



#include "mpt/base/namespace.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"



namespace mpt {
inline namespace MPT_INLINE_NS {



class exception_ustring_wrapper_base {
protected:
	exception_ustring_wrapper_base() = default;
	virtual ~exception_ustring_wrapper_base() = default;
public:
	virtual mpt::ustring uwhat() const = 0;
};

template <typename T>
class exception_ustring_wrapper : public T
	, public virtual mpt::exception_ustring_wrapper_base {
private:
	mpt::ustring m_what;
public:
	exception_ustring_wrapper(mpt::ustring str)
		: T(mpt::transcode<std::string>(mpt::exception_encoding, str))
		, m_what(str) {
		return;
	}
	~exception_ustring_wrapper() override = default;
	mpt::ustring uwhat() const override {
		return m_what;
	}
};



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_EXCEPTION_EXCEPTION_HPP
