/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
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

int CPortalManager::OnAddUserResponse(const UM_RESPONSE &response)
{
    return 0;
}

int CPortalManager::OnDeleteUserResponse(const UM_RESPONSE &response)
{
    return 0;
}

int CPortalManager::OnModifyUserResponse(const UM_RESPONSE &response)
{
    return 0;
}

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



