#include "serialize.h"
#include "estring.h"
#include <stddef.h>
#include <stdint.h>

void wnil(string_t resp)
{
    char type[] = {(char)SERNIL};
    str_append(resp, (char *)type, 1);
}

void werr(string_t resp, int errcode, char *msg, size_t msglen)
{
    char type[] = {(char)SERERR};
    str_append(resp, (char *)type, 1);
    str_append(resp, (char *)&errcode, 4);
    uint32_t len = (uint32_t)msglen; // NOTE: I guess `len` type must be uint64_t
    str_append(resp, (char *)&len, 4);
    str_append(resp, msg, msglen);
}

void wint(string_t resp, int n)
{
    char type[] = {(char)SERINT};
    str_append(resp, (char *)type, 1);
    str_append(resp, (char *)&n, 4);
}

void wstr(string_t resp, string_t str)
{
    char type[] = {(char)SERSTR};
    str_append(resp, (char *)type, 1);
    uint32_t len = (uint32_t)str->len; // NOTE: I guess `len` type must be uint64_t
    str_append(resp, (char *)&len, 4);
    str_append(resp, str->data, str->len);
}

void warr(string_t resp, uint32_t n)
{
    char type[] = {(char)SERARR};
    str_append(resp, (char *)type, 1);
    str_append(resp, (char *)&n, 4);
}
