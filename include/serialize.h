#ifndef _SERIALIZE_H
#define _SERIALIZE_H

#include "estring.h"
#include <stddef.h>
#include <stdint.h>

enum {
    SERNIL,
    SERERR,
    SERINT,
    SERSTR,
    SERARR,
};

void wnil(string_t resp);
void werr(string_t resp, int errcode, char *msg, size_t msglen);
void wint(string_t resp, int n);
void wstr(string_t resp, string_t str);
void warr(string_t resp, uint32_t n);

#endif
