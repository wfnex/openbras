/*
 * Copyright (c) 2014  Nanjing WFN Technology Co., Ltd.  All rights reserved.
*/

#ifndef CCHAPMS_H
#define CCHAPMS_H
#include "CChap.h"
#include "pppcrypt.h"
#include "md5.h"
#include "md4.h"

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
