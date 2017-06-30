/***********************************************************************
	Copyright (c) 2017, The OpenBRAS project authors. All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are
	met:

	  * Redistributions of source code must retain the above copyright
		notice, this list of conditions and the following disclaimer.

	  * Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in
		the documentation and/or other materials provided with the
		distribution.

	  * Neither the name of OpenBRAS nor the names of its contributors may
		be used to endorse or promote products derived from this software
		without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**********************************************************************/

/*
** ********************************************************************
** md4.h -- Header file for implementation of                        **
** MD4 Message Digest Algorithm                                      **
** Updated: 2/13/90 by Ronald L. Rivest                              **
** (C) 1990 RSA Data Security, Inc.                                  **
** ********************************************************************
*/

#include <stdint.h>


/* MDstruct is the data structure for a message digest computation.
*/
typedef struct {
	unsigned int buffer[4]; /* Holds 4-word result of MD computation */
	unsigned char count[8]; /* Number of bits processed so far */
	unsigned int done;      /* Nonzero means MD computation finished */
} MD4_CTXEx;

/* MD4Init(MD4_CTX *)
** Initialize the MD4_CTX prepatory to doing a message digest
** computation.
*/
void MD4Init(MD4_CTXEx *MD);

/* MD4Update(MD,X,count)
** Input: X -- a pointer to an array of unsigned characters.
**        count -- the number of bits of X to use (an unsigned int).
** Updates MD using the first "count" bits of X.
** The array pointed to by X is not modified.
** If count is not a multiple of 8, MD4Update uses high bits of
** last byte.
** This is the basic input routine for a user.
** The routine terminates the MD computation when count < 512, so
** every MD computation should end with one call to MD4Update with a
** count less than 512.  Zero is OK for a count.
*/
void MD4Update(MD4_CTXEx *MD, unsigned char *X, unsigned int count);

/* MD4Print(MD)
** Prints message digest buffer MD as 32 hexadecimal digits.
** Order is from low-order byte of buffer[0] to high-order byte
** of buffer[3].
** Each byte is printed with high-order hexadecimal digit first.
*/
void MD4Print(MD4_CTXEx *);

/* MD4Final(buf, MD)
** Returns message digest from MD and terminates the message
** digest computation.
*/
void MD4Final(unsigned char *, MD4_CTXEx *);

/*
** End of md4.h
****************************(cut)***********************************/

