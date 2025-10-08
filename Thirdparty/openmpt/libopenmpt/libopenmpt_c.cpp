/*
 * libopenmpt_c.cpp
 * ----------------
 * Purpose: libopenmpt C interface implementation
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "openmpt/all/BuildSettings.hpp"

#if defined(__MINGW32__) && !defined(__MINGW64__)
#include <sys/types.h>
#endif

#include "libopenmpt_internal.h"
#include "libopenmpt.h"
#include "libopenmpt_ext.h"

#include "libopenmpt_impl.hpp"
#include "libopenmpt_ext_impl.hpp"

#include <limits>
#include <new>
#include <stdexcept>
#include <string_view>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(_MSC_VER)
#pragma warning(disable:4702) /* unreachable code */
#endif

namespace openmpt {

static const char * strdup( const char * src ) {
	char * dst = (char*)std::calloc( std::strlen( src ) + 1, sizeof( char ) );
	if ( !dst ) {
		return NULL;
	}
	std::strcpy( dst, src );
	return dst;
}

class logfunc_logger : public log_interface {
private:
	openmpt_log_func m_logfunc;
	void * m_user;
public:
	logfunc_logger( openmpt_log_func func, void * user ) : m_logfunc(func), m_user(user) {
		return;
	}
	void log( const std::string & message ) const override  {
		if ( m_logfunc ) {
			m_logfunc( message.c_str(), m_user );
		} else {
			openmpt_log_func_default( message.c_str(), m_user );
		}
	}
}; // class logfunc_logger

namespace interface {

class invalid_module_pointer : public openmpt::exception {
public:
	invalid_module_pointer()
		: openmpt::exception("module * not valid")
	{
		return;
	}
	invalid_module_pointer(const invalid_module_pointer&) = default;
	virtual ~invalid_module_pointer() noexcept = default;
};

class argument_null_pointer : public openmpt::exception {
public:
	argument_null_pointer()
		: openmpt::exception("argument null pointer")
	{
		return;
	}
	argument_null_pointer(const argument_null_pointer&) = default;
	virtual ~argument_null_pointer() noexcept = default;
};

} // namespace interface

static std::string format_exception( const char * const function ) {
	std::string err;
	try {
		// cppcheck false-positive
		// cppcheck-suppress rethrowNoCurrentException
		throw;
	} catch ( const openmpt::exception & e ) {
		err += function;
		err += ": ";
		err += "ERROR: ";
		const char * what = e.what();
		err += what ? what : "";
	} catch ( const std::bad_alloc & e ) {
		err += function;
		err += ": ";
		err += "OUT OF MEMORY: ";
		const char * what = e.what();
		err += what ? what : "";
	} catch ( const std::exception & e ) {
		err += function;
		err += ": ";
		err += "INTERNAL ERROR: ";
		const char * what = e.what();
		err += what ? what : "";
	} catch ( ... ) {
		err += function;
		err += ": ";
		err += "UNKNOWN INTERNAL ERROR";
	}
	return err;
}

static void error_message_from_exception( const char * * error_message, const std::exception & e ) {
	if ( error_message ) {
		const char * what = e.what();
		*error_message = ( what ? openmpt::strdup( what ) : openmpt::strdup( "" ) );
	}
}

static int error_from_exception( const char * * error_message ) {
	int error = 0;
	if ( error_message ) {
		if ( *error_message ) {
			openmpt_free_string( *error_message );
			*error_message = NULL;
		}
	}
	try {
		// cppcheck false-positive
		// cppcheck-suppress rethrowNoCurrentException
		throw;

	} catch ( const std::bad_alloc & e ) {
		error = OPENMPT_ERROR_OUT_OF_MEMORY;
		error_message_from_exception( error_message, e );

	} catch ( const openmpt::interface::invalid_module_pointer & e ) {
		error = OPENMPT_ERROR_INVALID_MODULE_POINTER;
		error_message_from_exception( error_message, e );
	} catch ( const openmpt::interface::argument_null_pointer & e ) {
		error = OPENMPT_ERROR_ARGUMENT_NULL_POINTER;
		error_message_from_exception( error_message, e );
	} catch ( const openmpt::exception & e ) {
		error = OPENMPT_ERROR_GENERAL;
		error_message_from_exception( error_message, e );

	} catch ( const std::invalid_argument & e ) {
		error = OPENMPT_ERROR_INVALID_ARGUMENT;
		error_message_from_exception( error_message, e );
	} catch ( const std::out_of_range & e ) {
		error = OPENMPT_ERROR_OUT_OF_RANGE;
		error_message_from_exception( error_message, e );
	} catch ( const std::length_error & e ) {
		error = OPENMPT_ERROR_LENGTH;
		error_message_from_exception( error_message, e );
	} catch ( const std::domain_error & e ) {
		error = OPENMPT_ERROR_DOMAIN;
		error_message_from_exception( error_message, e );
	} catch ( const std::logic_error & e ) {
		error = OPENMPT_ERROR_LOGIC;
		error_message_from_exception( error_message, e );

	} catch ( const std::underflow_error & e ) {
		error = OPENMPT_ERROR_UNDERFLOW;
		error_message_from_exception( error_message, e );
	} catch ( const std::overflow_error & e ) {
		error = OPENMPT_ERROR_OVERFLOW;
		error_message_from_exception( error_message, e );
	} catch ( const std::range_error & e ) {
		error = OPENMPT_ERROR_RANGE;
		error_message_from_exception( error_message, e );
	} catch ( const std::runtime_error & e ) {
		error = OPENMPT_ERROR_RUNTIME;
		error_message_from_exception( error_message, e );

	} catch ( const std::exception & e ) {
		error = OPENMPT_ERROR_EXCEPTION;
		error_message_from_exception( error_message, e );

	} catch ( ... ) {
		error = OPENMPT_ERROR_UNKNOWN;

	}
	return error;
}

} // namespace openmpt

extern "C" {

struct openmpt_module {
	openmpt_log_func logfunc;
	void * loguser;
	openmpt_error_func errfunc;
	void * erruser;
	int error;
	const char * error_message;
	openmpt::module_impl * impl;
};

struct openmpt_module_ext {
	openmpt_module mod;
	openmpt::module_ext_impl * impl;
};

} // extern "C"

namespace openmpt {

static void do_report_exception( const char * const function, openmpt_log_func const logfunc = 0, void * const loguser = 0, openmpt_error_func errfunc = 0, void * const erruser = 0, openmpt::module_impl * const impl = 0, openmpt_module * const mod = 0, int * const err = 0, const char * * err_msg = 0 ) {
	int error = OPENMPT_ERROR_OK;
	const char * error_message = NULL;
	int error_func_result = OPENMPT_ERROR_FUNC_RESULT_DEFAULT;
	if ( errfunc || mod || err || err_msg ) {
		error = error_from_exception( mod ? &error_message : NULL );
	}
	if ( errfunc ) {
		error_func_result = errfunc( error, erruser );
	}
	if ( mod && ( error_func_result & OPENMPT_ERROR_FUNC_RESULT_STORE ) ) {
		mod->error = error;
		mod->error_message = ( error_message ? openmpt::strdup( error_message ) : openmpt::strdup( "" ) );
	}
	if ( err ) {
		*err = error;
	}
	if ( err_msg ) {
		*err_msg = ( error_message ? openmpt::strdup( error_message ) : openmpt::strdup( "" ) );
	}
	if ( error_message ) {
		openmpt_free_string( error_message );
		error_message = NULL;
	}
	if ( error_func_result & OPENMPT_ERROR_FUNC_RESULT_LOG ) {
		try {
			const std::string message = format_exception( function );
			if ( impl ) {
				impl->PushToCSoundFileLog( message );
			} else if ( logfunc ) {
				logfunc( message.c_str(), loguser );
			} else {
				openmpt_log_func_default( message.c_str(), NULL );
			}
		} catch ( ... ) {
			fprintf( stderr, "openmpt: %s:%i: UNKNOWN INTERNAL ERROR in error handling: function='%s', logfunc=%p, loguser=%p, errfunc=%p, erruser=%p, impl=%p\n", __FILE__, static_cast<int>( __LINE__ ), function ? function : "", reinterpret_cast<void*>( logfunc ), loguser, reinterpret_cast<void*>( errfunc ), erruser, static_cast<void*>( impl ) );
			fflush( stderr );
		}
	}
}

static void report_exception( const char * const function, openmpt_module * mod = 0, int * error = 0, const char * * error_message = 0 ) {
	do_report_exception( function, mod ? mod->logfunc : NULL, mod ? mod->loguser : NULL, mod ? mod->errfunc : NULL, mod ? mod->erruser : NULL, mod ? mod->impl : 0, mod ? mod : NULL, error ? error : NULL, error_message ? error_message : NULL );
}

static void report_exception( const char * const function, openmpt_log_func const logfunc, void * const loguser, openmpt_error_func errfunc, void * const erruser, int * error, const char * * error_message ) {
	do_report_exception( function, logfunc, loguser, errfunc, erruser, 0, 0, error, error_message );
}

namespace interface {

template < typename T >
void check_soundfile( T * mod ) {
	if ( !mod ) {
		throw openmpt::interface::invalid_module_pointer();
	}
}

template < typename T >
void check_pointer( T * p ) {
	if ( !p ) {
		throw openmpt::interface::argument_null_pointer();
	}
}

} // namespace interface

} // namespace openmpt

extern "C" {

uint32_t openmpt_get_library_version(void) {
	try {
		return openmpt::get_library_version();
	} catch ( ... ) {
		openmpt::report_exception( __func__ );
	}
	return 0;
}

uint32_t openmpt_get_core_version(void) {
	try {
		return openmpt::get_core_version();
	} catch ( ... ) {
		openmpt::report_exception( __func__ );
	}
	return 0;
}

void openmpt_free_string( const char * str ) {
	try {
		std::free( const_cast< char * >( str ) );
	} catch ( ... ) {
		openmpt::report_exception( __func__ );
	}
	return;
}

const char * openmpt_get_string( const char * key ) {
	try {
		if ( !key ) {
			return openmpt::strdup( "" );
		}
		return openmpt::strdup( openmpt::string::get( key ).c_str() );
	} catch ( ... ) {
		openmpt::report_exception( __func__ );
	}
	return NULL;
}

const char * openmpt_get_supported_extensions(void) {
	try {
		std::string retval;
		bool first = true;
		std::vector<std::string> supported_extensions = openmpt::module_impl::get_supported_extensions();
		for ( std::vector<std::string>::iterator i = supported_extensions.begin(); i != supported_extensions.end(); ++i ) {
			if ( first ) {
				first = false;
			} else {
				retval += ";";
			}
			retval += *i;
		}
		return openmpt::strdup( retval.c_str() );
	} catch ( ... ) {
		openmpt::report_exception( __func__ );
	}
	return NULL;
}

int openmpt_is_extension_supported( const char * extension ) {
	try {
		if ( !extension ) {
			return 0;
		}
		return openmpt::module_impl::is_extension_supported( extension ) ? 1 : 0;
	} catch ( ... ) {
		openmpt::report_exception( __func__ );
	}
	return 0;
}

void openmpt_log_func_default( const char * message, void * /*user*/ ) {
	fprintf( stderr, "openmpt: %s\n", message );
	fflush( stderr );
}

void openmpt_log_func_silent( const char * /*message*/ , void * /*user*/ ) {
	return;
}

int openmpt_error_is_transient( int error ) {
	int result = 0;
	switch ( error ) {
		case OPENMPT_ERROR_OUT_OF_MEMORY:
			result = 1;
			break;
		default:
			result = 0;
			break;
	}
	return result;
}

const char * openmpt_error_string( int error ) {
	const char * text = "unknown error";
	switch ( error ) {
		case OPENMPT_ERROR_OK:                
			text = "";
			break;
		case OPENMPT_ERROR_UNKNOWN:
			text = "unknown internal error";
			break;
		case OPENMPT_ERROR_EXCEPTION:
			text = "unknown exception ";
			break;
		case OPENMPT_ERROR_OUT_OF_MEMORY:
			text = "out of memory";
			break;
		case OPENMPT_ERROR_RUNTIME:
			text = "runtime error";
			break;
		case OPENMPT_ERROR_RANGE:
			text = "range error";
			break;
		case OPENMPT_ERROR_OVERFLOW:
			text = "arithmetic overflow";
			break;
		case OPENMPT_ERROR_UNDERFLOW:
			text = "arithmetic underflow";
			break;
		case OPENMPT_ERROR_LOGIC:
			text = "logic error";
			break;
		case OPENMPT_ERROR_DOMAIN:
			text = "value domain error";
			break;
		case OPENMPT_ERROR_LENGTH:
			text = "maximum supported size exceeded";
			break;
		case OPENMPT_ERROR_OUT_OF_RANGE:
			text = "argument out of range";
			break;
		case OPENMPT_ERROR_INVALID_ARGUMENT:
			text = "invalid argument";
			break;
		case OPENMPT_ERROR_GENERAL:
			text = "libopenmpt error";
			break;
	}
	return openmpt::strdup( text );
}

int openmpt_error_func_default( int error, void * /* user */ ) {
	(void)error;
	return OPENMPT_ERROR_FUNC_RESULT_DEFAULT;
}

int openmpt_error_func_log( int error, void * /* user */ ) {
	(void)error;
	return OPENMPT_ERROR_FUNC_RESULT_LOG;
}

int openmpt_error_func_store( int error, void * /* user */ ) {
	(void)error;
	return OPENMPT_ERROR_FUNC_RESULT_STORE;
}

int openmpt_error_func_ignore( int error, void * /* user */ ) {
	(void)error;
	return OPENMPT_ERROR_FUNC_RESULT_NONE;
}

int openmpt_error_func_errno( int error, void * user ) {
	int * e = (int *)user;
	if ( !e ) {
		return OPENMPT_ERROR_FUNC_RESULT_DEFAULT;
	}
	*e = error;
	return OPENMPT_ERROR_FUNC_RESULT_NONE;
}

void * openmpt_error_func_errno_userdata( int * error ) {
	return (void *)error;
}

double openmpt_could_open_probability( openmpt_stream_callbacks stream_callbacks, void * stream, double effort, openmpt_log_func logfunc, void * loguser ) {
	return openmpt_could_open_probability2( stream_callbacks, stream, effort, logfunc, loguser, NULL, NULL, NULL, NULL );
}
double openmpt_could_open_propability( openmpt_stream_callbacks stream_callbacks, void * stream, double effort, openmpt_log_func logfunc, void * loguser ) {
	return openmpt_could_open_probability2( stream_callbacks, stream, effort, logfunc, loguser, NULL, NULL, NULL, NULL );
}

double openmpt_could_open_probability2( openmpt_stream_callbacks stream_callbacks, void * stream, double effort, openmpt_log_func logfunc, void * loguser, openmpt_error_func errfunc, void * erruser, int * error, const char * * error_message ) {
	try {
		openmpt::callback_stream_wrapper istream = { stream, stream_callbacks.read, stream_callbacks.seek, stream_callbacks.tell };
		return openmpt::module_impl::could_open_probability( istream, effort, openmpt::helper::make_unique<openmpt::logfunc_logger>( logfunc ? logfunc : openmpt_log_func_default, loguser ) );
	} catch ( ... ) {
		openmpt::report_exception( __func__, logfunc, loguser, errfunc, erruser, error, error_message );
	}
	return 0.0;
}

size_t openmpt_probe_file_header_get_recommended_size(void) {
	try {
		return openmpt::module_impl::probe_file_header_get_recommended_size();
	} catch ( ... ) {
		openmpt::report_exception( __func__ );
	}
	return 0;
}

int openmpt_probe_file_header( uint64_t flags, const void * data, size_t size, uint64_t filesize, openmpt_log_func logfunc, void * loguser, openmpt_error_func errfunc, void * erruser, int * error, const char * * error_message ) {
	try {
		return openmpt::module_impl::probe_file_header( flags, data, size, filesize );
	} catch ( ... ) {
		openmpt::report_exception( __func__, logfunc, loguser, errfunc, erruser, error, error_message );
	}
	return OPENMPT_PROBE_FILE_HEADER_RESULT_ERROR;
}

int openmpt_probe_file_header_without_filesize( uint64_t flags, const void * data, size_t size, openmpt_log_func logfunc, void * loguser, openmpt_error_func errfunc, void * erruser, int * error, const char * * error_message ) {
	try {
		return openmpt::module_impl::probe_file_header( flags, data, size );
	} catch ( ... ) {
		openmpt::report_exception( __func__, logfunc, loguser, errfunc, erruser, error, error_message );
	}
	return OPENMPT_PROBE_FILE_HEADER_RESULT_ERROR;
}

int openmpt_probe_file_header_from_stream( uint64_t flags, openmpt_stream_callbacks stream_callbacks, void * stream, openmpt_log_func logfunc, void * loguser, openmpt_error_func errfunc, void * erruser, int * error, const char * * error_message ) {
	try {
		openmpt::callback_stream_wrapper istream = { stream, stream_callbacks.read, stream_callbacks.seek, stream_callbacks.tell };
		return openmpt::module_impl::probe_file_header( flags, istream );
	} catch ( ... ) {
		openmpt::report_exception( __func__, logfunc, loguser, errfunc, erruser, error, error_message );
	}
	return OPENMPT_PROBE_FILE_HEADER_RESULT_ERROR;
}

openmpt_module * openmpt_module_create( openmpt_stream_callbacks stream_callbacks, void * stream, openmpt_log_func logfunc, void * user, const openmpt_module_initial_ctl * ctls ) {
	return openmpt_module_create2( stream_callbacks, stream, logfunc, user, NULL, NULL, NULL, NULL, ctls );
}

openmpt_module * openmpt_module_create2( openmpt_stream_callbacks stream_callbacks, void * stream, openmpt_log_func logfunc, void * loguser, openmpt_error_func errfunc, void * erruser, int * error, const char * * error_message, const openmpt_module_initial_ctl * ctls ) {
	try {
		openmpt_module * mod = (openmpt_module*)std::calloc( 1, sizeof( openmpt_module ) );
		if ( !mod ) {
			throw std::bad_alloc();
		}
		std::memset( mod, 0, sizeof( openmpt_module ) );
		mod->logfunc = logfunc ? logfunc : openmpt_log_func_default;
		mod->loguser = loguser;
		mod->errfunc = errfunc ? errfunc : NULL;
		mod->erruser = erruser;
		mod->error = OPENMPT_ERROR_OK;
		mod->error_message = NULL;
		mod->impl = 0;
		try {
			std::map< std::string, std::string > ctls_map;
			if ( ctls ) {
				for ( const openmpt_module_initial_ctl * it = ctls; it->ctl; ++it ) {
					if ( it->value ) {
						ctls_map[ it->ctl ] = it->value;
					} else {
						ctls_map.erase( it->ctl );
					}
				}
			}
			openmpt::callback_stream_wrapper istream = { stream, stream_callbacks.read, stream_callbacks.seek, stream_callbacks.tell };
			mod->impl = new openmpt::module_impl( istream, openmpt::helper::make_unique<openmpt::logfunc_logger>( mod->logfunc, mod->loguser ), ctls_map );
			return mod;
		} catch ( ... ) {
			#if defined(_MSC_VER)
			#pragma warning(push)
			#pragma warning(disable:6001) // false-positive: Using uninitialized memory 'mod'.
			#endif // _MSC_VER
				openmpt::report_exception( __func__, mod, error, error_message );
			#if defined(_MSC_VER)
			#pragma warning(pop)
			#endif // _MSC_VER
		}
		delete mod->impl;
		mod->impl = 0;
		if ( mod->error_message ) {
			openmpt_free_string( mod->error_message );
			mod->error_message = NULL;
		}
		std::free( (void*)mod );
		mod = NULL;
	} catch ( ... ) {
		openmpt::report_exception( __func__, 0, error, error_message );
	}
	return NULL;
}

openmpt_module * openmpt_module_create_from_memory( const void * filedata, size_t filesize, openmpt_log_func logfunc, void * user, const openmpt_module_initial_ctl * ctls ) {
	return openmpt_module_create_from_memory2( filedata, filesize, logfunc, user, NULL, NULL, NULL, NULL, ctls );
}

openmpt_module * openmpt_module_create_from_memory2( const void * filedata, size_t filesize, openmpt_log_func logfunc, void * loguser, openmpt_error_func errfunc, void * erruser, int * error, const char * * error_message, const openmpt_module_initial_ctl * ctls ) {
	try {
		openmpt_module * mod = (openmpt_module*)std::calloc( 1, sizeof( openmpt_module ) );
		if ( !mod ) {
			throw std::bad_alloc();
		}
		std::memset( mod, 0, sizeof( openmpt_module ) );
		mod->logfunc = logfunc ? logfunc : openmpt_log_func_default;
		mod->loguser = loguser;
		mod->errfunc = errfunc ? errfunc : NULL;
		mod->erruser = erruser;
		mod->error = OPENMPT_ERROR_OK;
		mod->error_message = NULL;
		mod->impl = 0;
		try {
			std::map< std::string, std::string > ctls_map;
			if ( ctls ) {
				for ( const openmpt_module_initial_ctl * it = ctls; it->ctl; ++it ) {
					if ( it->value ) {
						ctls_map[ it->ctl ] = it->value;
					} else {
						ctls_map.erase( it->ctl );
					}
				}
			}
			mod->impl = new openmpt::module_impl( filedata, filesize, openmpt::helper::make_unique<openmpt::logfunc_logger>( mod->logfunc, mod->loguser ), ctls_map );
			return mod;
		} catch ( ... ) {
			#if defined(_MSC_VER)
			#pragma warning(push)
			#pragma warning(disable:6001) // false-positive: Using uninitialized memory 'mod'.
			#endif // _MSC_VER
				openmpt::report_exception( __func__, mod, error, error_message );
			#if defined(_MSC_VER)
			#pragma warning(pop)
			#endif // _MSC_VER
		}
		delete mod->impl;
		mod->impl = 0;
		if ( mod->error_message ) {
			openmpt_free_string( mod->error_message );
			mod->error_message = NULL;
		}
		std::free( (void*)mod );
		mod = NULL;
	} catch ( ... ) {
		openmpt::report_exception( __func__, 0, error, error_message );
	}
	return NULL;
}

void openmpt_module_destroy( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		delete mod->impl;
		mod->impl = 0;
		if ( mod->error_message ) {
			openmpt_free_string( mod->error_message );
			mod->error_message = NULL;
		}
		std::free( (void*)mod );
		mod = NULL;
		return;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return;
}

void openmpt_module_set_log_func( openmpt_module * mod, openmpt_log_func logfunc, void * loguser ) {
	try {
		openmpt::interface::check_soundfile( mod );
		mod->logfunc = logfunc ? logfunc : openmpt_log_func_default;
		mod->loguser = loguser;
		return;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return;
}

void openmpt_module_set_error_func( openmpt_module * mod, openmpt_error_func errfunc, void * erruser ) {
	try {
		openmpt::interface::check_soundfile( mod );
		mod->errfunc = errfunc ? errfunc : NULL;
		mod->erruser = erruser;
		mod->error = OPENMPT_ERROR_OK;
		return;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return;
}

int openmpt_module_error_get_last( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->error;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return -1;
}

const char * openmpt_module_error_get_last_message( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->error_message ? openmpt::strdup( mod->error_message ) : openmpt::strdup( "" );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return NULL;
}

void openmpt_module_error_set_last( openmpt_module * mod, int error ) {
	try {
		openmpt::interface::check_soundfile( mod );
		mod->error = error;
		if ( mod->error_message ) {
			openmpt_free_string( mod->error_message );
			mod->error_message = NULL;
		}
		return;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return;
}

void openmpt_module_error_clear( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		mod->error = OPENMPT_ERROR_OK;
		if ( mod->error_message ) {
			openmpt_free_string( mod->error_message );
			mod->error_message = NULL;
		}
		return;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return;
}

int openmpt_module_select_subsong( openmpt_module * mod, int32_t subsong ) {
	try {
		openmpt::interface::check_soundfile( mod );
		mod->impl->select_subsong( subsong );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}

int32_t openmpt_module_get_selected_subsong( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_selected_subsong();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return -1;
}

int32_t openmpt_module_get_restart_order( openmpt_module * mod, int32_t subsong ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_restart_order( subsong );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return -1;
}

int32_t openmpt_module_get_restart_row( openmpt_module * mod, int32_t subsong ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_restart_row( subsong );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return -1;
}

int openmpt_module_set_repeat_count( openmpt_module * mod, int32_t repeat_count ) {
	try {
		openmpt::interface::check_soundfile( mod );
		mod->impl->set_repeat_count( repeat_count );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int32_t openmpt_module_get_repeat_count( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_repeat_count();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}

double openmpt_module_get_duration_seconds( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_duration_seconds();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0.0;
}

double openmpt_module_get_time_at_position( openmpt_module * mod, int32_t order, int32_t row ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_time_at_position( order, row );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return -1.0;
}

double openmpt_module_set_position_seconds( openmpt_module * mod, double seconds ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->set_position_seconds( seconds );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0.0;
}
double openmpt_module_get_position_seconds( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_position_seconds();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0.0;
}

double openmpt_module_set_position_order_row( openmpt_module * mod, int32_t order, int32_t row ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->set_position_order_row( order, row );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0.0;
}

int openmpt_module_get_render_param( openmpt_module * mod, int param, int32_t * value ) {
	try {
		openmpt::interface::check_soundfile( mod );
		openmpt::interface::check_pointer( value );
		*value = mod->impl->get_render_param( (openmpt::module::render_param)param );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int openmpt_module_set_render_param( openmpt_module * mod, int param, int32_t value ) {
	try {
		openmpt::interface::check_soundfile( mod );
		mod->impl->set_render_param( (openmpt::module::render_param)param, value );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}

size_t openmpt_module_read_mono( openmpt_module * mod, int32_t samplerate, size_t count, int16_t * mono ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->read( samplerate, count, mono );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
size_t openmpt_module_read_stereo( openmpt_module * mod, int32_t samplerate, size_t count, int16_t * left, int16_t * right ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->read( samplerate, count, left, right );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
size_t openmpt_module_read_quad( openmpt_module * mod, int32_t samplerate, size_t count, int16_t * left, int16_t * right, int16_t * rear_left, int16_t * rear_right ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->read( samplerate, count, left, right, rear_left, rear_right );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
size_t openmpt_module_read_float_mono( openmpt_module * mod, int32_t samplerate, size_t count, float * mono ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->read( samplerate, count, mono );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
size_t openmpt_module_read_float_stereo( openmpt_module * mod, int32_t samplerate, size_t count, float * left, float * right ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->read( samplerate, count, left, right );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
size_t openmpt_module_read_float_quad( openmpt_module * mod, int32_t samplerate, size_t count, float * left, float * right, float * rear_left, float * rear_right ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->read( samplerate, count, left, right, rear_left, rear_right );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
size_t openmpt_module_read_interleaved_stereo( openmpt_module * mod, int32_t samplerate, size_t count, int16_t * interleaved_stereo ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->read_interleaved_stereo( samplerate, count, interleaved_stereo );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
size_t openmpt_module_read_interleaved_quad( openmpt_module * mod, int32_t samplerate, size_t count, int16_t * interleaved_quad ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->read_interleaved_quad( samplerate, count, interleaved_quad );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
size_t openmpt_module_read_interleaved_float_stereo( openmpt_module * mod, int32_t samplerate, size_t count, float * interleaved_stereo ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->read_interleaved_stereo( samplerate, count, interleaved_stereo );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
size_t openmpt_module_read_interleaved_float_quad( openmpt_module * mod, int32_t samplerate, size_t count, float * interleaved_quad ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->read_interleaved_quad( samplerate, count, interleaved_quad );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}

const char * openmpt_module_get_metadata_keys( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		std::string retval;
		bool first = true;
		std::vector<std::string> metadata_keys = mod->impl->get_metadata_keys();
		for ( std::vector<std::string>::iterator i = metadata_keys.begin(); i != metadata_keys.end(); ++i ) {
			if ( first ) {
				first = false;
			} else {
				retval += ";";
			}
			retval += *i;
		}
		return openmpt::strdup( retval.c_str() );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return NULL;
}
const char * openmpt_module_get_metadata( openmpt_module * mod, const char * key ) {
	try {
		openmpt::interface::check_soundfile( mod );
		openmpt::interface::check_pointer( key );
		return openmpt::strdup( mod->impl->get_metadata( key ).c_str() );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return NULL;
}

double openmpt_module_get_current_estimated_bpm( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_current_estimated_bpm();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0.0;
}
int32_t openmpt_module_get_current_speed( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_current_speed();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int32_t openmpt_module_get_current_tempo( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_current_tempo();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
double openmpt_module_get_current_tempo2( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_current_tempo2();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int32_t openmpt_module_get_current_order( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_current_order();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int32_t openmpt_module_get_current_pattern( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_current_pattern();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int32_t openmpt_module_get_current_row( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_current_row();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int32_t openmpt_module_get_current_playing_channels( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_current_playing_channels();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}

float openmpt_module_get_current_channel_vu_mono( openmpt_module * mod, int32_t channel ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_current_channel_vu_mono( channel );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0.0;
}
float openmpt_module_get_current_channel_vu_left( openmpt_module * mod, int32_t channel ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_current_channel_vu_left( channel );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0.0;
}
float openmpt_module_get_current_channel_vu_right( openmpt_module * mod, int32_t channel ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_current_channel_vu_right( channel );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0.0;
}
float openmpt_module_get_current_channel_vu_rear_left( openmpt_module * mod, int32_t channel ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_current_channel_vu_rear_left( channel );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0.0;
}
float openmpt_module_get_current_channel_vu_rear_right( openmpt_module * mod, int32_t channel ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_current_channel_vu_rear_right( channel );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0.0;
}

int32_t openmpt_module_get_num_subsongs( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_num_subsongs();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int32_t openmpt_module_get_num_channels( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_num_channels();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int32_t openmpt_module_get_num_orders( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_num_orders();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int32_t openmpt_module_get_num_patterns( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_num_patterns();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int32_t openmpt_module_get_num_instruments( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_num_instruments();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int32_t openmpt_module_get_num_samples( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_num_samples();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}

const char * openmpt_module_get_subsong_name( openmpt_module * mod, int32_t index ) {
	try {
		openmpt::interface::check_soundfile( mod );
		std::vector<std::string> names = mod->impl->get_subsong_names();
		if ( names.size() >= (std::size_t)std::numeric_limits<int32_t>::max() ) {
			throw std::runtime_error("too many names");
		}
		if ( index < 0 || index >= (int32_t)names.size() ) {
			return openmpt::strdup( "" );
		}
		return openmpt::strdup( names[index].c_str() );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return NULL;
}
const char * openmpt_module_get_channel_name( openmpt_module * mod, int32_t index ) {
	try {
		openmpt::interface::check_soundfile( mod );
		std::vector<std::string> names = mod->impl->get_channel_names();
		if ( names.size() >= (std::size_t)std::numeric_limits<int32_t>::max() ) {
			throw std::runtime_error("too many names");
		}
		if ( index < 0 || index >= (int32_t)names.size() ) {
			return openmpt::strdup( "" );
		}
		return openmpt::strdup( names[index].c_str() );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return NULL;
}
const char * openmpt_module_get_order_name( openmpt_module * mod, int32_t index ) {
	try {
		openmpt::interface::check_soundfile( mod );
		std::vector<std::string> names = mod->impl->get_order_names();
		if ( names.size() >= (std::size_t)std::numeric_limits<int32_t>::max() ) {
			throw std::runtime_error("too many names");
		}
		if ( index < 0 || index >= (int32_t)names.size() ) {
			return openmpt::strdup( "" );
		}
		return openmpt::strdup( names[index].c_str() );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return NULL;
}
const char * openmpt_module_get_pattern_name( openmpt_module * mod, int32_t index ) {
	try {
		openmpt::interface::check_soundfile( mod );
		std::vector<std::string> names = mod->impl->get_pattern_names();
		if ( names.size() >= (std::size_t)std::numeric_limits<int32_t>::max() ) {
			throw std::runtime_error("too many names");
		}
		if ( index < 0 || index >= (int32_t)names.size() ) {
			return openmpt::strdup( "" );
		}
		return openmpt::strdup( names[index].c_str() );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return NULL;
}
const char * openmpt_module_get_instrument_name( openmpt_module * mod, int32_t index ) {
	try {
		openmpt::interface::check_soundfile( mod );
		std::vector<std::string> names = mod->impl->get_instrument_names();
		if ( names.size() >= (std::size_t)std::numeric_limits<int32_t>::max() ) {
			throw std::runtime_error("too many names");
		}
		if ( index < 0 || index >= (int32_t)names.size() ) {
			return openmpt::strdup( "" );
		}
		return openmpt::strdup( names[index].c_str() );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return NULL;
}
const char * openmpt_module_get_sample_name( openmpt_module * mod, int32_t index ) {
	try {
		openmpt::interface::check_soundfile( mod );
		std::vector<std::string> names = mod->impl->get_sample_names();
		if ( names.size() >= (std::size_t)std::numeric_limits<int32_t>::max() ) {
			throw std::runtime_error("too many names");
		}
		if ( index < 0 || index >= (int32_t)names.size() ) {
			return openmpt::strdup( "" );
		}
		return openmpt::strdup( names[index].c_str() );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return NULL;
}

int32_t openmpt_module_get_order_pattern( openmpt_module * mod, int32_t order ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_order_pattern( order );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}

int openmpt_module_is_order_skip_entry( openmpt_module * mod, int32_t order ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->is_order_skip_entry( order ) ? 1 : 0;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int openmpt_module_is_pattern_skip_item( openmpt_module * mod, int32_t pattern ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->is_pattern_skip_item( pattern ) ? 1 : 0;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int openmpt_module_is_order_stop_entry( openmpt_module * mod, int32_t order ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->is_order_stop_entry( order ) ? 1 : 0;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int openmpt_module_is_pattern_stop_item( openmpt_module * mod, int32_t pattern ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->is_pattern_stop_item( pattern ) ? 1 : 0;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}


int32_t openmpt_module_get_pattern_num_rows( openmpt_module * mod, int32_t pattern ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_pattern_num_rows( pattern );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}

int32_t openmpt_module_get_pattern_rows_per_beat( openmpt_module * mod, int32_t pattern ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_pattern_rows_per_beat( pattern );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int32_t openmpt_module_get_pattern_rows_per_measure( openmpt_module * mod, int32_t pattern ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_pattern_rows_per_measure( pattern );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}

uint8_t openmpt_module_get_pattern_row_channel_command( openmpt_module * mod, int32_t pattern, int32_t row, int32_t channel, int command ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return mod->impl->get_pattern_row_channel_command( pattern, row, channel, command );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}

const char * openmpt_module_format_pattern_row_channel_command( openmpt_module * mod, int32_t pattern, int32_t row, int32_t channel, int command ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return openmpt::strdup( mod->impl->format_pattern_row_channel_command( pattern, row, channel, command ).c_str() );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}

const char * openmpt_module_highlight_pattern_row_channel_command( openmpt_module * mod, int32_t pattern, int32_t row, int32_t channel, int command ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return openmpt::strdup( mod->impl->highlight_pattern_row_channel_command( pattern, row, channel, command ).c_str() );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}

const char * openmpt_module_format_pattern_row_channel( openmpt_module * mod, int32_t pattern, int32_t row, int32_t channel, size_t width, int pad ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return openmpt::strdup( mod->impl->format_pattern_row_channel( pattern, row, channel, width, pad ? true : false ).c_str() );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}

const char * openmpt_module_highlight_pattern_row_channel( openmpt_module * mod, int32_t pattern, int32_t row, int32_t channel, size_t width, int pad ) {
	try {
		openmpt::interface::check_soundfile( mod );
		return openmpt::strdup( mod->impl->highlight_pattern_row_channel( pattern, row, channel, width, pad ? true : false ).c_str() );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}

const char * openmpt_module_get_ctls( openmpt_module * mod ) {
	try {
		openmpt::interface::check_soundfile( mod );
		std::string retval;
		bool first = true;
		std::vector<std::string> ctls = mod->impl->get_ctls();
		for ( std::vector<std::string>::iterator i = ctls.begin(); i != ctls.end(); ++i ) {
			if ( first ) {
				first = false;
			} else {
				retval += ";";
			}
			retval += *i;
		}
		return openmpt::strdup( retval.c_str() );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return NULL;
}

const char * openmpt_module_ctl_get( openmpt_module * mod, const char * ctl ) {
	try {
		openmpt::interface::check_soundfile( mod );
		openmpt::interface::check_pointer( ctl );
		return openmpt::strdup( mod->impl->ctl_get( ctl ).c_str() );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return NULL;
}
int openmpt_module_ctl_get_boolean( openmpt_module * mod, const char * ctl ) {
	try {
		openmpt::interface::check_soundfile( mod );
		openmpt::interface::check_pointer( ctl );
		return mod->impl->ctl_get_boolean( ctl );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int64_t openmpt_module_ctl_get_integer( openmpt_module * mod, const char * ctl ) {
	try {
		openmpt::interface::check_soundfile( mod );
		openmpt::interface::check_pointer( ctl );
		return mod->impl->ctl_get_integer( ctl );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
double openmpt_module_ctl_get_floatingpoint( openmpt_module * mod, const char * ctl ) {
	try {
		openmpt::interface::check_soundfile( mod );
		openmpt::interface::check_pointer( ctl );
		return mod->impl->ctl_get_floatingpoint( ctl );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0.0;
}
const char * openmpt_module_ctl_get_text( openmpt_module * mod, const char * ctl ) {
	try {
		openmpt::interface::check_soundfile( mod );
		openmpt::interface::check_pointer( ctl );
		return openmpt::strdup( mod->impl->ctl_get_text( ctl ).c_str() );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return NULL;
}

int openmpt_module_ctl_set( openmpt_module * mod, const char * ctl, const char * value ) {
	try {
		openmpt::interface::check_soundfile( mod );
		openmpt::interface::check_pointer( ctl );
		openmpt::interface::check_pointer( value );
		mod->impl->ctl_set( ctl, value );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int openmpt_module_ctl_set_boolean( openmpt_module * mod, const char * ctl, int value ) {
	try {
		openmpt::interface::check_soundfile( mod );
		openmpt::interface::check_pointer( ctl );
		mod->impl->ctl_set_boolean( ctl, value ? true : false );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int openmpt_module_ctl_set_integer( openmpt_module * mod, const char * ctl, int64_t value ) {
	try {
		openmpt::interface::check_soundfile( mod );
		openmpt::interface::check_pointer( ctl );
		mod->impl->ctl_set_integer( ctl, value );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int openmpt_module_ctl_set_floatingpoint( openmpt_module * mod, const char * ctl, double value ) {
	try {
		openmpt::interface::check_soundfile( mod );
		openmpt::interface::check_pointer( ctl );
		mod->impl->ctl_set_floatingpoint( ctl, value );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}
int openmpt_module_ctl_set_text( openmpt_module * mod, const char * ctl, const char * value ) {
	try {
		openmpt::interface::check_soundfile( mod );
		openmpt::interface::check_pointer( ctl );
		openmpt::interface::check_pointer( value );
		mod->impl->ctl_set_text( ctl, value );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod );
	}
	return 0;
}

openmpt_module_ext * openmpt_module_ext_create( openmpt_stream_callbacks stream_callbacks, void * stream, openmpt_log_func logfunc, void * loguser, openmpt_error_func errfunc, void * erruser, int * error, const char * * error_message, const openmpt_module_initial_ctl * ctls ) {
	try {
		openmpt_module_ext * mod_ext = (openmpt_module_ext*)std::calloc( 1, sizeof( openmpt_module_ext ) );
		if ( !mod_ext ) {
			throw std::bad_alloc();
		}
		std::memset( mod_ext, 0, sizeof( openmpt_module_ext ) );
		openmpt_module * mod = &mod_ext->mod;
		std::memset( mod, 0, sizeof( openmpt_module ) );
		mod_ext->impl = 0;
		mod->logfunc = logfunc ? logfunc : openmpt_log_func_default;
		mod->loguser = loguser;
		mod->errfunc = errfunc ? errfunc : NULL;
		mod->erruser = erruser;
		mod->error = OPENMPT_ERROR_OK;
		mod->error_message = NULL;
		mod->impl = 0;
		try {
			std::map< std::string, std::string > ctls_map;
			if ( ctls ) {
				for ( const openmpt_module_initial_ctl * it = ctls; it->ctl; ++it ) {
					if ( it->value ) {
						ctls_map[ it->ctl ] = it->value;
					} else {
						ctls_map.erase( it->ctl );
					}
				}
			}
			openmpt::callback_stream_wrapper istream = { stream, stream_callbacks.read, stream_callbacks.seek, stream_callbacks.tell };
			mod_ext->impl = new openmpt::module_ext_impl( istream, openmpt::helper::make_unique<openmpt::logfunc_logger>( mod->logfunc, mod->loguser ), ctls_map );
			mod->impl = mod_ext->impl;
			return mod_ext;
		} catch ( ... ) {
			openmpt::report_exception( __func__, mod, error, error_message );
		}
		#if defined(_MSC_VER)
		#pragma warning(push)
		#pragma warning(disable:6001) // false-positive: Using uninitialized memory 'mod_ext'.
		#endif // _MSC_VER
			delete mod_ext->impl;
		#if defined(_MSC_VER)
		#pragma warning(pop)
		#endif // _MSC_VER
		mod_ext->impl = 0;
		mod->impl = 0;
		if ( mod->error_message ) {
			openmpt_free_string( mod->error_message );
			mod->error_message = NULL;
		}
		std::free( (void*)mod_ext );
		mod_ext = NULL;
	} catch ( ... ) {
		openmpt::report_exception( __func__, 0, error, error_message );
	}
	return NULL;
}

openmpt_module_ext * openmpt_module_ext_create_from_memory( const void * filedata, size_t filesize, openmpt_log_func logfunc, void * loguser, openmpt_error_func errfunc, void * erruser, int * error, const char * * error_message, const openmpt_module_initial_ctl * ctls ) {
	try {
		openmpt_module_ext * mod_ext = (openmpt_module_ext*)std::calloc( 1, sizeof( openmpt_module_ext ) );
		if ( !mod_ext ) {
			throw std::bad_alloc();
		}
		std::memset( mod_ext, 0, sizeof( openmpt_module_ext ) );
		openmpt_module * mod = &mod_ext->mod;
		std::memset( mod, 0, sizeof( openmpt_module ) );
		mod_ext->impl = 0;
		mod->logfunc = logfunc ? logfunc : openmpt_log_func_default;
		mod->loguser = loguser;
		mod->errfunc = errfunc ? errfunc : NULL;
		mod->erruser = erruser;
		mod->error = OPENMPT_ERROR_OK;
		mod->error_message = NULL;
		mod->impl = 0;
		try {
			std::map< std::string, std::string > ctls_map;
			if ( ctls ) {
				for ( const openmpt_module_initial_ctl * it = ctls; it->ctl; ++it ) {
					if ( it->value ) {
						ctls_map[ it->ctl ] = it->value;
					} else {
						ctls_map.erase( it->ctl );
					}
				}
			}
			mod_ext->impl = new openmpt::module_ext_impl( filedata, filesize, openmpt::helper::make_unique<openmpt::logfunc_logger>( mod->logfunc, mod->loguser ), ctls_map );
			mod->impl = mod_ext->impl;
			return mod_ext;
		} catch ( ... ) {
			openmpt::report_exception( __func__, mod, error, error_message );
		}
		#if defined(_MSC_VER)
		#pragma warning(push)
		#pragma warning(disable:6001) // false-positive: Using uninitialized memory 'mod_ext'.
		#endif // _MSC_VER
			delete mod_ext->impl;
		#if defined(_MSC_VER)
		#pragma warning(pop)
		#endif // _MSC_VER
		mod_ext->impl = 0;
		mod->impl = 0;
		if ( mod->error_message ) {
			openmpt_free_string( mod->error_message );
			mod->error_message = NULL;
		}
		std::free( (void*)mod_ext );
		mod_ext = NULL;
	} catch ( ... ) {
		openmpt::report_exception( __func__, 0, error, error_message );
	}
	return NULL;
}

void openmpt_module_ext_destroy( openmpt_module_ext * mod_ext ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		openmpt_module * mod = &mod_ext->mod;
		mod->impl = 0;
		delete mod_ext->impl;
		mod_ext->impl = 0;
		if ( mod->error_message ) {
			openmpt_free_string( mod->error_message );
			mod->error_message = NULL;
		}
		std::free( (void*)mod_ext );
		mod_ext = NULL;
		return;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return;
}

openmpt_module * openmpt_module_ext_get_module( openmpt_module_ext * mod_ext ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		openmpt_module * mod = &mod_ext->mod;
		return mod;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return NULL;
}



static int get_pattern_row_channel_volume_effect_type( openmpt_module_ext * mod_ext, int32_t pattern, int32_t row, int32_t channel ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		return mod_ext->impl->get_pattern_row_channel_volume_effect_type( pattern, row, channel );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return -1;
}
static int get_pattern_row_channel_effect_type( openmpt_module_ext * mod_ext, int32_t pattern, int32_t row, int32_t channel ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		return mod_ext->impl->get_pattern_row_channel_effect_type( pattern, row, channel );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return -1;
}



static int set_current_speed( openmpt_module_ext * mod_ext, int32_t speed ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		mod_ext->impl->set_current_speed( speed );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0;
}
static int set_current_tempo( openmpt_module_ext * mod_ext, int32_t tempo ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		mod_ext->impl->set_current_tempo( tempo );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0;
}
static int set_tempo_factor( openmpt_module_ext * mod_ext, double factor ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		mod_ext->impl->set_tempo_factor( factor );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0;
}
static double get_tempo_factor( openmpt_module_ext * mod_ext ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		return mod_ext->impl->get_tempo_factor();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0.0;
}
static int set_pitch_factor( openmpt_module_ext * mod_ext, double factor ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		mod_ext->impl->set_pitch_factor( factor );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0;
}
static double get_pitch_factor( openmpt_module_ext * mod_ext ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		return mod_ext->impl->get_pitch_factor();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0.0;
}
static int set_global_volume( openmpt_module_ext * mod_ext, double volume ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		mod_ext->impl->set_global_volume( volume );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0;
}
static double get_global_volume( openmpt_module_ext * mod_ext ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		return mod_ext->impl->get_global_volume();
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0.0;
}
static int set_channel_volume( openmpt_module_ext * mod_ext, int32_t channel, double volume ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		mod_ext->impl->set_channel_volume( channel, volume );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0;
}
static double get_channel_volume( openmpt_module_ext * mod_ext, int32_t channel ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		return mod_ext->impl->get_channel_volume( channel );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0.0;
}
static int set_channel_mute_status( openmpt_module_ext * mod_ext, int32_t channel, int mute ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		mod_ext->impl->set_channel_mute_status( channel, mute ? true : false );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0;
}
static int get_channel_mute_status( openmpt_module_ext * mod_ext, int32_t channel ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		return mod_ext->impl->get_channel_mute_status( channel ) ? 1 : 0;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return -1;
}
static int set_instrument_mute_status( openmpt_module_ext * mod_ext, int32_t instrument, int mute ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		mod_ext->impl->set_instrument_mute_status( instrument, mute ? true : false );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0;
}
static int get_instrument_mute_status( openmpt_module_ext * mod_ext, int32_t instrument ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		return mod_ext->impl->get_instrument_mute_status( instrument ) ? 1 : 0;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return -1;
}
static int32_t play_note( openmpt_module_ext * mod_ext, int32_t instrument, int32_t note, double volume, double panning ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		return mod_ext->impl->play_note( instrument, note, volume, panning );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return -1;
}
static int stop_note( openmpt_module_ext * mod_ext, int32_t channel ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		mod_ext->impl->stop_note( channel );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0;
}



static int note_off( openmpt_module_ext * mod_ext, int32_t channel ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		mod_ext->impl->note_off(channel );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0;
}
static int note_fade( openmpt_module_ext * mod_ext, int32_t channel ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		mod_ext->impl->note_fade(channel );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0;
}
static int set_channel_panning( openmpt_module_ext * mod_ext, int32_t channel, double panning ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		mod_ext->impl->set_channel_panning( channel, panning );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0;
}
static double get_channel_panning( openmpt_module_ext * mod_ext, int32_t channel ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		return mod_ext->impl->get_channel_panning( channel );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0.0;
}
static int set_note_finetune( openmpt_module_ext * mod_ext, int32_t channel, double finetune ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		mod_ext->impl->set_note_finetune( channel, finetune );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0;
}
static double get_note_finetune( openmpt_module_ext * mod_ext, int32_t channel ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		return mod_ext->impl->get_note_finetune( channel );
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0.0;
}



static int set_current_tempo2( openmpt_module_ext * mod_ext, double tempo ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		mod_ext->impl->set_current_tempo2( tempo );
		return 1;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0;
}



/* add stuff here */



int openmpt_module_ext_get_interface( openmpt_module_ext * mod_ext, const char * interface_id, void * interface, size_t interface_size ) {
	try {
		openmpt::interface::check_soundfile( mod_ext );
		openmpt::interface::check_pointer( interface_id );
		openmpt::interface::check_pointer( interface );
		std::memset( interface, 0, interface_size );
		int result = 0;
		std::string_view interface_id_sv = interface_id;
		if ( interface_id_sv == "" ) {
			result = 0;



		} else if ( ( interface_id_sv == LIBOPENMPT_EXT_C_INTERFACE_PATTERN_VIS ) && ( interface_size == sizeof( openmpt_module_ext_interface_pattern_vis ) ) ) {
			openmpt_module_ext_interface_pattern_vis * i = static_cast< openmpt_module_ext_interface_pattern_vis * >( interface );
			i->get_pattern_row_channel_volume_effect_type = &get_pattern_row_channel_volume_effect_type;
			i->get_pattern_row_channel_effect_type = &get_pattern_row_channel_effect_type;
			result = 1;



		} else if ( ( interface_id_sv == LIBOPENMPT_EXT_C_INTERFACE_INTERACTIVE ) && ( interface_size == sizeof( openmpt_module_ext_interface_interactive ) ) ) {
			openmpt_module_ext_interface_interactive * i = static_cast< openmpt_module_ext_interface_interactive * >( interface );
			i->set_current_speed = &set_current_speed;
			i->set_current_tempo = &set_current_tempo;
			i->set_tempo_factor = &set_tempo_factor;
			i->get_tempo_factor = &get_tempo_factor;
			i->set_pitch_factor = &set_pitch_factor;
			i->get_pitch_factor = &get_pitch_factor;
			i->set_global_volume = &set_global_volume;
			i->get_global_volume = &get_global_volume;
			i->set_channel_volume = &set_channel_volume;
			i->get_channel_volume = &get_channel_volume;
			i->set_channel_mute_status = &set_channel_mute_status;
			i->get_channel_mute_status = &get_channel_mute_status;
			i->set_instrument_mute_status = &set_instrument_mute_status;
			i->get_instrument_mute_status = &get_instrument_mute_status;
			i->play_note = &play_note;
			i->stop_note = &stop_note;
			result = 1;



		} else if ( ( interface_id_sv == LIBOPENMPT_EXT_C_INTERFACE_INTERACTIVE2 ) && ( interface_size == sizeof( openmpt_module_ext_interface_interactive2 ) ) ) {
			openmpt_module_ext_interface_interactive2 * i = static_cast< openmpt_module_ext_interface_interactive2 * >( interface );
			i->note_off = &note_off;
			i->note_fade = &note_fade;
			i->set_channel_panning = &set_channel_panning;
			i->get_channel_panning = &get_channel_panning;
			i->set_note_finetune = &set_note_finetune;
			i->get_note_finetune = &get_note_finetune;
			result = 1;



		} else if ( ( interface_id_sv == LIBOPENMPT_EXT_C_INTERFACE_INTERACTIVE3 ) && ( interface_size == sizeof( openmpt_module_ext_interface_interactive3 ) ) ) {
			openmpt_module_ext_interface_interactive3 * i = static_cast< openmpt_module_ext_interface_interactive3 * >( interface );
			i->set_current_tempo2 = &set_current_tempo2;
			result = 1;



/* add stuff here */



		} else {
			result = 0;
		}
		return result;
	} catch ( ... ) {
		openmpt::report_exception( __func__, mod_ext ? &mod_ext->mod : NULL );
	}
	return 0;
}

} // extern "C"
