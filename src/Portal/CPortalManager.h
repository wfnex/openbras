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
