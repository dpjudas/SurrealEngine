/*
 * libopenmpt_test.cpp
 * -------------------
 * Purpose: libopenmpt test suite driver
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "openmpt/all/BuildSettings.hpp"
#include "openmpt/all/PlatformFixes.hpp"

#include "mpt/base/integer.hpp"
#include "mpt/main/main.hpp"

#include "../../libopenmpt/libopenmpt_internal.h"

#include "../../test/test.h"

#include <iostream>
#include <locale>

#include <clocale>
#include <cstdlib>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif /* __EMSCRIPTEN__ */

namespace libopenmpt_test {

static mpt::uint8 main() {

#if defined(__EMSCRIPTEN__)
	EM_ASM(
		FS.mkdir('/test');
		FS.mount(NODEFS, {'root': '../test/'}, '/test');
		FS.mkdir('/libopenmpt');
		FS.mount(NODEFS, {'root': '../libopenmpt/'}, '/libopenmpt');
	);
#endif /* __EMSCRIPTEN__ */

	try {

		using namespace OpenMPT;

		Test::PrintHeader();

		// run test with "C" / classic() locale
		Test::DoTests();

		// try setting the C locale to the user locale
		setlocale( LC_ALL, "" );
		
		// run all tests again with a set C locale
		Test::DoTests();
		
		// try to set the C and C++ locales to the user locale
		try {
			std::locale old = std::locale::global( std::locale( "" ) );
			static_cast<void>( old );
		} catch ( ... ) {
			// Setting c++ global locale does not work.
			// This is no problem for libopenmpt, just continue.
		}
		
		// and now, run all tests once again
		Test::DoTests();

		Test::PrintFooter();

	} catch ( const std::exception & e ) {
		std::cerr << "TEST ERROR: exception: " << ( e.what() ? e.what() : "" ) << std::endl;
		return 255;
	} catch ( ... ) {
		std::cerr << "TEST ERROR: unknown exception" << std::endl;
		return 255;
	}
	return 0;
}

} // namespace libopenmpt_test

MPT_MAIN_IMPLEMENT_MAIN_NO_ARGS(libopenmpt_test)
