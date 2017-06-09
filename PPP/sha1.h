/* sha1.h */
/*
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
*/

#ifndef __SHA1_INCLUDE_
#include "pppdef.h"


#ifndef SHA1_SIGNATURE_SIZE
#ifdef SHA_DIGESTSIZE
#define SHA1_SIGNATURE_SIZE SHA_DIGESTSIZE
#else
#define SHA1_SIGNATURE_SIZE 20
#endif
#endif

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} SHA1_CTX;

void SHA1_Init(SHA1_CTX *);
void SHA1_Update(SHA1_CTX *, const unsigned char *, unsigned int);
void SHA1_Final(unsigned char[SHA1_SIGNATURE_SIZE], SHA1_CTX *);

#define __SHA1_INCLUDE_
#endif /* __SHA1_INCLUDE_ */

