/*
 * Copyright (c) 2014  Nanjing WFN Technology Co., Ltd.  All rights reserved.
*/

#ifndef CCHAP_H
#define CCHAP_H
#include "aceinclude.h"

#include "pppdef.h"
#include "sha1.h"
#include "random.h"
//#include "pppcrypt.h"


class VPN_PUBLIC CChap 
{
public:
    CChap(int code, CChap *pnext = NULL);
    virtual ~CChap();
    /*
     * Note: challenge and response arguments below are formatted as
     * a length byte followed by the actual challenge/response data.
     */
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
    int m_code;
    CChap *m_pNextChap;
};


#endif//CCHAP_H

