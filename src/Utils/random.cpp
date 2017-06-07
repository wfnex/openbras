/*
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
*/

#include "random.h"

void
magic_init()
{
    long seed;
    struct timeval t;

    gettimeofday(&t, NULL);
    seed = t.tv_sec ^ t.tv_usec;
    srand48(seed);
}

/*
 * magic - Returns the next magic number.
 */
uint32_t
magic()
{
    return (uint32_t) mrand48();
}

/*
 * random_bytes - Fill a buffer with random bytes.
 */
void
random_bytes(unsigned char *buf, int len)
{
    int i;

    for (i = 0; i < len; ++i)
        buf[i] = mrand48() >> 24;
}

double
drand48()
{
    return (double)rand() / (double)0x7fffffffL; /* 2**31-1 */
}

long
mrand48()
{
    return rand();
}

void
srand48(long seedval)
{
    srand((int)seedval);
}


