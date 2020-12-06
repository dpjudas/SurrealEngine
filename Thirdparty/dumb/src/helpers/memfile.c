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
 * memfile.c - Module for reading data from           / / \  \
 *             memory using a DUMBFILE.              | <  /   \_
 *                                                   |  \/ /\   /
 * By entheh.                                         \_  /  > /
 *                                                      | \ / /
 *                                                      |  ' /
 *                                                       \__/
 */

#include <stdlib.h>
#include <string.h>

#include "dumb.h"

typedef struct MEMFILE MEMFILE;

struct MEMFILE {
    const char *ptr, *ptr_begin;
    size_t left, size;
};

static int dumb_memfile_skip(void *f, dumb_off_t n) {
    MEMFILE *m = f;
    if (n > (dumb_off_t)m->left)
        return -1;
    m->ptr += n;
    m->left -= n;
    return 0;
}

static int dumb_memfile_getc(void *f) {
    MEMFILE *m = f;
    if (m->left <= 0)
        return -1;
    m->left--;
    return *(const unsigned char *)m->ptr++;
}

static dumb_ssize_t dumb_memfile_getnc(char *ptr, size_t n, void *f) {
    MEMFILE *m = f;
    if (n > m->left)
        n = m->left;
    memcpy(ptr, m->ptr, n);
    m->ptr += n;
    m->left -= n;
    return n;
}

static void dumb_memfile_close(void *f) { free(f); }

static int dumb_memfile_seek(void *f, dumb_off_t n) {
    MEMFILE *m = f;

    m->ptr = m->ptr_begin + n;
    m->left = m->size - n;

    return 0;
}

static dumb_off_t dumb_memfile_get_size(void *f) {
    MEMFILE *m = f;
    return m->size;
}

static const DUMBFILE_SYSTEM memfile_dfs = {NULL,
                                            &dumb_memfile_skip,
                                            &dumb_memfile_getc,
                                            &dumb_memfile_getnc,
                                            &dumb_memfile_close,
                                            &dumb_memfile_seek,
                                            &dumb_memfile_get_size};

DUMBFILE *dumbfile_open_memory(const char *data, size_t size) {
    MEMFILE *m = malloc(sizeof(*m));
    if (!m)
        return NULL;

    m->ptr_begin = data;
    m->ptr = data;
    m->left = size;
    m->size = size;

    return dumbfile_open_ex(m, &memfile_dfs);
}
