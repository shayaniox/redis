#ifndef _SLIST_H
#define _SLIST_H

#include "estring.h"
#include <stddef.h>

struct slist {
    struct string **data;
    size_t cap;
    size_t len;
};

struct slist *new_slist(string_t *data, size_t len);
struct slist *slist_append(struct slist *list, string_t data);
int slist_free(struct slist *list);
void pslist(struct slist *list);

#endif
