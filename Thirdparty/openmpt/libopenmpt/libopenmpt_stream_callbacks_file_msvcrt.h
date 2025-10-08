/*
 * libopenmpt_stream_callbacks_file_msvcrt.h
 * -----------------------------------------
 * Purpose: libopenmpt public c interface
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#ifndef LIBOPENMPT_STREAM_CALLBACKS_FILE_MSVCRT_H
#define LIBOPENMPT_STREAM_CALLBACKS_FILE_MSVCRT_H

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

static size_t openmpt_stream_file_msvcrt_read_func( void * stream, void * dst, size_t bytes ) {
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

static int openmpt_stream_file_msvcrt_seek_func( void * stream, int64_t offset, int whence ) {
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
	if ( (__int64)offset != offset ) {
		return -1;
	}
	return _fseeki64( f, (__int64)offset, fwhence ) ? -1 : 0;
}

static int64_t openmpt_stream_file_msvcrt_tell_func( void * stream ) {
	FILE * f = 0;
	__int64 result = 0;
	int64_t retval = 0;
	f = (FILE*)stream;
	if ( !f ) {
		return -1;
	}
	result = _ftelli64( f );
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
 *
 * \remarks The stream argument must be passed as `(void*)(FILE*)file`.
 * \sa \ref libopenmpt_c_fileio
 * \sa openmpt_stream_callbacks
 * \sa openmpt_could_open_probability2
 * \sa openmpt_probe_file_header_from_stream
 * \sa openmpt_module_create2
 */
static openmpt_stream_callbacks openmpt_stream_get_file_msvcrt_callbacks(void) {
	openmpt_stream_callbacks retval;
	memset( &retval, 0, sizeof( openmpt_stream_callbacks ) );
	retval.read = openmpt_stream_file_msvcrt_read_func;
	retval.seek = openmpt_stream_file_msvcrt_seek_func;
	retval.tell = openmpt_stream_file_msvcrt_tell_func;
	return retval;
}

#ifdef __cplusplus
}
#endif

/*!
 * @}
 */

#endif /* LIBOPENMPT_STREAM_CALLBACKS_FILE_MSVCRT_H */
