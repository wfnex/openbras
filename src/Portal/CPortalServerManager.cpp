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

#include "CPortalServerManager.h"
#include "CPortalServerChannel.h"
#include "CPortalManager.h"

CPortalServerManager::CPortalServerManager(CPortalManager &mgr)
    :m_portalmgr(mgr)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerManager::CPortalServerManager\n")); 
}

CPortalServerManager::~CPortalServerManager()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerManager::~CPortalServerManager\n")); 
}

//Add Channel
int CPortalServerManager::AddChannel(const ACE_INET_Addr &peeraddr, 
        const ACE_INET_Addr &localaddr,
        CCmAutoPtr<CPortalServerChannel> &channel)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, -1);

    CPairInetAddr addrPair(localaddr, peeraddr);
    ACE_TCHAR remote_str[80]={0};
    ACE_TCHAR local_str[80]={0};
    peeraddr.addr_to_string (remote_str, 80);
    localaddr.addr_to_string(local_str,80);

    ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%P|%t) CPortalServerManager::AddChannel - ")
              ACE_TEXT ("peeraddr=%s,localaddr=%s\n"),
              remote_str,
              local_str));

    UDPTransportsType::iterator it = m_PortalServers.find(addrPair);
    if (it != m_PortalServers.end())
    {
        return -1;
    }

    UDPTransportsType::value_type nodeNew(addrPair, channel);
    m_PortalServers.insert(nodeNew);

    return 0;
}

//Find Channel
CPortalServerChannel *CPortalServerManager::FindChannel(const ACE_INET_Addr &peeraddr, const ACE_INET_Addr &localaddr)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, NULL);
    ACE_TCHAR remote_str[80]={0};
    ACE_TCHAR local_str[80]={0};
    peeraddr.addr_to_string (remote_str, 80);
    localaddr.addr_to_string(local_str,80);

    ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%P|%t) CPortalServerManager::FindChannel - ")
              ACE_TEXT ("peeraddr=%s,localaddr=%s\n"),
              remote_str,
              local_str));

    CPairInetAddr addrPair(localaddr, peeraddr);

    UDPTransportsType::iterator it = m_PortalServers.find(addrPair);
    if (it == m_PortalServers.end())
    {
        return NULL;
    }
    CCmAutoPtr<CPortalServerChannel> &channel = it->second;
    return channel.Get();
}

//Remove Channel
int CPortalServerManager::RemoveChannel(const ACE_INET_Addr &peeraddr, 
        const ACE_INET_Addr &localaddr)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, NULL);
    ACE_TCHAR remote_str[80]={0};
    ACE_TCHAR local_str[80]={0};
    peeraddr.addr_to_string (remote_str, 80);
    localaddr.addr_to_string(local_str,80);

    ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%P|%t) CPortalServerManager::RemoveChannel - ")
              ACE_TEXT ("peeraddr=%s,localaddr=%s\n"),
              remote_str,
              local_str));

    CPairInetAddr addrPair(localaddr, peeraddr);

    UDPTransportsType::iterator it = m_PortalServers.find(addrPair);
    if (it == m_PortalServers.end())
    {
        return -1;
    }
    m_PortalServers.erase(it);
    return 0;
}

int CPortalServerManager::RemoveChannel(CCmAutoPtr<CPortalServerChannel> &channel)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, NULL);
    ACE_INET_Addr peeraddr = channel->GetPeerAddr();
    ACE_INET_Addr localaddr = channel->GetLocalAddr();

    ACE_TCHAR remote_str[80]={0};
    ACE_TCHAR local_str[80]={0};
    peeraddr.addr_to_string (remote_str, 80);
    localaddr.addr_to_string(local_str,80);

    ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%P|%t) CPortalServerManager::RemoveChannel - ")
              ACE_TEXT ("peeraddr=%s,localaddr=%s\n"),
              remote_str,
              local_str));

    CPairInetAddr addrPair(localaddr, peeraddr);
    UDPTransportsType::iterator it = m_PortalServers.find(addrPair);
    if (it == m_PortalServers.end())
    {
        return -1;
    }
    m_PortalServers.erase(it);
    return 0;
}


void CPortalServerManager::Dump()
{
    
}

//Start Listen
int CPortalServerManager::StartListen(const ACE_INET_Addr &localaddr)
{
    m_localaddr = localaddr;
    
    ACE_DEBUG ((LM_DEBUG, "CPortalServerManager::StartListen\n"));
    
    ACE_TCHAR localaddr_str[BUFSIZ]={0};
    m_localaddr.addr_to_string (localaddr_str, sizeof localaddr_str);
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerManager::StartListen, localaddr=%s\n", localaddr_str));
    
    m_handler = ACE_OS::socket (AF_INET, SOCK_DGRAM, 0);
    if (m_handler == ACE_INVALID_HANDLE)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerManager::StartListen, handle error\n"));
        return -1;
    }

    int one = 1;
    if (ACE_OS::setsockopt (m_handler,
                            SOL_SOCKET,
                            SO_REUSEADDR,
                            (const char*) &one,
                            sizeof one) == -1)
      {
          ACE_OS::closesocket(m_handler);
          m_handler = ACE_INVALID_HANDLE;
          ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerManager::StartListen, setsockopt SO_REUSEADDR error\n"));
          return -1;
      }

    if ((ACE_Addr&)m_localaddr == ACE_Addr::sap_any)
    {
        if (ACE::bind_port (m_handler,
            INADDR_ANY,
            AF_INET) == -1)
        {
            ACE_OS::closesocket(m_handler);
            m_handler = ACE_INVALID_HANDLE;
            ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerManager::StartListen, bind_port  error\n"));
            return -1;
        }
    }
    else
    {
        if (ACE_OS::bind (m_handler,
                                 reinterpret_cast<sockaddr *> (m_localaddr.get_addr ()),
                                 m_localaddr.get_size ()) == -1)
        {
            ACE_OS::closesocket(m_handler);
            m_handler = ACE_INVALID_HANDLE;
            ACE_ERROR((LM_ERROR,
                     ACE_TEXT ("(%P|%t) CPortalServerManager::StartListen: %p\n"),
                     ACE_TEXT ("bind error")));

            return -1;
        }
    }

    int size = 262144; // 256 * 1024 = 262144
    
    if (ACE_OS::setsockopt(m_handler, SOL_SOCKET,
                                    SO_RCVBUF,
                                    (char *) &size,
                                    sizeof (size)) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerManager::StartListen, setsockopt SO_RCVBUF error. size = %d\n", size));
        return -1;
    }

    if (ACE_OS::setsockopt(m_handler, SOL_SOCKET,
                                    SO_SNDBUF,
                                    (char *) &size,
                                    sizeof (size)) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerManager::StartListen, setsockopt SO_SNDBUF error. size = %d\n", size));
        return -1;
    }

    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerManager::StartListen, register_handler\n"));

    // Register with the reactor for input.
    if (ACE_Reactor::instance()->register_handler (this,ACE_Event_Handler::READ_MASK) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_ERROR_RETURN ((LM_ERROR,
                 ACE_TEXT ("(%P|%t) CPortalServerManager::StartListen: %p\n"),
                 ACE_TEXT ("register_handler for input")),
                -1);
    }

    return 0;
}

//Stop Listen
int CPortalServerManager::StopListen()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerManager::StopListen\n"));

    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CPortalServerManager::StopListen \n")));

    if (ACE_Reactor::instance())
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CPortalServerManager::remove_handler \n")));
        ACE_Reactor::instance()->remove_handler (this,
                                                ACE_Event_Handler::ALL_EVENTS_MASK |
                                                ACE_Event_Handler::DONT_CALL);
    }
    
    if (m_handler != ACE_INVALID_HANDLE)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CPortalServerManager::closesocket \n"))); 
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
    }

    return 0;
}

//Get Local Addr
int CPortalServerManager::GetLocalAddr (ACE_INET_Addr &address) const
{
    address = m_localaddr;

    return 0;
}

//Input Handle
int CPortalServerManager::handle_input (ACE_HANDLE fd)
{
    ACE_INET_Addr addrRecv;
    ACE_INET_Addr localAddr;
    static char szBuf[1024*16];
    GetLocalAddr(localAddr);
    int addr_len = addrRecv.get_size ();
    ssize_t rval_recv = ACE_OS::recvfrom(fd, szBuf, sizeof(szBuf),0, (sockaddr *)addrRecv.get_addr(),(int*)&addr_len);
    addrRecv.set_size (addr_len);
    ACE_TCHAR remote_str[80]={0};
    ACE_TCHAR local_str[80]={0};
    addrRecv.addr_to_string (remote_str, 80);
    localAddr.addr_to_string(local_str,80);

    ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%P|%t) CPortalServerManager::handle_input - ")
              ACE_TEXT ("activity occurred on handle %d!,%s,local=%s\n"),
              m_handler,
              remote_str,
              local_str));
    
    ACE_DEBUG ((LM_INFO,
            ACE_TEXT ("(%P|%t) CPortalServerManager::handle_input - ")
            ACE_TEXT ("message from %d bytes received.\n"),
            rval_recv));

    if ((rval_recv == -1) || (rval_recv == 0))
    {
          ACE_ERROR_RETURN ((LM_ERROR,
                             ACE_TEXT ("(%P|%t) CPortalServerManager::handle_input - ")
                             ACE_TEXT ("closing daemon (fd = %d)\n"),
                             this->get_handle ()),
                            0); 
    }

    uint32_t serverip=addrRecv.get_ip_address();
    PortalServerCfg *pcfg = m_portalmgr.GetPortalConfig().GetServerCfg(serverip);
    if (pcfg == NULL)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CPortalServerManager::handle_input can not find the server cfg drop it \n"))); 
        return 0;
    }
    //addrRecv.set_port_number(pcfg->port);


    CPortalServerChannel *pchannel = FindChannel(addrRecv,localAddr);
    if (pchannel)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CPortalServerManager::handle_input find the channel\n")));
        pchannel->RcvData(szBuf,rval_recv);
    }
    else
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CPortalServerManager::handle_input can not find ,create one\n")));
        CCmAutoPtr<CPortalServerChannel> channel(new CPortalServerChannel(*this,localAddr, addrRecv,pcfg));
        if (channel.Get())
        {
            channel->set_handle(fd);
            AddChannel(addrRecv,localAddr,channel);
            channel->RcvData(szBuf,rval_recv);
        }
    }
    return 0;
}

//Get Handle
ACE_HANDLE CPortalServerManager::get_handle (void) const
{
    return m_handler;
}

//Close Handle
int CPortalServerManager::handle_close (ACE_HANDLE handle,
                        ACE_Reactor_Mask close_mask)
{
    return 0;
}

//Get Portal Manager
CPortalManager &CPortalServerManager::GetPortalManager()
{
    return m_portalmgr;
}



