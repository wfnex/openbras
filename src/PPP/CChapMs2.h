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


#ifndef CCHAPMS2_H
#define CCHAPMS2_H
#include "CChapMS.h"
#include "openssl/sha.h"

class VPN_PUBLIC CChapMs2 : public CChapMs 
{
public:
    CChapMs2(CChap *pnext = NULL);
    virtual ~CChapMs2();
    virtual void GenerateChallenge(unsigned char *challenge);
    virtual int VerifyResponse(int id, char *name,
        unsigned char *secret, int secret_len,
        unsigned char *challenge, unsigned char *response,
        char *message, int message_space);
    virtual void MakeResponse(unsigned char *response, int id, char *our_name,
        unsigned char *challenge, char *secret, int secret_len,
        unsigned char *priv);
    virtual int CheckSuccess(int id, unsigned char *pkt, int len);
    virtual void HandleFailure(unsigned char *pkt, int len);
protected:
    void ChallengeHash(u_char PeerChallenge[16], u_char *rchallenge,
        char *username, u_char Challenge[8]);
    void ChapMS2_NT(u_char *rchallenge, u_char PeerChallenge[16], char *username,
    char *secret, int secret_len, u_char NTResponse[24]);
    void GenerateAuthenticatorResponse(u_char PasswordHashHash[MD4_SIGNATURE_SIZE],
        u_char NTResponse[24], u_char PeerChallenge[16],
        u_char *rchallenge, char *username,
        u_char authResponse[]);
    void GenerateAuthenticatorResponsePlain
    (char *secret, int secret_len,
        u_char NTResponse[24], u_char PeerChallenge[16],
        u_char *rchallenge, char *username,
        u_char authResponse[]);
    void ChapMS2(u_char *rchallenge, u_char *PeerChallenge,
    char *user, char *secret, int secret_len, unsigned char *response,
    u_char authResponse[], int authenticator);
private:
    
};


#endif//CCHAPMS_H

