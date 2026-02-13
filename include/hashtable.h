#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include "estring.h"
#include <stddef.h>
#include <stdint.h>

struct HashNode {
    struct HashNode *next;
    uint64_t hcode;
    string_t key;
    string_t value;
};

struct HashNode *node_create(const char *key, const char *val);
void node_free(struct HashNode *node);

struct HashMap {
    struct HashNode **buckets;
    size_t mask;
    size_t size;
};

struct HashTable {
    struct HashMap main;
    struct HashMap temp;
    size_t resizing_pos;
};

void ht_insert(struct HashTable *ht, struct HashNode *node);
struct HashNode *ht_pop(struct HashTable *ht, struct HashNode *node);
struct HashNode *ht_lookup(struct HashTable *ht, struct HashNode *node);
void ht_print(struct HashTable *ht, struct HashNode *node);
void ht_clear(struct HashTable *ht);

#define container_of(ptr, type, member) ({          \
    typeof(((type *)0)->member) *__mptr = (ptr);    \
    (type *)((void *)ptr - offsetof(type, member)); \
})

typedef struct Entry {
    struct HashNode node;
    string_t key;
    string_t value;
} Entry;

#endif
