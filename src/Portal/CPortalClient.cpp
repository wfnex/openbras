/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
**********************************************************************/

#include "CPortalClient.h"
#include "CPortalServerChannel.h"
#include "CPortalServerManager.h"
#include "CPortalManager.h"
#include "CPortalConfig.h"
#include "CPortalConnector.h"
CPortalClient::CPortalClient(CPortalManager &mgr)
    :m_selectalg(AlwaysFirst)
    ,m_portalmgr(mgr)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalClient::CPortalClient\n"));   
}

CPortalClient::~CPortalClient()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalClient::~CPortalClient\n"));
    // Commented by mazhh: There is no need to call Close() here, as ~CIPOEModule() will call it.
}

int CPortalClient::CreateConnector(PortalServerCfg &cfg)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, -1);

    CCmAutoPtr<CPortalConnector> channel(new CPortalConnector(*this,cfg));
    if (channel.Get()==NULL)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalClient::CreateConnector, out of memory\n")); 
        return -1;
    }
    m_connectors.push_back(channel);
    ACE_INET_Addr PeerAddr;
    PeerAddr.set_port_number(cfg.port,1);
    struct sockaddr_in peeraddr = {0};
    peeraddr.sin_family = AF_INET;
    peeraddr.sin_port = htons(cfg.port);
    peeraddr.sin_addr.s_addr = htonl(cfg.ip_address);
    PeerAddr.set_addr (&peeraddr, sizeof(peeraddr));

    ACE_TCHAR peeraddr_str[80]={0};
    PeerAddr.addr_to_string (peeraddr_str, 80);

    
    if (channel->StartConnect(PeerAddr) == -1)
    {
        ACE_DEBUG ((LM_ERROR,
          ACE_TEXT ("(%P|%t) CPortalClient::CreateConnector failure")
          ACE_TEXT ("PortalServer=%s\n"),
          peeraddr_str));

        
        return -1;
    }
    else
    {
        ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("(%P|%t) CPortalClient::CreateConnector success!!")
                  ACE_TEXT ("PortalServer=%s\n"),
                  peeraddr_str));

        
        return 0;
    }
}

int CPortalClient::DestroyConnector(uint8_t serverid)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, -1);
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalClient::DestroyConnector, serverid=%d\n",serverid)); 

    std::list<CCmAutoPtr<CPortalConnector>>::iterator it = m_connectors.begin();
    while(it != m_connectors.end())
    {
        CCmAutoPtr<CPortalConnector> &channel = *it;
        if (channel.Get())
        {
            if (channel->GetServerId() == serverid)
            {
                it = m_connectors.erase(it);
                break;
            }
        }
        it++;
    }

    return 0;
}

int CPortalClient::DestroyConnector(uint32_t peerip, uint16_t peerport)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, -1);
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalClient::DestroyConnector, peerip=%d, peerport=%d\n",peerip, peerport)); 

    std::list<CCmAutoPtr<CPortalConnector>>::iterator it = m_connectors.begin();
    while(it != m_connectors.end())
    {
        CCmAutoPtr<CPortalConnector> &channel = *it;
        if (channel.Get())
        {
            if ((channel->GetServerIP() == peerip) && (channel->GetServerPort() == peerport))
            {
                it = m_connectors.erase(it);
                break;
            }
        }
        it++;
    }

    return 0;
}


CPortalConnector *CPortalClient::FindActiveServer()
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, NULL);
    std::list<CCmAutoPtr<CPortalConnector>>::iterator it = m_connectors.begin();
    while(it != m_connectors.end())
    {
        CCmAutoPtr<CPortalConnector> &channel = *it;
        if (channel->IsChannelAlive())
        {
            return channel.Get();
        }
        it++;
    }
    return NULL;   
}


