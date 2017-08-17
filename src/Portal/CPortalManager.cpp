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

#include "CPortalManager.h"
#include "CPortalUserMgr.h"
#include "CPortalServerManager.h"
#include "CPortalServerChannel.h"

CPortalManager::CPortalManager()
    :m_client(*this)
    ,m_fakeserver(*this)
    ,m_servermgr(*this)
    ,m_usrmgr(*this)
    ,m_config(*this)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalManager::CPortalManager\n" )); 
}

CPortalManager::~CPortalManager()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalManager::~CPortalManager\n" )); 
    IAuthManager::instance().Close();
    ISessionManager::instance().Close();

    m_servermgr.StopListen();
    m_fakeserver.StopListen();
}

//Init config Portal ListenIp and fake http server ListenIp
int CPortalManager::Init()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalManager::Init\n" )); 

    if (m_config.Init() == -1)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalManager::Init, config init error failure\n" ));
        return -1;
    }

    ACE_INET_Addr listenip = m_config.GetPortalListenAddr();
    ACE_INET_Addr httplistenip=m_config.GetHttpListenAdd();

    IAuthManager::instance().OpenWithSink(this);
    ISessionManager::instance().openWithSink(this);
    
    if (m_servermgr.StartListen(listenip) == -1)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalManager::Init, Portal Client StartListen error\n" ));
        return -1;
    }

    if (m_fakeserver.StartListen(httplistenip) == -1)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalManager::Init, Fake Server StartListen error\n" ));
        return -1;
    }
    ACE_TCHAR listenip_str[80]={0};
    ACE_TCHAR httplistenip_str[80]={0};
    listenip.addr_to_string (listenip_str, 80);
    httplistenip.addr_to_string(httplistenip_str,80);

    ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%P|%t) CPortalManager::Init OK!!!!!!!!!!!!! ")
              ACE_TEXT ("portal listenip=%s,fake http server listenip=%s\n"),
              listenip_str,
              httplistenip_str));

    return 0;
}

//Auth Response
int CPortalManager::OnAuthResponse(const Auth_Response *response)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalManager::OnAuthResponse, user_ip=%d\n",response->user_ip)); 

    uint64_t userid = 0;
    CPortalUser *puser = NULL;
    m_usrmgr.FindUser(response->user_ip,0,puser);
    if (puser)
    {
        ACE_INET_Addr portalpeeraddr = puser->GetPortalServerIP();
        ACE_INET_Addr portallocaladdr = puser->GetPortalLocalIP();
        CPortalServerChannel *pchannel = m_servermgr.FindChannel(portalpeeraddr,portallocaladdr);
        if (pchannel)
        {
            pchannel->OnAuthResponse(response);
        }
    }
    else
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalManager::OnAuthResponse can not find user\n")); 
    }
    return 0;
}

//Add User Response
int CPortalManager::OnAddUserResponse(const UM_RESPONSE &response)
{
    return 0;
}

//Delete User Response
int CPortalManager::OnDeleteUserResponse(const UM_RESPONSE &response)
{
    return 0;
}

//Modify User Response
int CPortalManager::OnModifyUserResponse(const UM_RESPONSE &response)
{
    return 0;
}

//Kick User Notify
int CPortalManager::OnKickUserNotify(const Sm_Kick_User* kickInfo)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalManager::OnKickUserNotify\n" )); 

    uint32_t userid=kickInfo->userip;
    uint32_t vrf=kickInfo->vrf;
    CPortalUser *puser = NULL;
    m_usrmgr.FindUser(userid,vrf,puser);
    if (puser)
    {
        ACE_INET_Addr serverAdd = puser->GetPortalServerIP();
        ACE_INET_Addr UASIPAdd = puser->GetPortalLocalIP();
        CPortalServerChannel *pchannel = m_servermgr.FindChannel(serverAdd,UASIPAdd);
        if (pchannel)
        {
            pchannel->NotifyLogoutRequest(userid);
        }
    }

    m_usrmgr.RemoveUser(userid,vrf);

    return 0;
}



