/*
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
 */

#ifndef CAUTHEN_H
#define CAUTHEN_H

#include "IAuthManager.h"

class IAuthenSvrSink
{
public:
    virtual ~IAuthenSvrSink(){};
    
    virtual void SendAuthRequest2AM(Auth_Request &authReq) = 0;
    virtual void OnAuthenOutput(unsigned char *packet, size_t size) = 0;
    virtual void OnAuthenResult(int result, int protocol) = 0; // result为1表示认证成功，为0表示认证失败。
};

#endif // CAUTHEN_H

