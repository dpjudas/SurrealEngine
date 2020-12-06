/*  _______         ____    __         ___    ___
 * \    _  \       \    /  \  /       \   \  /   /       '   '  '
 *  |  | \  \       |  |    ||         |   \/   |         .      .
 *  |  |  |  |      |  |    ||         ||\  /|  |
 *  |  |  |  |      |  |    ||         || \/ |  |         '  '  '
 *  |  |  |  |      |  |    ||         ||    |  |         .      .
 *  |  |_/  /        \  \__//          ||    |  |
 * /_______/ynamic    \____/niversal  /__\  /____\usic   /|  .  . ibliotheque
 *                                                      /  \
 *                                                     / .  \
 * internal/dumb.h - DUMB's internal declarations.    / / \  \
 *                                                   | <  /   \_
 * This header file provides access to the           |  \/ /\   /
 * internal structure of DUMB, and is liable          \_  /  > /
 * to change, mutate or cease to exist at any           | \ / /
 * moment. Include it at your own peril.                |  ' /
 *                                                       \__/
 * ...
 *
 * Seriously. You don't need access to anything in this file. All right, you
 * probably do actually. But if you use it, you will be relying on a specific
 * version of DUMB, so please check DUMB_VERSION defined in dumb.h. Please
 * contact the authors so that we can provide a public API for what you need.
 */

#ifndef INTERNAL_DUMB_H
#define INTERNAL_DUMB_H

#include "../dumb.h"

#undef MIN
#undef MAX
#undef MID

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MID(x, y, z) MAX((x), MIN((y), (z)))

#undef ABS
#define ABS(x) (((x) >= 0) ? (x) : (-(x)))

#ifndef LONG_LONG
#if defined __GNUC__ || defined __INTEL_COMPILER || defined __MWERKS__ ||      \
    defined __sgi
#define LONG_LONG long long
#elif defined _MSC_VER || defined __WATCOMC__
#define LONG_LONG __int64
#else
#error 64-bit integer type unknown
#endif
#endif

typedef struct DUH_SIGTYPE_DESC_LINK {
    struct DUH_SIGTYPE_DESC_LINK *next;
    DUH_SIGTYPE_DESC *desc;
} DUH_SIGTYPE_DESC_LINK;

typedef struct DUH_SIGNAL {
    sigdata_t *sigdata;
    DUH_SIGTYPE_DESC *desc;
} DUH_SIGNAL;

struct DUH {
    dumb_off_t length;

    int n_tags;
    char *(*tag)[2];

    int n_signals;
    DUH_SIGNAL **signal;
};

DUH_SIGTYPE_DESC *_dumb_get_sigtype_desc(long type);

#endif /* INTERNAL_DUMB_H */
