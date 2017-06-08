// Interface for authentication manager

#ifndef __IAUTHMANAGER_H__
#define __IAUTHMANAGER_H__

#include "aim_ex.h"
#include "aceinclude.h"

class ACE_Export IAuthManagerSink 
{
public:
    virtual ~IAuthManagerSink(){};
    virtual int OnAuthResponse(const Auth_Response *response) = 0;  
};

class ACE_Export IAuthManager
{
public:
    static IAuthManager &instance();
    virtual ~IAuthManager(){};
    
    virtual int OpenWithSink(IAuthManagerSink *psink) = 0;
    virtual int Close() = 0;
    virtual int AuthRequest(const Auth_Request *request) = 0;
};

#endif//__IAUTHMANAGER_H__
