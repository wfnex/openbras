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


#ifndef CCHAPMS_H
#define CCHAPMS_H
#include "CChap.h"
#include "pppcrypt.h"
#include "openssl/md5.h"
#include "openssl/md4.h"

int MD4_Init(MD4_CTX *c);
int MD4_Update(MD4_CTX *c, const void *data_, size_t len);
int MD4_Final(unsigned char *md, MD4_CTX *c);


class VPN_PUBLIC CChapMs : public CChap 
{
public:
    CChapMs(int code=CHAP_MICROSOFT, CChap *pnext = NULL);
    virtual ~CChapMs();
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
    void ChapMS(u_char *rchallenge, char *secret, int secret_len,
       unsigned char *response);
    void ChapMS_NT(u_char *rchallenge, char *secret, int secret_len,
    u_char NTResponse[24]);
    void ChallengeResponse(u_char *challenge,
      u_char PasswordHash[MD4_SIGNATURE_SIZE],
      u_char response[24]);
    void NTPasswordHash(u_char *secret, int secret_len, u_char hash[MD4_SIGNATURE_SIZE]);
    void ascii2unicode(char ascii[], int ascii_len, u_char unicode[]);
private:
    
};


#endif//CCHAPMS_H
