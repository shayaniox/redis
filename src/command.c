#include "command.h"
#include "estring.h"
#include "hashtable.h"
#include "serialize.h"
#include <stddef.h>
#include <string.h>

enum res_code get(struct HashTable *ht, string_t key, string_t resp)
{
    struct HashNode *temp = node_create(key->data, NULL);
    struct HashNode *node = ht_lookup(ht, temp);
    node_free(temp);
    if (!node) {
        char *msg = "your fucking key not found, you idiot! [command: get]";
        werr(resp, ERR_NOTFOUND, msg, strlen(msg));
        return RES_NX;
    }
    wstr(resp, node->value);

    return RES_OK;
}

enum res_code set(struct HashTable *ht, string_t key, string_t value, string_t resp)
{
    struct HashNode *node = node_create(key->data, value->data);
    ht_insert(ht, node);
    wnil(resp);

    return RES_OK;
}

enum res_code del(struct HashTable *ht, string_t key, string_t resp)
{
    struct HashNode *temp = node_create(key->data, NULL);
    struct HashNode *node = ht_pop(ht, temp);
    if (!node) {
        char *msg = "your fucking key not found, you idiot! [command: del]";
        werr(resp, ERR_NOTFOUND, msg, strlen(msg));
        return RES_NX;
    }
    node_free(temp);
    node_free(node);
    wnil(resp);

    return RES_OK;
}

enum res_code keys(struct HashTable *ht, string_t resp)
{
    struct slist *keys = ht_scan(ht);
    warr(resp, keys->len);
    for (size_t i = 0; i < keys->len; i++) {
        wstr(resp, keys->data[i]);
    }
    slist_free(keys);

    return RES_OK;
}
