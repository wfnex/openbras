/*
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
*/

#include "CChapMd5.h"
#include "md5.h"
#include "md4.h"
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

void CChapMd5::GenerateChallenge(unsigned char *challenge)
{
    int clen;
    clen = (int)(drand48() * (MD5_MAX_CHALLENGE - MD5_MIN_CHALLENGE))
    + MD5_MIN_CHALLENGE;
    *challenge++ = clen;
    random_bytes(challenge, clen);

}

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

int CChapMd5::CheckSuccess(int id, unsigned char *pkt, int len)
{
    return 1;
}

void CChapMd5::HandleFailure(unsigned char *pkt, int len)
{
    return;
}



