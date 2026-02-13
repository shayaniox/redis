#include "slist.h"
#include "estring.h"
#include "log.h"
#include "util.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct slist *new_slist(string_t *data, size_t len)
{
    struct slist *list = calloc(sizeof(struct slist), 1);
    if (list == NULL)
        return NULL;

    if (data && len) {
        list->cap = list->len = len;
        list->data = calloc(sizeof(string_t), len);
        memcpy(list->data, data, sizeof(string_t) * len);
    }

    return list;
}

struct slist *slist_append(struct slist *list, string_t data)
{
    if (list->cap == list->len) {
        list->cap = find_next_cap(list->cap);
        list->data = reallocarray(list->data, list->cap, sizeof(string_t));
        if (list->data == NULL) return NULL;
    }
    list->data[list->len] = data;
    list->len++;

    return list;
}

int slist_free(struct slist *list)
{
    if (list == NULL)
        return 0;
    for (size_t i = 0; i < list->len; i++) {
        str_free(list->data[i]);
    }
    free(list->data);
    free(list);

    return 0;
}

void pslist(struct slist *list)
{
    printf("[");
    for (size_t i = 0; i < list->len; i++)
        printf("\t%s\n", list->data[i]->data);
    printf("]");
}
