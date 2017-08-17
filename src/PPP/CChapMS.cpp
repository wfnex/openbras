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


#include "CChapMS.h"

CChapMs::CChapMs(int code, CChap *pnext)
    :CChap(code, pnext)
{
}

CChapMs::~CChapMs()
{
}

//Generate Challenge
void CChapMs::GenerateChallenge(unsigned char *challenge)
{
    *challenge++ = 8;
    random_bytes(challenge, 8);
}

//Verify Response
int CChapMs::VerifyResponse(int id, char *name,
    unsigned char *secret, int secret_len,
    unsigned char *challenge, unsigned char *response,
    char *message, int message_space)
{
    unsigned char md[MS_CHAP_RESPONSE_LEN] ={0};
    int diff;
    int challenge_len, response_len;

    challenge_len = *challenge++;   /* skip length, is 8 */
    response_len = *response++;
    if (response_len != MS_CHAP_RESPONSE_LEN)
        goto bad;

#ifndef MSLANMAN
    if (!response[MS_CHAP_USENT]) {
        /* Should really propagate this into the error packet. */
        goto bad;
    }
#endif

    /* Generate the expected response. */
    ChapMS(challenge, (char *)secret, secret_len, md);

#ifdef MSLANMAN
    /* Determine which part of response to verify against */
    if (!response[MS_CHAP_USENT])
        diff = memcmp(&response[MS_CHAP_LANMANRESP],
                  &md[MS_CHAP_LANMANRESP], MS_CHAP_LANMANRESP_LEN);
    else
#endif
        diff = memcmp(&response[MS_CHAP_NTRESP], &md[MS_CHAP_NTRESP],
                  MS_CHAP_NTRESP_LEN);

    if (diff == 0) {
        snprintf(message, message_space, "Access granted");
        return 1;
    }

 bad:
    /* See comments below for MS-CHAP V2 */
    snprintf(message, message_space, "E=691 R=1 C=%0.*B V=0",
         challenge_len, challenge);
    return 0;

}

//Make Response
void CChapMs::MakeResponse(unsigned char *response, int id, char *our_name,
    unsigned char *challenge, char *secret, int secret_len,
    unsigned char *priv)
{
    challenge++;	/* skip length, should be 8 */
    *response++ = MS_CHAP_RESPONSE_LEN;
    ChapMS(challenge, secret, secret_len, response);
}

//Check for Success
int CChapMs::CheckSuccess(int id, unsigned char *pkt, int len)
{
    return 1;
}

//Failure Handle
void CChapMs::HandleFailure(unsigned char *inp, int len)
{
    int err;
    char *p, *msg;

    /* We want a null-terminated string for strxxx(). */
    msg = (char *)malloc(len + 1);
    if (!msg) {
        ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("Out of memory in chapms_handle_failure\n")));
        return;
    }
    BCOPY(inp, msg, len);
    msg[len] = 0;
    p = msg;

    /*
     * Deal with MS-CHAP formatted failure messages; just print the
     * M=<message> part (if any).  For MS-CHAP we're not really supposed
     * to use M=<message>, but it shouldn't hurt.  See
     * chapms[2]_verify_response.
     */
    if (!strncmp(p, "E=", 2))
        err = strtol(p+2, NULL, 10); /* Remember the error code. */
    else
        goto print_msg; /* Message is badly formatted. */

    if (len && ((p = strstr(p, " M=")) != NULL)) {
        /* M=<message> field found. */
        p += 3;
    } else {
        /* No M=<message>; use the error code. */
        switch (err) {
        case MS_CHAP_ERROR_RESTRICTED_LOGON_HOURS:
            p = "E=646 Restricted logon hours";
            break;

        case MS_CHAP_ERROR_ACCT_DISABLED:
            p = "E=647 Account disabled";
            break;

        case MS_CHAP_ERROR_PASSWD_EXPIRED:
            p = "E=648 Password expired";
            break;

        case MS_CHAP_ERROR_NO_DIALIN_PERMISSION:
            p = "E=649 No dialin permission";
            break;

        case MS_CHAP_ERROR_AUTHENTICATION_FAILURE:
            p = "E=691 Authentication failure";
            break;

        case MS_CHAP_ERROR_CHANGING_PASSWORD:
            /* Should never see this, we don't support Change Password. */
            p = "E=709 Error changing password";
            break;

        default:
            free(msg);
            return;
        }
    }
print_msg:
    free(msg);
}


void
CChapMs::ChapMS(u_char *rchallenge, char *secret, int secret_len,
       unsigned char *response)
{
    BZERO(response, MS_CHAP_RESPONSE_LEN);

    ChapMS_NT(rchallenge, secret, secret_len, &response[MS_CHAP_NTRESP]);

#ifdef MSLANMAN
    ChapMS_LANMan(rchallenge, secret, secret_len,
		  &response[MS_CHAP_LANMANRESP]);

    /* preferred method is set by option  */
    response[MS_CHAP_USENT] = !ms_lanman;
#else
    response[MS_CHAP_USENT] = 1;
#endif
}

void
CChapMs::ChapMS_NT(u_char *rchallenge, char *secret, int secret_len,
    u_char NTResponse[24])
{
    u_char	    unicodePassword[MAX_NT_PASSWORD * 2] ={0};
    u_char	PasswordHash[MD4_SIGNATURE_SIZE] ={0};

    /* Hash the Unicode version of the secret (== password). */
    ascii2unicode(secret, secret_len, unicodePassword);
    NTPasswordHash(unicodePassword, secret_len * 2, PasswordHash);

    ChallengeResponse(rchallenge, PasswordHash, NTResponse);
}

//Challenge Response
void
CChapMs::ChallengeResponse(u_char *challenge,
      u_char PasswordHash[MD4_SIGNATURE_SIZE],
      u_char response[24])
{
    u_char    ZPasswordHash[21] ={0};

    BZERO(ZPasswordHash, sizeof(ZPasswordHash));
    BCOPY(PasswordHash, ZPasswordHash, MD4_SIGNATURE_SIZE);

    (void) DesSetkey(ZPasswordHash + 0);
    DesEncrypt(challenge, response + 0);
    (void) DesSetkey(ZPasswordHash + 7);
    DesEncrypt(challenge, response + 8);
    (void) DesSetkey(ZPasswordHash + 14);
    DesEncrypt(challenge, response + 16);
}

//NT Password Hash
void
CChapMs::NTPasswordHash(u_char *secret, int secret_len, u_char hash[MD4_SIGNATURE_SIZE])
{

    int mdlen = secret_len * 8;

    MD4_CTX md4Context;

    MD4_Init(&md4Context);
    /* MD4Update can take at most 64 bytes at a time */
    while (mdlen > 512) {
        MD4_Update(&md4Context, secret, 512);
        secret += 64;
        mdlen -= 512;
    }
    MD4_Update(&md4Context, secret, mdlen);
    MD4_Final(hash, &md4Context);
}


//ascii unicode
void
CChapMs::ascii2unicode(char ascii[], int ascii_len, u_char unicode[])
{
    int i;

    BZERO(unicode, ascii_len * 2);
    for (i = 0; i < ascii_len; i++)
        unicode[i * 2] = (u_char) ascii[i];
}




