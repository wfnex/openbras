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


#ifndef CIPOEMODULE_H
#define CIPOEMODULE_H

#include "aceinclude.h"
#include "CDHCPServer.h"
#include "ISessionManager.h"
#include "IAuthManager.h"

class IEventReactor;

class ACE_Export CIPOEModule : public IAuthManagerSink, public ISessionManagerSink
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

