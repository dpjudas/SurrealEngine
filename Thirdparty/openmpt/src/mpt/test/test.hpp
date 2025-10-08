/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_TEST_TEST_HPP
#define MPT_TEST_TEST_HPP



#include "mpt/base/namespace.hpp"
#include "mpt/base/source_location.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <variant>

#include <cstddef>
#include <cstdlib>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace test {



template <typename S, typename T, typename = void>
struct is_to_stream_writable : std::false_type { };

template <typename S, typename T>
struct is_to_stream_writable<S, T, std::void_t<decltype(std::declval<S &>() << std::declval<T>())>> : std::true_type { };

template <typename T>
inline auto format(const T & x) -> typename std::enable_if<mpt::test::is_to_stream_writable<std::ostringstream, T>::value, std::string>::type {
	std::ostringstream s;
	s << x;
	return std::move(s).str();
}

template <typename T>
inline auto format(const T & x) -> typename std::enable_if<!mpt::test::is_to_stream_writable<std::ostringstream, T>::value, std::string>::type {
	return typeid(x).name();
}

inline std::string get_exception_text() {
	std::string result;
	try {
		// cppcheck false-positive
		// cppcheck-suppress rethrowNoCurrentException
		throw;
	} catch (const std::exception & e) {
		result = e.what();
	} catch (...) {
		result = "unknown exception";
	}
	return result;
}

struct result_success {
};
struct result_failure {
	std::string text{};
};
struct result_unexpected_exception {
	std::string text{};
};

struct result {
	std::variant<std::monostate, result_success, result_failure, result_unexpected_exception> info{std::monostate{}};
};

struct statistics_counters {
	std::size_t total{0};
	std::size_t run{0};
	std::size_t successes{0};
	std::size_t failures{0};
	std::size_t unexpected_exceptions{0};
	std::size_t completed{0};
	constexpr statistics_counters & operator+=(const statistics_counters & other) noexcept {
		total += other.total;
		run += other.run;
		successes += other.successes;
		failures += other.failures;
		unexpected_exceptions += other.unexpected_exceptions;
		completed += other.completed;
		return *this;
	}
};

struct group_statistics {
	statistics_counters tests{};
	statistics_counters cases{};
	statistics_counters local_cases{};
};

struct global_statistics {
	statistics_counters groups{};
	statistics_counters tests{};
	statistics_counters cases{};
	std::map<std::string, group_statistics> individual_group_statistics{};
	explicit constexpr operator bool() noexcept {
		return succeeded();
	}
	constexpr bool operator!() noexcept {
		return failed();
	}
	constexpr bool succeeded() noexcept {
		return groups.successes == groups.run;
	}
	constexpr bool failed() noexcept {
		return groups.failures > 0 || groups.unexpected_exceptions > 0;
	}
};

class reporter_interface {
protected:
	virtual ~reporter_interface() = default;

public:
	virtual void run_begin(const mpt::source_location & loc) = 0;
	virtual void group_begin(const mpt::source_location & loc, const char * name) = 0;
	virtual void test_begin(const mpt::source_location & loc, const char * name) = 0;
	virtual void case_run(const mpt::source_location & loc) = 0;
	virtual void case_run(const mpt::source_location & loc, const char * text_e) = 0;
	virtual void case_run(const mpt::source_location & loc, const char * text_ex, const char * text_e) = 0;
	virtual void case_run(const mpt::source_location & loc, const char * text_a, const char * text_cmp, const char * text_b) = 0;
	virtual void case_result(const mpt::source_location & loc, const mpt::test::result & result) = 0;
	virtual void test_end(const mpt::source_location & loc, const char * name, const statistics_counters & counters) = 0;
	virtual void group_end(const mpt::source_location & loc, const char * name, const group_statistics & statistics) = 0;
	virtual void run_end(const mpt::source_location & loc, const global_statistics & statistics) = 0;
	virtual void immediate_breakpoint() = 0;
};

class silent_reporter
	: public reporter_interface {
public:
	silent_reporter() = default;
	~silent_reporter() override = default;

public:
	virtual void run_begin(const mpt::source_location &) override {
	}
	virtual void group_begin(const mpt::source_location &, const char *) override {
	}
	virtual void test_begin(const mpt::source_location &, const char *) override {
	}
	virtual void case_run(const mpt::source_location &) override {
	}
	virtual void case_run(const mpt::source_location &, const char *) override {
	}
	virtual void case_run(const mpt::source_location &, const char *, const char *) override {
	}
	virtual void case_run(const mpt::source_location &, const char *, const char *, const char *) override {
	}
	virtual void case_result(const mpt::source_location &, const mpt::test::result &) override {
	}
	virtual void test_end(const mpt::source_location &, const char *, const statistics_counters &) override {
	}
	virtual void group_end(const mpt::source_location &, const char *, const group_statistics &) override {
	}
	virtual void run_end(const mpt::source_location &, const global_statistics &) override {
	}
	virtual void immediate_breakpoint() override {
	}
};

class simple_reporter : public reporter_interface {
private:
	std::ostream & s;

public:
	simple_reporter(std::ostream & s_)
		: s(s_) {
		s.flush();
	}
	~simple_reporter() override {
		s.flush();
	}

public:
	void run_begin(const mpt::source_location & loc) override {
		static_cast<void>(loc);
		s << "Running test suite ..." << std::endl;
	}
	void group_begin(const mpt::source_location & loc, const char * name) override {
		static_cast<void>(loc);
		s << "Running group '" << name << "' ..." << std::endl;
	}
	void test_begin(const mpt::source_location & loc, const char * name) override {
		static_cast<void>(loc);
		s << " Running test '" << name << "' ..." << std::endl;
	}
	void case_run(const mpt::source_location & loc) override {
		static_cast<void>(loc);
		s << "  Checking ..." << std::endl;
	}
	void case_run(const mpt::source_location & loc, const char * text_e) override {
		static_cast<void>(loc);
		s << "  Checking '" << text_e << "' ..." << std::endl;
	}
	void case_run(const mpt::source_location & loc, const char * text_ex, const char * text_e) override {
		static_cast<void>(loc);
		if (text_ex) {
			s << "  Checking '" << text_e << " throws " << text_ex << "' ..." << std::endl;
		} else {
			s << "  Checking '" << text_e << " throws' ..." << std::endl;
		}
	}
	void case_run(const mpt::source_location & loc, const char * text_a, const char * text_cmp, const char * text_b) override {
		static_cast<void>(loc);
		s << "  Checking '" << text_a << " " << text_cmp << " " << text_b << "' ..." << std::endl;
	}
	void case_result(const mpt::source_location & loc, const mpt::test::result & result) override {
		static_cast<void>(loc);
		s << "  Checking done: ";
		if (std::holds_alternative<result_success>(result.info)) {
			s << "Success.";
		} else if (std::holds_alternative<result_failure>(result.info)) {
			s << "FAILURE: " << std::get<result_failure>(result.info).text;
		} else if (std::holds_alternative<result_unexpected_exception>(result.info)) {
			s << "UNEXPECTED EXCEPTION: " << std::get<result_unexpected_exception>(result.info).text;
		}
		s << std::endl;
	}
	void test_end(const mpt::source_location & loc, const char * name, const statistics_counters & counters) override {
		static_cast<void>(loc);
		static_cast<void>(counters);
		s << " Running test '" << name << "' done." << std::endl;
	}
	void group_end(const mpt::source_location & loc, const char * name, const group_statistics & statistics) override {
		static_cast<void>(loc);
		static_cast<void>(statistics);
		s << "Running group '" << name << "' done." << std::endl;
	}
	void run_end(const mpt::source_location & loc, const global_statistics & statistics) override {
		static_cast<void>(loc);
		s << "Running test suite done." << std::endl;
		s << "groups: " << statistics.groups.total << " | " << statistics.groups.successes << " passed";
		if (statistics.groups.failures || statistics.groups.unexpected_exceptions) {
			s << " | " << statistics.groups.failures << " FAILED";
			if (statistics.groups.unexpected_exceptions) {
				s << " | " << statistics.groups.unexpected_exceptions << " UNEXPECTED EXCEPTIONS";
			}
		}
		s << std::endl;
		s << "tests: " << statistics.tests.total << " | " << statistics.tests.successes << " passed";
		if (statistics.tests.failures || statistics.tests.unexpected_exceptions) {
			s << " | " << statistics.tests.failures << " FAILED";
			if (statistics.tests.unexpected_exceptions) {
				s << " | " << statistics.tests.unexpected_exceptions << " UNEXPECTED EXCEPTIONS";
			}
		}
		s << std::endl;
		s << "checks: " << statistics.cases.total << " | " << statistics.cases.successes << " passed";
		if (statistics.cases.failures || statistics.cases.unexpected_exceptions) {
			s << " | " << statistics.cases.failures << " FAILED";
			if (statistics.cases.unexpected_exceptions) {
				s << " | " << statistics.cases.unexpected_exceptions << " UNEXPECTED EXCEPTIONS";
			}
		}
		s << std::endl;
	}
	void immediate_breakpoint() override {
		return;
	}
};

struct group;

struct context {
	mpt::test::group & group;
	mpt::test::reporter_interface & reporter;
	mpt::test::group_statistics statistics{};
};

using void_context_function = void (*)(mpt::test::context &);

struct group {
	group * next{nullptr};
	const char * name{""};
	void_context_function func{nullptr};
	inline group(const char * name_, void_context_function f)
		: name(name_)
		, func(f) {
		next = group_list();
		group_list() = this;
	}
	group_statistics run(mpt::test::reporter_interface & reporter, const mpt::source_location & loc = mpt::source_location::current()) {
		mpt::test::context context{*this, reporter};
		context.reporter.group_begin(loc, name);
		if (func) {
			func(context);
		}
		context.reporter.group_end(loc, name, context.statistics);
		return context.statistics;
	}

public:
	[[nodiscard]] static inline group *& group_list() noexcept {
		static group * group_list = nullptr;
		return group_list;
	}
};

inline global_statistics run_all(mpt::test::reporter_interface & reporter, const mpt::source_location & loc = mpt::source_location::current()) {
	global_statistics statistics{};
	reporter.run_begin(loc);
	for (group * g = group::group_list(); g; g = g->next) {
		statistics.groups.total++;
		statistics.groups.run++;
		group_statistics s = g->run(reporter, loc);
		if (s.tests.unexpected_exceptions) {
			statistics.groups.unexpected_exceptions++;
		} else if (s.tests.failures) {
			statistics.groups.failures++;
		} else {
			statistics.groups.successes++;
		}
		statistics.tests += s.tests;
		statistics.cases += s.cases;
		statistics.groups.completed++;
		statistics.individual_group_statistics[g->name] = s;
	}
	reporter.run_end(loc, statistics);
	return statistics;
}

struct test {

	mpt::test::context & context;
	const char * name{""};
	mpt::source_location source_location{mpt::source_location::current()};
	void (*breakpoint)(void){nullptr};

	test(const test &) = delete;
	test & operator=(const test &) = delete;

	inline test(mpt::test::context & context_, void (*breakpoint_)(void) = nullptr, const mpt::source_location & source_location_ = mpt::source_location::current())
		: context(context_)
		, source_location(source_location_)
		, breakpoint(breakpoint_) {
		report_test_begin();
	}
	inline test(mpt::test::context & context_, const char * name_, void (*breakpoint_)(void) = nullptr, const mpt::source_location & source_location_ = mpt::source_location::current())
		: context(context_)
		, name(name_)
		, source_location(source_location_)
		, breakpoint(breakpoint_) {
		report_test_begin();
	}

	inline ~test() {
		report_test_end();
	}

	inline void immediate_breakpoint() {
		if (breakpoint) {
			breakpoint();
		} else {
			context.reporter.immediate_breakpoint();
		}
	}

	void report_test_begin() {
		context.statistics.tests.total++;
		context.statistics.tests.run++;
		context.statistics.local_cases = statistics_counters{};
		context.reporter.test_begin(source_location, name);
	}

	void report_run() {
		context.statistics.local_cases.total++;
		context.statistics.local_cases.run++;
		context.reporter.case_run(source_location);
	}
	void report_run(const char * text_e) {
		context.statistics.local_cases.total++;
		context.statistics.local_cases.run++;
		context.reporter.case_run(source_location, text_e);
	}
	void report_run(const char * text_ex, const char * text_e) {
		context.statistics.local_cases.total++;
		context.statistics.local_cases.run++;
		context.reporter.case_run(source_location, text_ex, text_e);
	}
	void report_run(const char * text_a, const char * text_cmp, const char * text_b) {
		context.statistics.local_cases.total++;
		context.statistics.local_cases.run++;
		context.reporter.case_run(source_location, text_a, text_cmp, text_b);
	}

	void report_result(mpt::test::result result) {
		if (std::holds_alternative<result_success>(result.info)) {
			context.statistics.local_cases.successes++;
		} else if (std::holds_alternative<result_failure>(result.info)) {
			context.statistics.local_cases.failures++;
		} else if (std::holds_alternative<result_unexpected_exception>(result.info)) {
			context.statistics.local_cases.unexpected_exceptions++;
		}
		context.statistics.local_cases.completed++;
		context.reporter.case_result(source_location, result);
	}

	void report_test_end() {
		context.statistics.cases += context.statistics.local_cases;
		if (context.statistics.local_cases.unexpected_exceptions) {
			context.statistics.tests.unexpected_exceptions++;
		} else if (context.statistics.local_cases.failures) {
			context.statistics.tests.failures++;
		} else {
			context.statistics.tests.successes++;
		}
		context.statistics.tests.completed++;
		context.reporter.test_end(source_location, name, context.statistics.local_cases);
	}

	template <typename Texception, typename Tcallable, typename std::enable_if<std::is_invocable<Tcallable>::value, bool>::type = true>
	inline test & expect_throws(Tcallable c, const char * text_ex = nullptr, const char * text_e = nullptr) {
		const std::type_info & tiexception = typeid(Texception);
		const std::type_info & tic = typeid(decltype(c()));
		report_run(text_ex ? text_ex : tiexception.name(), text_e ? text_e : tic.name());
		mpt::test::result result;
		try {
			c();
			immediate_breakpoint();
			result.info = mpt::test::result_failure{};
		} catch (const Texception &) {
			result.info = mpt::test::result_success{};
		} catch (...) {
			immediate_breakpoint();
			result.info = mpt::test::result_unexpected_exception{mpt::test::get_exception_text()};
		}
		report_result(result);
		return *this;
	}

	template <typename Tcallable, typename std::enable_if<std::is_invocable<Tcallable>::value, bool>::type = true>
	inline test & expect_throws_any(Tcallable c, const char * text_e = nullptr) {
		const std::type_info & tic = typeid(decltype(c()));
		report_run(nullptr, text_e ? text_e : tic.name());
		mpt::test::result result;
		try {
			c();
			immediate_breakpoint();
			result.info = mpt::test::result_failure{};
		} catch (...) {
			result.info = mpt::test::result_success{};
		}
		report_result(result);
		return *this;
	}

	template <typename Texpr, typename std::enable_if<std::is_invocable<Texpr>::value, bool>::type = true>
	inline test & expect(Texpr e, const char * text_e = nullptr) {
		const std::type_info & tie = typeid(decltype(std::invoke(e)));
		report_run(text_e ? text_e : tie.name());
		mpt::test::result result;
		try {
			const auto ve = std::invoke(e);
			if (!ve) {
				immediate_breakpoint();
				result.info = mpt::test::result_failure{/*mpt::test::format(ve)*/};
			} else {
				result.info = mpt::test::result_success{};
			}
		} catch (...) {
			immediate_breakpoint();
			result.info = mpt::test::result_unexpected_exception{mpt::test::get_exception_text()};
		}
		report_result(result);
		return *this;
	}

	template <typename Ta, typename Tcmp, typename Tb, typename std::enable_if<std::is_invocable<Ta>::value, bool>::type = true, typename std::enable_if<std::is_invocable<Tb>::value, bool>::type = true>
	inline test & expect(Ta && a, Tcmp cmp, Tb && b, const char * text_a = nullptr, const char * text_cmp = nullptr, const char * text_b = nullptr) {
		const std::type_info & tia = typeid(decltype(std::invoke(a)));
		const std::type_info & ticmp = typeid(decltype(cmp));
		const std::type_info & tib = typeid(decltype(std::invoke(b)));
		report_run(text_a ? text_a : tia.name(), text_cmp ? text_cmp : ticmp.name(), text_b ? text_b : tib.name());
		mpt::test::result result;
		try {
			const auto va = std::invoke(a);
			const auto vb = std::invoke(b);
			if (!cmp(va, vb)) {
				immediate_breakpoint();
				result.info = mpt::test::result_failure{mpt::test::format(va) + " " + mpt::test::format(cmp) + " " + mpt::test::format(vb)};
			} else {
				result.info = mpt::test::result_success{};
			}
		} catch (...) {
			immediate_breakpoint();
			result.info = mpt::test::result_unexpected_exception{mpt::test::get_exception_text()};
		}
		report_result(result);
		return *this;
	}

	template <typename Texpr, typename std::enable_if<!std::is_invocable<Texpr>::value, bool>::type = true>
	inline test & expect(Texpr && e, const char * text_e = nullptr) {
		const std::type_info & tie = typeid(decltype(std::forward<Texpr>(e)));
		report_run(text_e ? text_e : tie.name());
		mpt::test::result result;
		try {
			const auto ve = std::forward<Texpr>(e);
			if (!ve) {
				immediate_breakpoint();
				result.info = mpt::test::result_failure{/*mpt::test::format(ve)*/};
			} else {
				result.info = mpt::test::result_success{};
			}
		} catch (...) {
			immediate_breakpoint();
			result.info = mpt::test::result_unexpected_exception{mpt::test::get_exception_text()};
		}
		report_result(result);
		return *this;
	}

	template <typename Ta, typename Tcmp, typename Tb, typename std::enable_if<!std::is_invocable<Ta>::value, bool>::type = true, typename std::enable_if<!std::is_invocable<Tb>::value, bool>::type = true>
	inline test & expect(Ta && a, Tcmp cmp, Tb && b, const char * text_a = nullptr, const char * text_cmp = nullptr, const char * text_b = nullptr) {
		const std::type_info & tia = typeid(decltype(std::forward<Ta>(a)));
		const std::type_info & ticmp = typeid(decltype(cmp));
		const std::type_info & tib = typeid(decltype(std::forward<Tb>(b)));
		report_run(text_a ? text_a : tia.name(), text_cmp ? text_cmp : ticmp.name(), text_b ? text_b : tib.name());
		mpt::test::result result;
		try {
			const auto va = std::forward<Ta>(a);
			const auto vb = std::forward<Tb>(b);
			if (!cmp(va, vb)) {
				immediate_breakpoint();
				result.info = mpt::test::result_failure{mpt::test::format(va) + " " + mpt::test::format(cmp) + " " + mpt::test::format(vb)};
			} else {
				result.info = mpt::test::result_success{};
			}
		} catch (...) {
			immediate_breakpoint();
			result.info = mpt::test::result_unexpected_exception{mpt::test::get_exception_text()};
		}
		report_result(result);
		return *this;
	}
};



} // namespace test



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_TEST_TEST_HPP
