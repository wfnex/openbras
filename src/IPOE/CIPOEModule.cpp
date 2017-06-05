/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
**********************************************************************/

#include "CIPOEModule.h"
#include "CDHCPAuthRespEventHndl.h"
#include "CDHCPKickUserEventHndl.h"

CIPOEModule::CIPOEModule(ACE_Reactor *preactor)
    :m_dhcpserver(*this)
    ,m_sessionmgr(ISessionManager::instance())
    ,m_AuthMgr(IAuthManager::instance())
    ,m_reactor(preactor)
    ,m_eventReactor(IEventReactor::Instance())
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CIPOEModule::CIPOEModule\n")); 
}

CIPOEModule::~CIPOEModule()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CIPOEModule::~CIPOEModule\n")); 

    m_dhcpserver.StopListen();
    m_AuthMgr.Close();
    m_sessionmgr.Close();
    m_eventReactor.Close();
}

int CIPOEModule::Start(const std::string &serverip)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CIPOEModule::Start\n")); 

    if (m_dhcpserver.StartListen(serverip, m_reactor) == -1)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CIPOEModule::Start dhcp server failure\n"));
        return -1;
    }

    if (m_AuthMgr.OpenWithSink(this) == -1)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CIPOEModule::Start Authmgr open failure\n"));
        return -1;
    }

    if (m_sessionmgr.openWithSink(this) == -1)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CIPOEModule::Start SessionMgr openfailure\n"));
        return -1;
    }
    
    return 0;
}

int CIPOEModule::AuthRequest(const Auth_Request &request)
{
    ACE_DEBUG ((LM_DEBUG, "CIPOEModule::AuthRequest, m_AuthMgr.AuthRequest\n"));
    return m_AuthMgr.AuthRequest(&request);
}

int CIPOEModule::AddUserRequest(const Session_User_Ex &user)
{
    ACE_DEBUG ((LM_DEBUG, "CIPOEModule::AddUserRequest, m_sessionmgr.addUserRequest\n"));
    return m_sessionmgr.addUserRequest(&user);
}

int CIPOEModule::DeleteUserRequest(const Session_Offline &user)
{
    ACE_DEBUG ((LM_DEBUG, "CIPOEModule::DeleteUserRequest, m_sessionmgr.deleteUserRequest\n"));
    return m_sessionmgr.deleteUserRequest(&user);
}

int CIPOEModule::OnAuthResponse(const Auth_Response *response)
{
    ACE_DEBUG ((LM_DEBUG, "(%P|%t) CIPOEModule::OnAuthResponse\n"));
    
    CDHCPAuthRespEventHndl *pevent = new CDHCPAuthRespEventHndl(m_dhcpserver, response);
    if (NULL == pevent)
    {
        ACE_DEBUG ((LM_ERROR, "(%P|%t) Failed to new CDHCPAuthRespEventHndl.\n"));
        return -1;
    }

    int retVal = m_eventReactor.ScheduleEvent(pevent);
    if (retVal != 0)
    {
        ACE_DEBUG ((LM_ERROR, "(%P|%t) Failed to schedule event. retVal = %d\n", retVal));
        return -1;
    }
    
    return 0;
}

//ISessionManagerSink
int CIPOEModule::OnAddUserResponse(const UM_RESPONSE &response)
{
    return 0;
}

int CIPOEModule::OnDeleteUserResponse(const UM_RESPONSE &response)
{
    return 0;
}

int CIPOEModule::OnModifyUserResponse(const UM_RESPONSE &response)
{
    return 0;
}

int CIPOEModule::OnKickUserNotify(const Sm_Kick_User* kickInfo)
{
    ACE_DEBUG ((LM_INFO, "CIPOEModule::onKickUserNotify\n"));

    if (NULL == kickInfo)
    {
        ACE_DEBUG ((LM_ERROR, "NULL arg\n"));
        return -1;
    }

    CDHCPKickUserEventHndl *pevent = new CDHCPKickUserEventHndl(m_dhcpserver, kickInfo);
    if (NULL == pevent)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to new CKickUserHandleEvent.\n"));
        return -1;
    }

    int retVal = m_eventReactor.ScheduleEvent(pevent);
    if (retVal != 0)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to schedule event. retVal = %d\n", retVal));
        return -1;
    }
    
    return 0;
}


