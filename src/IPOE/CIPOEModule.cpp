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

//Start dhcpserver
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

//Auth Request
int CIPOEModule::AuthRequest(const Auth_Request &request)
{
    ACE_DEBUG ((LM_DEBUG, "CIPOEModule::AuthRequest, m_AuthMgr.AuthRequest\n"));
    return m_AuthMgr.AuthRequest(&request);
}

// Add User Request
int CIPOEModule::AddUserRequest(const Session_User_Ex &user)
{
    ACE_DEBUG ((LM_DEBUG, "CIPOEModule::AddUserRequest, m_sessionmgr.addUserRequest\n"));
    return m_sessionmgr.addUserRequest(&user);
}

//Delete User Request
int CIPOEModule::DeleteUserRequest(const Session_Offline &user)
{
    ACE_DEBUG ((LM_DEBUG, "CIPOEModule::DeleteUserRequest, m_sessionmgr.deleteUserRequest\n"));
    return m_sessionmgr.deleteUserRequest(&user);
}

//Auth Response
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
//Add User Response
int CIPOEModule::OnAddUserResponse(const UM_RESPONSE &response)
{
    return 0;
}

//Delete User Response
int CIPOEModule::OnDeleteUserResponse(const UM_RESPONSE &response)
{
    return 0;
}

//Modify User Response
int CIPOEModule::OnModifyUserResponse(const UM_RESPONSE &response)
{
    return 0;
}

//Kick User Notify
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


