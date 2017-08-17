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


#include "CChapMd5.h"
#include "openssl/md5.h"
#include "openssl/md4.h"
#include <string.h>

#define MD5_HASH_SIZE       16
#define MD5_MIN_CHALLENGE   16
#define MD5_MAX_CHALLENGE   24

CChapMd5::CChapMd5(CChap *pnext)
    :CChap(CHAP_MD5, pnext)
{
}

CChapMd5::~CChapMd5()
{
}

//Generate Challenge
void CChapMd5::GenerateChallenge(unsigned char *challenge)
{
    int clen;
    clen = (int)(drand48() * (MD5_MAX_CHALLENGE - MD5_MIN_CHALLENGE))
    + MD5_MIN_CHALLENGE;
    *challenge++ = clen;
    random_bytes(challenge, clen);

}

//Verify Response
int CChapMd5::VerifyResponse(int id, char *name,
    unsigned char *secret, int secret_len,
    unsigned char *challenge, unsigned char *response,
    char *message, int message_space)
{
    MD5_CTX ctx;
    unsigned char idbyte = id;
    unsigned char hash[MD5_HASH_SIZE];
    int challenge_len, response_len;

    challenge_len = *challenge++;
    response_len = *response++;
    if (response_len == MD5_HASH_SIZE) 
    {
        /* Generate hash of ID, secret, challenge */
        MD5_Init(&ctx);
        MD5_Update(&ctx, &idbyte, 1);
        MD5_Update(&ctx, secret, secret_len);
        MD5_Update(&ctx, challenge, challenge_len);
        MD5_Final(hash, &ctx);

        /* Test if our hash matches the peer's response */
        if (memcmp(hash, response, MD5_HASH_SIZE) == 0) {
            snprintf(message, message_space, "Access granted");
            return 1;
        }
    }
    snprintf(message, message_space, "Access denied");
    return 0;

}

//Make Response
void CChapMd5::MakeResponse(unsigned char *response, int id, char *our_name,
    unsigned char *challenge, char *secret, int secret_len,
    unsigned char *priv)
{
    MD5_CTX ctx;
    unsigned char idbyte = id;
    int challenge_len = *challenge++;

    MD5_Init(&ctx);
    MD5_Update(&ctx, &idbyte, 1);
    MD5_Update(&ctx, (u_char *)secret, secret_len);
    MD5_Update(&ctx, challenge, challenge_len);
    MD5_Final(&response[1], &ctx);
    response[0] = MD5_HASH_SIZE;
}

//Check for Success
int CChapMd5::CheckSuccess(int id, unsigned char *pkt, int len)
{
    return 1;
}

//Failure Handle
void CChapMd5::HandleFailure(unsigned char *pkt, int len)
{
    return;
}



