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
 * duhtag.c - Function to return the tags stored      / / \  \
 *            in a DUH struct (typically author      | <  /   \_
 *            information).                          |  \/ /\   /
 *                                                    \_  /  > /
 * By entheh.                                           | \ / /
 *                                                      |  ' /
 *                                                       \__/
 */

#include <string.h>

#include "dumb.h"
#include "internal/dumb.h"

const char *duh_get_tag(DUH *duh, const char *key) {
    int i;
    ASSERT(key);
    if (!duh || !duh->tag)
        return NULL;

    for (i = 0; i < duh->n_tags; i++)
        if (strcmp(key, duh->tag[i][0]) == 0)
            return duh->tag[i][1];

    return NULL;
}

int duh_get_tag_iterator_size(DUH *duh) {
    return (duh && duh->tag ? duh->n_tags : 0);
}

int duh_get_tag_iterator_get(DUH *duh, const char **key, const char **tag,
                             int i) {
    ASSERT(key);
    ASSERT(tag);
    if (!duh || !duh->tag || i >= duh->n_tags)
        return -1;

    *key = duh->tag[i][0];
    *tag = duh->tag[i][1];

    return 0;
}
