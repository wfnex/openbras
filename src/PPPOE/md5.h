#ifndef MD5_H
#define MD5_H
/*
 * LIC: GPL
 */

#include "BaseDefines.h"

struct MD5Context
{
	WORD32 buf[4];
	WORD32 bits[2];
	BYTE in[64];
};

void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context *context, BYTE const *buf, WORD32 len);
void MD5Final(BYTE digest[16], struct MD5Context *context);
void MD5Transform(WORD32 buf[4], WORD32 const in[16]);

/*
 * This is needed to make RSAREF happy on some MS-DOS compilers.
 */
//typedef struct MD5Context MD5_CTX;

#endif /* !MD5_H */
