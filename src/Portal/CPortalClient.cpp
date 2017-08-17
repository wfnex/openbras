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

//Create Connector
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

//remove Connector
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

//Find Active Server
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


