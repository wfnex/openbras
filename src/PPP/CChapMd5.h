/*
 * Copyright (c) 2014  Nanjing WFN Technology Co., Ltd.  All rights reserved.
*/

#ifndef CCHAPMD5_H
#define CCHAPMD5_H
#include "CChap.h"

class VPN_PUBLIC CChapMd5 : public CChap 
{
public:
    CChapMd5(CChap *pnext = NULL);
    virtual ~CChapMd5();
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

private:
    
};


#endif//CCHAPMD5_H

