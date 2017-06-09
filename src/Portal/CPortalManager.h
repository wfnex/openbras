/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
**********************************************************************/
#ifndef CPORTALMANAGER_H
#define CPORTALMANAGER_H
#include "aceinclude.h"
#include "CPortalClient.h"
#include "CPortalFakeServer.h"
#include "ISessionManager.h"
#include "IAuthManager.h"
#include "CPortalFakeServer.h"
#include "CPortalClient.h"
#include "CPortalServerManager.h"
#include "CPortalUserMgr.h"
#include "CPortalConfig.h"

class CPortalManager : public IAuthManagerSink, public ISessionManagerSink
{
public:
    CPortalManager();
    ~CPortalManager();
    int Init();
public:
    //IAuthManagerSink
    virtual int OnAuthResponse(const Auth_Response *response); 
public:
    //ISessionManagerSink
    virtual int OnAddUserResponse(const UM_RESPONSE &response); 
    virtual int OnDeleteUserResponse(const UM_RESPONSE &response);
    virtual int OnModifyUserResponse(const UM_RESPONSE &response);
    virtual int OnKickUserNotify(const Sm_Kick_User* kickInfo);

    CPortalClient &GetPortalClient()
    {
        return m_client;
    }
    CPortalFakeServer &GetPortalFakeServer()
    {
        return m_fakeserver;
    }
    CPortalServerManager &GetPortalServerMgr()
    {
        return m_servermgr;
    }
    
    CPortalUserMgr &GetPortalUserMgr()
    {
        return m_usrmgr;
    }
    CPortalConfig &GetPortalConfig()
    {
        return m_config;
    }
private:
    CPortalClient m_client;
    CPortalFakeServer m_fakeserver;
    CPortalServerManager m_servermgr;
    CPortalUserMgr m_usrmgr;
    CPortalConfig m_config;
};


#endif//CPORTALMANAGER_H
