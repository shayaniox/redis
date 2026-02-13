#include <stdlib.h>

size_t find_next_cap(size_t cap)
{
    for (size_t i = 0; i < 5; i++)
        cap |= cap >> (1 << i);
    cap++;

    return cap;
}
