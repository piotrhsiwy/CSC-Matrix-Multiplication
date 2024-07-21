#include "compare.h"

#include <stdint.h>

int cmp_u64(const void* const aptr, const void* const bptr)
{
    const uint64_t a = *(uint64_t*)aptr;
    const uint64_t b = *(uint64_t*)bptr;
    return a > b ? 1 : (a < b ? -1 : 0);
}
