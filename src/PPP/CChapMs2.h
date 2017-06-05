/*
 * Copyright (c) 2014  Nanjing WFN Technology Co., Ltd.  All rights reserved.
*/

#ifndef CCHAPMS2_H
#define CCHAPMS2_H
#include "CChapMS.h"

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

