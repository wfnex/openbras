/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
 ***********************************************************************/

#ifndef CIPOEMODULE_H
#define CIPOEMODULE_H

#include "aceinclude.h"
#include "CDHCPServer.h"
#include "ISessionManager.h"
#include "IAuthManager.h"

class IEventReactor;

class CIPOEModule : public IAuthManagerSink, public ISessionManagerSink
{
public:
    CIPOEModule(ACE_Reactor *preactor);
    virtual ~CIPOEModule();
    
    int Start(const std::string &serverip);
    int AuthRequest(const Auth_Request &request);
    int AddUserRequest(const Session_User_Ex &user);
    int DeleteUserRequest(const Session_Offline &user);
    
    //For IAuthManagerSink interface
    virtual int OnAuthResponse(const Auth_Response *response);
    
    //For ISessionManagerSink interface
    virtual int OnAddUserResponse(const UM_RESPONSE &response); 
    virtual int OnDeleteUserResponse(const UM_RESPONSE &response);
    virtual int OnModifyUserResponse(const UM_RESPONSE &response);
    virtual int OnKickUserNotify(const Sm_Kick_User* kickInfo);
    
private:
    CDHCPServer m_dhcpserver;
    ISessionManager &m_sessionmgr;
    IAuthManager &m_AuthMgr;
    ACE_Reactor * m_reactor;
    IEventReactor &m_eventReactor;
};

#endif//CIPOEMODULE_H

