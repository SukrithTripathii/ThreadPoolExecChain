#include <intrin.h>
#include "Rand.h"

unsigned int RandintMod3()
{
    unsigned __int64 t = __rdtsc();

    // Mix bits a little
    t ^= (t >> 17);
    t ^= (t << 31);
    t ^= (t >> 8);

    return (unsigned int)(t % 3);
}