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
 * packfile.c - Packfile input module.                / / \  \
 *                                                   | <  /   \_
 * By entheh.                                        |  \/ /\   /
 *                                                    \_  /  > /
 * Note that this does not use file compression;        | \ / /
 * for that you must open the file yourself and         |  ' /
 * then use dumbfile_open_packfile().                    \__/
 */

#include <allegro.h>

#include "aldumb.h"

typedef struct dumb_packfile {
    PACKFILE *p;
    long size;
    long pos;
} dumb_packfile;

static void *dumb_packfile_open_ex(PACKFILE *p, long size) {
    dumb_packfile *file = (dumb_packfile *)malloc(sizeof(dumb_packfile));
    if (!file)
        return 0;
    file->p = p;
    file->size = size;
    file->pos = 0;
    return file;
}

static void *dumb_packfile_open(const char *filename) {
    PACKFILE *p = pack_fopen(filename, F_READ);
    if (p)
        return dumb_packfile_open_ex(p, file_size_ex(filename));
    else
        return 0;
}

static int dumb_packfile_skip(void *f, dumb_off_t n) {
    dumb_packfile *file = (dumb_packfile *)f;
    file->pos += n;
    return pack_fseek(file->p, file->pos);
}

static int dumb_packfile_getc(void *f) {
    dumb_packfile *file = (dumb_packfile *)f;
    int c = pack_getc(file->p);
    if (c != EOF)
        file->pos++;
    return c;
}

static dumb_ssize_t dumb_packfile_getnc(char *ptr, size_t n, void *f) {
    dumb_packfile *file = (dumb_packfile *)f;
    errno = 0;
    long nr = pack_fread(ptr, n, file->p);
    if (nr > 0) {
        file->pos += nr;
        return nr;
    }
    return errno != 0 ? -1 : 0;
}

static void dumb_packfile_close(void *f) {
    dumb_packfile *file = (dumb_packfile *)f;
    pack_fclose(file->p);
    free(f);
}

static void dumb_packfile_noclose(void *f) { free(f); }

static int dumb_packfile_seek(void *f, dumb_off_t n) {
    dumb_packfile *file = (dumb_packfile *)f;
    file->pos = n;
    return pack_fseek(file->p, n);
}

static dumb_off_t dumb_packfile_get_size(void *f) {
    dumb_packfile *file = (dumb_packfile *)f;
    return file->size;
}

static DUMBFILE_SYSTEM packfile_dfs = {
    &dumb_packfile_open,    &dumb_packfile_skip,  &dumb_packfile_getc,
    &dumb_packfile_getnc,   &dumb_packfile_close, &dumb_packfile_seek,
    &dumb_packfile_get_size};

void dumb_register_packfiles(void) { register_dumbfile_system(&packfile_dfs); }

static DUMBFILE_SYSTEM packfile_dfs_leave_open = {NULL,
                                                  &dumb_packfile_skip,
                                                  &dumb_packfile_getc,
                                                  &dumb_packfile_getnc,
                                                  &dumb_packfile_noclose,
                                                  &dumb_packfile_seek,
                                                  &dumb_packfile_get_size};

/* XXX no way to get the file size from an existing PACKFILE without reading the
 * entire contents first */

DUMBFILE *dumbfile_open_packfile(PACKFILE *p) {
    return dumbfile_open_ex(dumb_packfile_open_ex(p, 0x7fffffff),
                            &packfile_dfs_leave_open);
}

DUMBFILE *dumbfile_from_packfile(PACKFILE *p) {
    return p ? dumbfile_open_ex(dumb_packfile_open_ex(p, 0x7fffffff),
                                &packfile_dfs)
             : NULL;
}
