#ifndef _COMMAND_H
#define _COMMAND_H

#include "estring.h"
#include "hashtable.h"

enum res_code {
    RES_OK = 0,
    RES_ERR = 1,
    RES_NX = 2,
};

enum err_code {
    ERR_NOTFOUND = 0,
    ERR_UNKONWN = 1,
    ERR_TOOBIG = 2,
};

enum res_code get(struct HashTable *ht, string_t key, string_t resp);
enum res_code set(struct HashTable *ht, string_t key, string_t value, string_t resp);
enum res_code del(struct HashTable *ht, string_t key, string_t resp);
enum res_code keys(struct HashTable *ht, string_t resp);

#endif /* ifndef _COMMAND_H */
