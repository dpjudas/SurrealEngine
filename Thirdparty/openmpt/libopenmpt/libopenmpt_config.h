/*
 * libopenmpt_config.h
 * -------------------
 * Purpose: libopenmpt public interface configuration
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#ifndef LIBOPENMPT_CONFIG_H
#define LIBOPENMPT_CONFIG_H

/* clang-format off */

/*! \defgroup libopenmpt libopenmpt */

/*! \addtogroup libopenmpt
  @{
*/

/* provoke warnings if already defined */
#define LIBOPENMPT_API
#undef LIBOPENMPT_API
#define LIBOPENMPT_CXX_API
#undef LIBOPENMPT_CXX_API

/*!
  @}
*/

/*! \addtogroup libopenmpt_c
  @{
*/

/*! \brief Defined if libopenmpt/libopenmpt_stream_callbacks_buffer.h exists.
 * \remarks
 *   Use the following to check for availability:
 *   \code
 *   #include <libopenmpt/libopenmpt.h>
 *   #if defined(LIBOPENMPT_STREAM_CALLBACKS_BUFFER)
 *   #include <libopenmpt/libopenmpt_stream_callbacks_buffer.h>
 *   #endif
 *   \endcode
 */
#define LIBOPENMPT_STREAM_CALLBACKS_BUFFER

/*! \brief Defined if libopenmpt/libopenmpt_stream_callbacks_fd.h exists.
 * \since 0.3.0
 * \remarks
 *   Use the following to check for availability:
 *   \code
 *   #include <libopenmpt/libopenmpt.h>
 *   #if defined(LIBOPENMPT_STREAM_CALLBACKS_FD) || ((OPENMPT_API_VERSION_MAJOR == 0) && ((OPENMPT_API_VERSION_MINOR == 2) || (OPENMPT_API_VERSION_MINOR == 1)))
 *   #include <libopenmpt/libopenmpt_stream_callbacks_fd.h>
 *   #endif
 *   \endcode
 */
#define LIBOPENMPT_STREAM_CALLBACKS_FD

/*! \brief Defined if libopenmpt/libopenmpt_stream_callbacks_file.h exists.
 * \since 0.3.0
 * \remarks
 *   Use the following to check for availability:
 *   \code
 *   #include <libopenmpt/libopenmpt.h>
 *   #if defined(LIBOPENMPT_STREAM_CALLBACKS_FILE) || ((OPENMPT_API_VERSION_MAJOR == 0) && ((OPENMPT_API_VERSION_MINOR == 2) || (OPENMPT_API_VERSION_MINOR == 1)))
 *   #include <libopenmpt/libopenmpt_stream_callbacks_file.h>
 *   #endif
 *   \endcode
 */
#define LIBOPENMPT_STREAM_CALLBACKS_FILE

 /*! \brief Defined if libopenmpt/libopenmpt_stream_callbacks_file_mingw.h exists.
  * \since 0.7.0
  * \remarks
  *   This macro does not determine if the interfaces required to use libopenmpt/libopenmpt_stream_callbacks_file_posix_lfs64.h are available.
  *   It is the libopenmpt user's responsibility to check for availability of the _off64_t, _ftelli64(), and _fseeki64().
  *   Use the following to check for availability:
  *   \code
  *   #include <libopenmpt/libopenmpt.h>
  *   #if defined(LIBOPENMPT_STREAM_CALLBACKS_FILE_MINGW)
  *   #include <libopenmpt/libopenmpt_stream_callbacks_file_mingw.h>
  *   #endif
  *   \endcode
  */
#define LIBOPENMPT_STREAM_CALLBACKS_FILE_MINGW

 /*! \brief Defined if libopenmpt/libopenmpt_stream_callbacks_file_msvcrt.h exists.
  * \since 0.7.0
  * \remarks
  *   This macro does not determine if the interfaces required to use libopenmpt/libopenmpt_stream_callbacks_file_posix_lfs64.h are available.
  *   It is the libopenmpt user's responsibility to check for availability of the __int64, _ftelli64(), and _fseeki64().
  *   Use the following to check for availability:
  *   \code
  *   #include <libopenmpt/libopenmpt.h>
  *   #if defined(LIBOPENMPT_STREAM_CALLBACKS_FILE_MSVCRT)
  *   #include <libopenmpt/libopenmpt_stream_callbacks_file_msvcrt.h>
  *   #endif
  *   \endcode
  */
#define LIBOPENMPT_STREAM_CALLBACKS_FILE_MSVCRT

 /*! \brief Defined if libopenmpt/libopenmpt_stream_callbacks_file_posix.h exists.
  * \since 0.7.0
  * \remarks
  *   This macro does not determine if the interfaces required to use libopenmpt/libopenmpt_stream_callbacks_file_posix_lfs64.h are available.
  *   It is the libopenmpt user's responsibility to check for availability of the _off_t, ftello(), and fseeko().
  *   Use the following to check for availability:
  *   \code
  *   #include <libopenmpt/libopenmpt.h>
  *   #if defined(LIBOPENMPT_STREAM_CALLBACKS_FILE_MINGW)
  *   #include <libopenmpt/libopenmpt_stream_callbacks_file_mingw.h>
  *   #endif
  *   \endcode
  */
#define LIBOPENMPT_STREAM_CALLBACKS_FILE_POSIX

/*! \brief Defined if libopenmpt/libopenmpt_stream_callbacks_file_posix_lfs64.h exists.
 * \since 0.7.0
 * \remarks
 *   This macro does not determine if the interfaces required to use libopenmpt/libopenmpt_stream_callbacks_file_posix_lfs64.h are available.
 *   It is the libopenmpt user's responsibility to check for availability of the off64_t, ftello64(), and fseeko64().
 *   Use the following to check for availability:
 *   \code
 *   #include <libopenmpt/libopenmpt.h>
 *   #if defined(LIBOPENMPT_STREAM_CALLBACKS_FILE_POSIX_LFS64)
 *   #include <libopenmpt/libopenmpt_stream_callbacks_file_posix_lfs64.h>
 *   #endif
 *   \endcode
 */
#define LIBOPENMPT_STREAM_CALLBACKS_FILE_POSIX_LFS64

/*!
  @}
*/

/*! \addtogroup libopenmpt
  @{
*/

#if defined(__DOXYGEN__)

#define LIBOPENMPT_API_HELPER_EXPORT
#define LIBOPENMPT_API_HELPER_IMPORT
#define LIBOPENMPT_API_HELPER_PUBLIC
#define LIBOPENMPT_API_HELPER_LOCAL

#elif defined(_MSC_VER)

#define LIBOPENMPT_API_HELPER_EXPORT __declspec(dllexport)
#define LIBOPENMPT_API_HELPER_IMPORT __declspec(dllimport)
#define LIBOPENMPT_API_HELPER_PUBLIC
#define LIBOPENMPT_API_HELPER_LOCAL

#ifdef __cplusplus
#define LIBOPENMPT_API_HELPER_EXPORT_CLASS __declspec(dllexport)
#define LIBOPENMPT_API_HELPER_IMPORT_CLASS __declspec(dllimport)
#define LIBOPENMPT_API_HELPER_PUBLIC_CLASS 
#define LIBOPENMPT_API_HELPER_LOCAL_CLASS  
#endif

#elif defined(__EMSCRIPTEN__)

#define LIBOPENMPT_API_HELPER_EXPORT __attribute__((visibility("default"))) __attribute__((used))
#define LIBOPENMPT_API_HELPER_IMPORT __attribute__((visibility("default"))) __attribute__((used))
#define LIBOPENMPT_API_HELPER_PUBLIC __attribute__((visibility("default"))) __attribute__((used))
#define LIBOPENMPT_API_HELPER_LOCAL  __attribute__((visibility("hidden")))

#ifdef __cplusplus
#define LIBOPENMPT_API_HELPER_EXPORT_CLASS __attribute__((visibility("default")))
#define LIBOPENMPT_API_HELPER_IMPORT_CLASS __attribute__((visibility("default")))
#define LIBOPENMPT_API_HELPER_PUBLIC_CLASS __attribute__((visibility("default")))
#define LIBOPENMPT_API_HELPER_LOCAL_CLASS  __attribute__((visibility("hidden")))
#define LIBOPENMPT_API_HELPER_EXPORT_MEMBER __attribute__((visibility("default"))) __attribute__((used))
#define LIBOPENMPT_API_HELPER_IMPORT_MEMBER __attribute__((visibility("default"))) __attribute__((used))
#define LIBOPENMPT_API_HELPER_PUBLIC_MEMBER __attribute__((visibility("default"))) __attribute__((used))
#define LIBOPENMPT_API_HELPER_LOCAL_MEMBER  __attribute__((visibility("hidden")))
#endif

#elif (defined(__GNUC__) || defined(__clang__)) && defined(_WIN32)

#define LIBOPENMPT_API_HELPER_EXPORT __declspec(dllexport)
#define LIBOPENMPT_API_HELPER_IMPORT __declspec(dllimport)
#define LIBOPENMPT_API_HELPER_PUBLIC __attribute__((visibility("default")))
#define LIBOPENMPT_API_HELPER_LOCAL  __attribute__((visibility("hidden")))

#ifdef __cplusplus
#define LIBOPENMPT_API_HELPER_EXPORT_CLASS __declspec(dllexport)
#define LIBOPENMPT_API_HELPER_IMPORT_CLASS __declspec(dllimport)
#define LIBOPENMPT_API_HELPER_PUBLIC_CLASS __attribute__((visibility("default")))
#define LIBOPENMPT_API_HELPER_LOCAL_CLASS  __attribute__((visibility("hidden")))
#endif

#elif defined(__GNUC__) || defined(__clang__)

#define LIBOPENMPT_API_HELPER_EXPORT __attribute__((visibility("default")))
#define LIBOPENMPT_API_HELPER_IMPORT __attribute__((visibility("default")))
#define LIBOPENMPT_API_HELPER_PUBLIC __attribute__((visibility("default")))
#define LIBOPENMPT_API_HELPER_LOCAL  __attribute__((visibility("hidden")))

#ifdef __cplusplus
#define LIBOPENMPT_API_HELPER_EXPORT_CLASS __attribute__((visibility("default")))
#define LIBOPENMPT_API_HELPER_IMPORT_CLASS __attribute__((visibility("default")))
#define LIBOPENMPT_API_HELPER_PUBLIC_CLASS __attribute__((visibility("default")))
#define LIBOPENMPT_API_HELPER_LOCAL_CLASS  __attribute__((visibility("hidden")))
#endif

#elif defined(_WIN32)

#define LIBOPENMPT_API_HELPER_EXPORT __declspec(dllexport)
#define LIBOPENMPT_API_HELPER_IMPORT __declspec(dllimport)
#define LIBOPENMPT_API_HELPER_PUBLIC
#define LIBOPENMPT_API_HELPER_LOCAL

#ifdef __cplusplus

#define LIBOPENMPT_API_HELPER_EXPORT_CLASS __declspec(dllexport)
#define LIBOPENMPT_API_HELPER_IMPORT_CLASS __declspec(dllimport)
#define LIBOPENMPT_API_HELPER_PUBLIC_CLASS 
#define LIBOPENMPT_API_HELPER_LOCAL_CLASS  

#endif

#else

#define LIBOPENMPT_API_HELPER_EXPORT 
#define LIBOPENMPT_API_HELPER_IMPORT 
#define LIBOPENMPT_API_HELPER_PUBLIC 
#define LIBOPENMPT_API_HELPER_LOCAL  

#endif

#ifdef __cplusplus

#ifndef LIBOPENMPT_API_HELPER_EXPORT_CLASS
#define LIBOPENMPT_API_HELPER_EXPORT_CLASS
#endif
#ifndef LIBOPENMPT_API_HELPER_EXPORT_MEMBER
#define LIBOPENMPT_API_HELPER_EXPORT_MEMBER
#endif
#ifndef LIBOPENMPT_API_HELPER_IMPORT_CLASS
#define LIBOPENMPT_API_HELPER_IMPORT_CLASS
#endif
#ifndef LIBOPENMPT_API_HELPER_IMPORT_MEMBER
#define LIBOPENMPT_API_HELPER_IMPORT_MEMBER
#endif
#ifndef LIBOPENMPT_API_HELPER_PUBLIC_CLASS
#define LIBOPENMPT_API_HELPER_PUBLIC_CLASS
#endif
#ifndef LIBOPENMPT_API_HELPER_PUBLIC_MEMBER
#define LIBOPENMPT_API_HELPER_PUBLIC_MEMBER
#endif
#ifndef LIBOPENMPT_API_HELPER_LOCAL_CLASS
#define LIBOPENMPT_API_HELPER_LOCAL_CLASS
#endif
#ifndef LIBOPENMPT_API_HELPER_LOCAL_MEMBER
#define LIBOPENMPT_API_HELPER_LOCAL_MEMBER
#endif

#endif

#if defined(LIBOPENMPT_BUILD_DLL)
#define LIBOPENMPT_API        LIBOPENMPT_API_HELPER_EXPORT
#elif defined(LIBOPENMPT_USE_DLL)
#define LIBOPENMPT_API        LIBOPENMPT_API_HELPER_IMPORT
#else
#define LIBOPENMPT_API        LIBOPENMPT_API_HELPER_PUBLIC
#endif

#ifdef __cplusplus

#if defined(LIBOPENMPT_BUILD_DLL)
#define LIBOPENMPT_CXX_API        LIBOPENMPT_API_HELPER_EXPORT
#define LIBOPENMPT_CXX_API_CLASS  LIBOPENMPT_API_HELPER_EXPORT_CLASS
#define LIBOPENMPT_CXX_API_MEMBER LIBOPENMPT_API_HELPER_EXPORT_MEMBER
#elif defined(LIBOPENMPT_USE_DLL)
#define LIBOPENMPT_CXX_API        LIBOPENMPT_API_HELPER_IMPORT
#define LIBOPENMPT_CXX_API_CLASS  LIBOPENMPT_API_HELPER_IMPORT_CLASS
#define LIBOPENMPT_CXX_API_MEMBER LIBOPENMPT_API_HELPER_IMPORT_MEMBER
#else
#define LIBOPENMPT_CXX_API        LIBOPENMPT_API_HELPER_PUBLIC
#define LIBOPENMPT_CXX_API_CLASS  LIBOPENMPT_API_HELPER_PUBLIC_CLASS
#define LIBOPENMPT_CXX_API_MEMBER LIBOPENMPT_API_HELPER_PUBLIC_MEMBER
#endif

#if defined(LIBOPENMPT_USE_DLL)
#if defined(_MSC_VER) && !defined(_DLL)
#error "C++ interface is disabled if libopenmpt is built as a DLL and the runtime is statically linked. This is not supported by microsoft and cannot possibly work. Ever."
#undef LIBOPENMPT_CXX_API
#define LIBOPENMPT_CXX_API LIBOPENMPT_API_HELPER_LOCAL
#undef LIBOPENMPT_CXX_API_CLASS
#define LIBOPENMPT_CXX_API_CLASS LIBOPENMPT_API_HELPER_LOCAL_CLASS
#undef LIBOPENMPT_CXX_API_MEMBER
#define LIBOPENMPT_CXX_API_MEMBER LIBOPENMPT_API_HELPER_LOCAL_MEMBER
#endif
#endif

#endif

/*!
  @}
*/


/* C */

#if !defined(LIBOPENMPT_NO_DEPRECATE)
#if defined(__clang__)
#define LIBOPENMPT_DEPRECATED __attribute__((deprecated))
#elif defined(__GNUC__)
#define LIBOPENMPT_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define LIBOPENMPT_DEPRECATED __declspec(deprecated)
#else
#define LIBOPENMPT_DEPRECATED
#endif
#else
#define LIBOPENMPT_DEPRECATED
#endif

#ifndef __cplusplus
#if !defined(LIBOPENMPT_NO_DEPRECATE)
LIBOPENMPT_DEPRECATED static const int LIBOPENMPT_DEPRECATED_STRING_CONSTANT = 0;
#define LIBOPENMPT_DEPRECATED_STRING( str ) ( LIBOPENMPT_DEPRECATED_STRING_CONSTANT ? ( str ) : ( str ) )
#else
#define LIBOPENMPT_DEPRECATED_STRING( str ) str
#endif
#else
#define LIBOPENMPT_DEPRECATED_STRING( str ) str
#endif

#if defined(__STDC__) && (__STDC__ == 1)
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define LIBOPENMPT_C_INLINE inline
#else
#define LIBOPENMPT_C_INLINE
#endif
#else
#define LIBOPENMPT_C_INLINE
#endif


/* C++ */

#ifdef __cplusplus

#if defined(LIBOPENMPT_ASSUME_CPLUSPLUS)
#endif

#if !defined(LIBOPENMPT_NO_DEPRECATE)
#define LIBOPENMPT_ATTR_DEPRECATED [[deprecated]]
#else
#define LIBOPENMPT_ATTR_DEPRECATED
#endif

#endif


/* clang-format on */

#include "libopenmpt_version.h"

#endif /* LIBOPENMPT_CONFIG_H */
