#ifndef _COMMAND_H
#define _COMMAND_H

#include "estring.h"

enum res_code {
    RES_OK = 0,
    RES_ERR = 1,
    RES_NX = 2,
};

enum res_code get(string_t key, string_t value);
enum res_code set(string_t key, string_t value);
enum res_code del(string_t key);

#endif /* ifndef _COMMAND_H */
