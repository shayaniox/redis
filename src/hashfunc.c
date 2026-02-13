#include <stdint.h>

uint64_t djb2(const char *str)
{
    uint64_t hash = 5381;
    char ch;
    while ((ch = *str++))
        hash = (hash << 5) + hash + ch;
    return hash;
}
