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


#include "CChapMs2.h"

CChapMs2::CChapMs2(CChap *pnext)
    :CChapMs(CHAP_MICROSOFT_V2, pnext)
{
}

CChapMs2::~CChapMs2()
{
}

//Generate Challenge
void CChapMs2::GenerateChallenge(unsigned char *challenge)
{
    *challenge++ = 16;
    random_bytes(challenge, 16);
}

//Verify Response
int CChapMs2::VerifyResponse(int id, char *name,
    unsigned char *secret, int secret_len,
    unsigned char *challenge, unsigned char *response,
    char *message, int message_space)
{
   unsigned char md[MS_CHAP2_RESPONSE_LEN] ={0};
   char saresponse[MS_AUTH_RESPONSE_LENGTH+1] ={0};
   int challenge_len, response_len;

   challenge_len = *challenge++;   /* skip length, is 16 */
   response_len = *response++;
   if (response_len != MS_CHAP2_RESPONSE_LEN)
       goto bad;   /* not even the right length */

   /* Generate the expected response and our mutual auth. */
   ChapMS2(challenge, &response[MS_CHAP2_PEER_CHALLENGE], name,
       (char *)secret, secret_len, md,
       (unsigned char *)saresponse, MS_CHAP2_AUTHENTICATOR);

   /* compare MDs and send the appropriate status */
   /*
    * Per RFC 2759, success message must be formatted as
    *     "S=<auth_string> M=<message>"
    * where
    *     <auth_string> is the Authenticator Response (mutual auth)
    *     <message> is a text message
    *
    * However, some versions of Windows (win98 tested) do not know
    * about the M=<message> part (required per RFC 2759) and flag
    * it as an error (reported incorrectly as an encryption error
    * to the user).  Since the RFC requires it, and it can be
    * useful information, we supply it if the peer is a conforming
    * system.  Luckily (?), win98 sets the Flags field to 0x04
    * (contrary to RFC requirements) so we can use that to
    * distinguish between conforming and non-conforming systems.
    *
    * Special thanks to Alex Swiridov <say@real.kharkov.ua> for
    * help debugging this.
    */
   if (memcmp(&md[MS_CHAP2_NTRESP], &response[MS_CHAP2_NTRESP],
          MS_CHAP2_NTRESP_LEN) == 0) {
       if (response[MS_CHAP2_FLAGS])
           snprintf(message, message_space, "S=%s", saresponse);
       else
           snprintf(message, message_space, "S=%s M=%s",
                saresponse, "Access granted");
       return 1;
   }

bad:
   /*
    * Failure message must be formatted as
    *     "E=e R=r C=c V=v M=m"
    * where
    *     e = error code (we use 691, ERROR_AUTHENTICATION_FAILURE)
    *     r = retry (we use 1, ok to retry)
    *     c = challenge to use for next response, we reuse previous
    *     v = Change Password version supported, we use 0
    *     m = text message
    *
    * The M=m part is only for MS-CHAPv2.  Neither win2k nor
    * win98 (others untested) display the message to the user anyway.
    * They also both ignore the E=e code.
    *
    * Note that it's safe to reuse the same challenge as we don't
    * actually accept another response based on the error message
    * (and no clients try to resend a response anyway).
    *
    * Basically, this whole bit is useless code, even the small
    * implementation here is only because of overspecification.
    */
   snprintf(message, message_space, "E=691 R=1 C=%0.*B V=0 M=%s",
        challenge_len, challenge, "Access denied");
   return 0;

}

//Make Response
void CChapMs2::MakeResponse(unsigned char *response, 
                                    int id, 
                                    char *our_name,
                                    unsigned char *challenge, 
                                    char *secret, 
                                    int secret_len,
                                    unsigned char *priv)
{
    unsigned char auth_response[MS_AUTH_RESPONSE_LENGTH+2]={0};

    challenge++;    /* skip length, should be 16 */
    *response++ = MS_CHAP2_RESPONSE_LEN;

    ChapMS2(challenge,
        NULL,
        our_name, secret, secret_len, response, auth_response,
        MS_CHAP2_AUTHENTICATEE);
}

//Check for Success
int CChapMs2::CheckSuccess(int id, unsigned char *msg, int len)
{
    if ((len < MS_AUTH_RESPONSE_LENGTH + 2) ||strncmp((char *)msg, "S=", 2) != 0) 
    {
        /* Packet does not start with "S=" */
        ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("MS-CHAPv2 Success packet is badly formed\n")));
        return 0;
    }
    msg += 2;
    len -= 2;
    if (len < MS_AUTH_RESPONSE_LENGTH) 
    {
         ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("MS-CHAPv2 mutual authentication failed.\n")));
        /* Authenticator Response did not match expected. */
        return 0;
    }
    /* Authenticator Response matches. */
    msg += MS_AUTH_RESPONSE_LENGTH; /* Eat it */
    len -= MS_AUTH_RESPONSE_LENGTH;
    if ((len >= 3) && !strncmp((char *)msg, " M=", 3))
    {
        msg += 3; /* Eat the delimiter */
    } 
    else if (len)
    {
        /* Packet has extra text which does not begin " M=" */
        ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("MS-CHAPv2 Success packet is badly formed.\n")));
        return 0;
    }
    return 1;

}

//Failure Handle
void CChapMs2::HandleFailure(unsigned char *pkt, int len)
{
    CChapMs::HandleFailure(pkt, len);
}

void
CChapMs2::ChapMS2(u_char *rchallenge, 
                u_char *PeerChallenge,
                char *user, 
                char *secret, 
                int secret_len, 
                unsigned char *response,
                u_char authResponse[], 
                int authenticator)
{
    /* ARGSUSED */
    u_char *p = &response[MS_CHAP2_PEER_CHALLENGE];
    int i;

    BZERO(response, MS_CHAP2_RESPONSE_LEN);

    /* Generate the Peer-Challenge if requested, or copy it if supplied. */
    if (!PeerChallenge)
    for (i = 0; i < MS_CHAP2_PEER_CHAL_LEN; i++)
        *p++ = (u_char) (drand48() * 0xff);
    else
    BCOPY(PeerChallenge, &response[MS_CHAP2_PEER_CHALLENGE],
        MS_CHAP2_PEER_CHAL_LEN);

    /* Generate the NT-Response */
    ChapMS2_NT(rchallenge, &response[MS_CHAP2_PEER_CHALLENGE], user,
        secret, secret_len, &response[MS_CHAP2_NTRESP]);

    /* Generate the Authenticator Response. */
    GenerateAuthenticatorResponsePlain(secret, secret_len,
                &response[MS_CHAP2_NTRESP],
                &response[MS_CHAP2_PEER_CHALLENGE],
                rchallenge, user, authResponse);
}

//Generate Authenticator Response Plain
void
CChapMs2::GenerateAuthenticatorResponsePlain
    (char *secret, int secret_len,
        u_char NTResponse[24], u_char PeerChallenge[16],
        u_char *rchallenge, char *username,
        u_char authResponse[])
{
    u_char	unicodePassword[MAX_NT_PASSWORD * 2] ={0};
    u_char	PasswordHash[MD4_SIGNATURE_SIZE] ={0};
    u_char	PasswordHashHash[MD4_SIGNATURE_SIZE] ={0};

    /* Hash (x2) the Unicode version of the secret (== password). */
    ascii2unicode(secret, secret_len, unicodePassword);
    NTPasswordHash(unicodePassword, secret_len * 2, PasswordHash);
    NTPasswordHash(PasswordHash, sizeof(PasswordHash),
        PasswordHashHash);

    GenerateAuthenticatorResponse(PasswordHashHash, NTResponse, PeerChallenge,
            rchallenge, username, authResponse);
}

//Generate Authenticator Response
void
CChapMs2::GenerateAuthenticatorResponse(u_char PasswordHashHash[MD4_SIGNATURE_SIZE],
    u_char NTResponse[24], u_char PeerChallenge[16],
    u_char *rchallenge, char *username,
    u_char authResponse[])
{
    /*
     * "Magic" constants used in response generation, from RFC 2759.
     */
    u_char Magic1[39] = /* "Magic server to client signing constant" */
    { 0x4D, 0x61, 0x67, 0x69, 0x63, 0x20, 0x73, 0x65, 0x72, 0x76,
	  0x65, 0x72, 0x20, 0x74, 0x6F, 0x20, 0x63, 0x6C, 0x69, 0x65,
	  0x6E, 0x74, 0x20, 0x73, 0x69, 0x67, 0x6E, 0x69, 0x6E, 0x67,
	  0x20, 0x63, 0x6F, 0x6E, 0x73, 0x74, 0x61, 0x6E, 0x74 };
    u_char Magic2[41] = /* "Pad to make it do more than one iteration" */
	{ 0x50, 0x61, 0x64, 0x20, 0x74, 0x6F, 0x20, 0x6D, 0x61, 0x6B,
	  0x65, 0x20, 0x69, 0x74, 0x20, 0x64, 0x6F, 0x20, 0x6D, 0x6F,
	  0x72, 0x65, 0x20, 0x74, 0x68, 0x61, 0x6E, 0x20, 0x6F, 0x6E,
	  0x65, 0x20, 0x69, 0x74, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6F,
	  0x6E };

    int		i;
    SHA_CTX	sha1Context;
    u_char	Digest[SHA_DIGEST_LENGTH] ={0};
    u_char	Challenge[8];

    SHA_Init(&sha1Context);
    SHA_Update(&sha1Context, PasswordHashHash, MD4_SIGNATURE_SIZE);
    SHA_Update(&sha1Context, NTResponse, 24);
    SHA_Update(&sha1Context, Magic1, sizeof(Magic1));
    SHA_Final(Digest, &sha1Context);

    ChallengeHash(PeerChallenge, rchallenge, username, Challenge);

    SHA_Init(&sha1Context);
    SHA_Update(&sha1Context, Digest, sizeof(Digest));
    SHA_Update(&sha1Context, Challenge, sizeof(Challenge));
    SHA_Update(&sha1Context, Magic2, sizeof(Magic2));
    SHA_Final(Digest, &sha1Context);

    /* Convert to ASCII hex string. */
    for (i = 0; i < MAX((MS_AUTH_RESPONSE_LENGTH / 2), sizeof(Digest)); i++)
        sprintf((char *)&authResponse[i * 2], "%02X", Digest[i]);
}

//Chap MS2_NT
void
CChapMs2::ChapMS2_NT(u_char *rchallenge, u_char PeerChallenge[16], char *username,
    char *secret, int secret_len, u_char NTResponse[24])
{
    u_char  unicodePassword[MAX_NT_PASSWORD * 2] ={0};
    u_char  PasswordHash[MD4_SIGNATURE_SIZE] ={0};
    u_char  Challenge[8];

    ChallengeHash(PeerChallenge, rchallenge, username, Challenge);

    /* Hash the Unicode version of the secret (== password). */
    ascii2unicode(secret, secret_len, unicodePassword);
    NTPasswordHash(unicodePassword, secret_len * 2, PasswordHash);

    ChallengeResponse(Challenge, PasswordHash, NTResponse);
}

//Challenge Hash
void
CChapMs2::ChallengeHash(u_char PeerChallenge[16], u_char *rchallenge,
        char *username, u_char Challenge[8])
    
{
    SHA_CTX        sha1Context;
    u_char          sha1Hash[SHA_DIGEST_LENGTH] ={0};
    char            *user;

    /* remove domain from "domain\username" */
    if ((user = strrchr(username, '\\')) != NULL)
        ++user;
    else
        user = username;

    SHA_Init(&sha1Context);
    SHA_Update(&sha1Context, PeerChallenge, 16);
    SHA_Update(&sha1Context, rchallenge, 16);
    SHA_Update(&sha1Context, (unsigned char *)user, strlen(user));
    SHA_Final(sha1Hash, &sha1Context);

    BCOPY(sha1Hash, Challenge, 8);
}




