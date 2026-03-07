#include "hashtable.h"
#include "estring.h"
#include "hashfunc.h"
#include "log.h"
#include "slist.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int resize_count = 128;
const int k_max_load_factor = 8;

struct HashNode *node_create(const char *key, const char *val)
{
    assert(key != NULL);
    struct HashNode *node = calloc(1, sizeof(struct HashNode));
    node->hcode = djb2(key);
    node->key = str_from(key);
    if (val)
        node->value = str_from(val);
    return node;
}

void node_free(struct HashNode *node)
{
    str_free(node->key);
    str_free(node->value);
    free(node);
}

static void h_init(struct HashMap *hm, size_t n)
{
    assert((n & (n - 1)) == 0);
    if (!hm) {
        return;
    }
    hm->buckets = calloc(n, sizeof(struct HashNode *));
    hm->mask = n - 1;
    hm->size = 0;
}

static void h_insert(struct HashMap *hm, struct HashNode *node)
{
    size_t pos = node->hcode & hm->mask;
    struct HashNode *next = hm->buckets[pos];
    node->next = next;
    hm->buckets[pos] = node;
    hm->size++;
}

static struct HashNode **h_lookup(struct HashMap *hm, struct HashNode *node)
{
    if (hm->buckets == NULL) {
        warn("hm buckest is null");
        return NULL;
    }
    size_t pos = node->hcode & hm->mask;
    struct HashNode **from = &hm->buckets[pos];
    while (*from) {
        if (node->hcode == (*from)->hcode)
            return from;
        from = &(*from)->next;
    }

    return NULL;
}

static struct HashNode *h_detach(struct HashMap *hm, struct HashNode **from)
{
    struct HashNode *node = *from;
    *from = (*from)->next;
    hm->size--;
    return node;
}

void h_clear(struct HashMap *hm)
{
    size_t pos = 0;
    struct HashNode *node = hm->buckets[pos];
    struct HashNode *next;
    while (pos < hm->mask + 1) {
        if (!node) {
            pos++;
            node = (pos < hm->mask + 1) ? hm->buckets[pos] : NULL;
            continue;
        }
        next = node->next;
        node_free(node);
        node = next;
    }
}

struct slist *h_scan(struct HashMap *h)
{
    struct slist *keys = new_slist(NULL, 0);
    if (!h->buckets) {
        return keys;
    }

    size_t pos = 0;
    struct HashNode *node = h->buckets[pos];
    while (pos < (h->mask + 1)) {
        if (!node) {
            pos++;
            node = pos < (h->mask + 1) ? h->buckets[pos] : NULL;
            continue;
        }
        slist_append(keys, str_dup(node->key));
        node = node->next;
    }

    return keys;
}

static void ht_resize_init(struct HashTable *ht)
{
    assert(ht->temp.buckets == NULL);
    ht->temp = ht->main;
    h_init(&ht->main, (ht->main.mask + 1) << 1);
    ht->resizing_pos = 0;
}

static void ht_resize(struct HashTable *ht)
{
    if (!ht->temp.buckets)
        return;

    int n = 0;
    struct HashNode **from = ht->temp.buckets;
    while (ht->resizing_pos < ht->temp.mask + 1 && n < resize_count) {
        if (!(*from)) {
            ht->resizing_pos++;
            continue;
        }
        struct HashNode *node = h_detach(&ht->temp, from);
        h_insert(&ht->main, node);
    }
    info("Moved %d keys", n);
}

void ht_insert(struct HashTable *ht, struct HashNode *node)
{
    if (!ht->main.buckets) {
        h_init(&ht->main, 4);
    }

    struct HashNode **from = h_lookup(&ht->main, node);
    if (from) {
        struct HashNode *oldnode = *from;
        *from = node;
        node->next = oldnode->next;
        node_free(oldnode);
    }
    else {
        h_insert(&ht->main, node);
    }

    if (ht->temp.buckets == NULL) {
        int load_factor = ht->main.size / (ht->main.mask + 1);
        if (load_factor >= k_max_load_factor) {
            ht_resize_init(ht);
        }
    }
    else {
        struct HashNode **from = h_lookup(&ht->temp, node);
        if (from) {
            node_free(*from);
        }
    }
    ht_resize(ht);
}

struct HashNode *ht_pop(struct HashTable *ht, struct HashNode *node)
{
    ht_resize(ht);

    struct HashNode **result;
    result = h_lookup(&ht->temp, node);
    if (result) {
        return h_detach(&ht->temp, result);
    }

    result = h_lookup(&ht->main, node);
    if (result) {
        return h_detach(&ht->main, result);
    }

    return NULL;
}

struct HashNode *ht_lookup(struct HashTable *ht, struct HashNode *node)
{
    ht_resize(ht);

    struct HashNode **result = h_lookup(&ht->main, node);
    if (!result) {
        error("looking for node in temp");
        result = h_lookup(&ht->temp, node);
    }

    return result ? *result : NULL;
}

void ht_print(struct HashTable *ht)
{
    size_t pos = 0;
    struct HashNode *node = ht->main.buckets[pos];
    while (pos < (ht->main.mask + 1)) {
        if (!node) {
            pos++;
            node = pos < (ht->main.mask + 1) ? ht->main.buckets[pos] : NULL;
            printf("-----------------------------\n");
            continue;
        }
        printf("[%s] -> [%s]\n", node->key->data, node->value->data);
        node = node->next;
    }
}

void ht_clear(struct HashTable *ht)
{
    if (ht->temp.buckets) {
        h_clear(&ht->temp);
        free(ht->temp.buckets);
    }
    if (ht->main.buckets) {
        h_clear(&ht->main);
        free(ht->main.buckets);
    }
}

struct slist *ht_scan(struct HashTable *ht)
{
    struct slist *keys = h_scan(&ht->main);
    if (ht->temp.buckets) {
        struct slist *tempkeys = h_scan(&ht->main);
        for (size_t i = 0; i < tempkeys->len; i++)
            slist_append(keys, str_dup(tempkeys->data[i]));
    }

    return keys;
}
