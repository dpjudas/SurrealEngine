/*
 * libopenmpt_stream_callbacks_file.h
 * ----------------------------------
 * Purpose: libopenmpt public c interface
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#ifndef LIBOPENMPT_STREAM_CALLBACKS_FILE_H
#define LIBOPENMPT_STREAM_CALLBACKS_FILE_H

#include "libopenmpt.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*! \addtogroup libopenmpt_c
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* This stuff has to be in a header file because of possibly different MSVC CRTs which cause problems for FILE * crossing CRT boundaries. */

static size_t openmpt_stream_file_read_func( void * stream, void * dst, size_t bytes ) {
	FILE * f = 0;
	size_t retval = 0;
	f = (FILE*)stream;
	if ( !f ) {
		return 0;
	}
	retval = fread( dst, 1, bytes, f );
	if ( retval <= 0 ) {
		return 0;
	}
	return retval;
}

static int openmpt_stream_file_seek_func( void * stream, int64_t offset, int whence ) {
	FILE * f = 0;
	int fwhence = 0;
	f = (FILE*)stream;
	if ( !f ) {
		return -1;
	}
	switch ( whence ) {
		case OPENMPT_STREAM_SEEK_SET:
			fwhence = SEEK_SET;
			break;
		case OPENMPT_STREAM_SEEK_CUR:
			fwhence = SEEK_CUR;
			break;
		case OPENMPT_STREAM_SEEK_END:
			fwhence = SEEK_END;
			break;
		default:
			return -1;
			break;
	}
	if ( (long)offset != offset ) {
		return -1;
	}
	return fseek( f, (long)offset, fwhence ) ? -1 : 0;
}

static int64_t openmpt_stream_file_tell_func( void * stream ) {
	FILE * f = 0;
	long result = 0;
	int64_t retval = 0;
	f = (FILE*)stream;
	if ( !f ) {
		return -1;
	}
	result = ftell( f );
	if ( (int64_t)result != result ) {
		return -1;
	}
	retval = (int64_t)result;
	if ( retval < 0 ) {
		return -1;
	}
	return retval;
}

/*! \brief Provide openmpt_stream_callbacks for standard C FILE objects
 *
 * Fills openmpt_stream_callbacks suitable for passing a standard C FILE object as a stream parameter to functions doing file input/output.
 * Since 0.7.0, it does not try to use platform-specific file seeking any more,
 * but sticks to standard C fseek/ftell only, which means on platforms where
 * long is 32bit, there is no 64bit file access possible any more.
 *
 * \remarks The stream argument must be passed as `(void*)(FILE*)file`.
 * \sa \ref libopenmpt_c_fileio
 * \sa openmpt_stream_callbacks
 * \sa openmpt_could_open_probability2
 * \sa openmpt_probe_file_header_from_stream
 * \sa openmpt_module_create2
 * \sa openmpt_stream_get_file_callbacks2()
 * \sa openmpt_stream_get_file_mingw_callbacks()
 * \sa openmpt_stream_get_file_msvcrt_callbacks()
 * \sa openmpt_stream_get_file_posix_callbacks()
 * \sa openmpt_stream_get_file_posix_lfs64_callbacks()
 * \deprecated Please use openmpt_stream_get_file_callbacks2().
 */
LIBOPENMPT_DEPRECATED static LIBOPENMPT_C_INLINE openmpt_stream_callbacks openmpt_stream_get_file_callbacks(void) {
	openmpt_stream_callbacks retval;
	memset( &retval, 0, sizeof( openmpt_stream_callbacks ) );
	retval.read = openmpt_stream_file_read_func;
	retval.seek = openmpt_stream_file_seek_func;
	retval.tell = openmpt_stream_file_tell_func;
	return retval;
}

/*! \brief Provide openmpt_stream_callbacks for standard C FILE objects
 *
 * Fills openmpt_stream_callbacks suitable for passing a standard C FILE object as a stream parameter to functions doing file input/output.
 * This function uses the standard C fseek and ftell, thus platform-specific file size limits apply accordingly.
 *
 * \remarks The stream argument must be passed as `(void*)(FILE*)file`.
 * \remarks The provided callbacks are limited to handling files up to the size representable in the platform's long type.
 * \sa \ref libopenmpt_c_fileio
 * \sa openmpt_stream_callbacks
 * \sa openmpt_could_open_probability2
 * \sa openmpt_probe_file_header_from_stream
 * \sa openmpt_module_create2
 * \since 0.7.0
 */
static openmpt_stream_callbacks openmpt_stream_get_file_callbacks2(void) {
	openmpt_stream_callbacks retval;
	memset( &retval, 0, sizeof( openmpt_stream_callbacks ) );
	retval.read = openmpt_stream_file_read_func;
	retval.seek = openmpt_stream_file_seek_func;
	retval.tell = openmpt_stream_file_tell_func;
	return retval;
}

#ifdef __cplusplus
}
#endif

/*!
 * @}
 */

#endif /* LIBOPENMPT_STREAM_CALLBACKS_FILE_H */
