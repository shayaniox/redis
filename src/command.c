#include "command.h"
#include "estring.h"
#include "log.h"

enum res_code get(string_t key, string_t value)
{
    info("get: [%s]", key->data);
    str_set(value, "value", 5);
    return RES_OK;
}

enum res_code set(string_t key, string_t value)
{
    info("set: [%s] to [%s]", key->data, value->data);
    return RES_ERR;
}

enum res_code del(string_t key)
{
    info("del: [%s]", key->data);
    return RES_NX;
}
