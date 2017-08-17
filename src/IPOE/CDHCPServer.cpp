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

#include "CDHCPServer.h"
#include "CDHCPSession.h"
#include "CIPOEModule.h"

CDHCPServer::CDHCPServer(CIPOEModule &ipoe)
    :m_ipoe(ipoe)
    ,m_handler(ACE_INVALID_HANDLE)
    ,m_reactor(ACE_Reactor::instance())
    ,m_port(67)
    ,m_broadcasthandler(*this)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPServer::CDHCPServer\n"));   
}

CDHCPServer::~CDHCPServer()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPServer::~CDHCPServer\n"));
    // Commented by mazhh: There is no need to call Close() here, as ~CIPOEModule() will call it.
}

//Start Listen
int CDHCPServer::StartListen(const std::string &serverip, ACE_Reactor *preactor)
{
    m_ip = serverip;
    m_localaddr = ACE_INET_Addr(BOOTP_REQUEST_PORT, serverip.c_str());
    m_reactor = preactor;

    #if 0
    if (m_broadcast.open(m_localaddr,AF_INET,0,1,NULL) == -1)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CDHCPServer::StartListen open broadcast socket failure\n"));
        return -1;
    }
    #endif
    
    if (this->Open(m_reactor) == -1)
    {
        ACE_DEBUG ((LM_ERROR, "(%P|%t) CDHCPServer::StartListen open failure\n"));
        return -1;
    }

    ACE_DEBUG ((LM_DEBUG, "(%P|%t) CDHCPServer::StartListen,%s\n", serverip.c_str()));

    return 0;
}

//Stop Listen
int CDHCPServer::StopListen()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPServer::StopListen\n"));

    this->Close();
    ClearAllSession();

    return 0;
}

//Open the reactor and establish a socket link locally
int CDHCPServer::Open(ACE_Reactor *reactor)
{
    ACE_DEBUG ((LM_DEBUG, "(%P|%t) CDHCPServer::Open,  reactor=%#x\n", reactor));

    ACE_DEBUG ((LM_DEBUG, "m_broadcasthandler.Open\n"));
    if (m_broadcasthandler.Open(reactor) == -1)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CDHCPServer::Open, broadcast handler open failure\n"));

        return -1;
    }
    
    ACE_TCHAR localaddr_str[BUFSIZ]={0};
    m_localaddr.addr_to_string (localaddr_str, sizeof localaddr_str);
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPServer::Open, localaddr=%s\n", localaddr_str));
    
    m_handler = ACE_OS::socket (AF_INET, SOCK_DGRAM, 0);
    if (m_handler == ACE_INVALID_HANDLE)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CDHCPServer::Open, handle error\n"));
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
          ACE_DEBUG ((LM_ERROR,"(%P|%t) CDHCPServer::Open, setsockopt SO_REUSEADDR error\n"));
          return -1;
      }

    int broadcastone = 1;
    if (ACE_OS::setsockopt (m_handler,
                            SOL_SOCKET,
                            SO_BROADCAST,
                            (char *)&broadcastone,
                            sizeof broadcastone) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CDHCPServer::Open, setsockopt SO_BROADCAST error\n"));
        return -1;
    }

#if 0
    if ((ACE_Addr&)m_localaddr == ACE_Addr::sap_any)
    {
        if (ACE::bind_port (m_handler,
            INADDR_ANY,
            AF_INET) == -1)
        {
            ACE_OS::closesocket(m_handler);
            m_handler = ACE_INVALID_HANDLE;
            ACE_DEBUG ((LM_ERROR,"(%P|%t) CDHCPServer::Open, bind_port  error\n"));
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
                     ACE_TEXT ("(%P|%t) CDHCPServer::open: %p\n"),
                     ACE_TEXT ("bind error")));

            return -1;
        }
    }
#endif

    //ACE_INET_Addr bindip(BOOTP_REQUEST_PORT, "0.0.0.0");
    ACE_INET_Addr bindip = m_localaddr;
    if (ACE_OS::bind (m_handler,
                             reinterpret_cast<sockaddr *> (bindip.get_addr ()),
                             bindip.get_size ()) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_ERROR((LM_ERROR,
                 ACE_TEXT ("(%P|%t) CDHCPServer::open: %p\n"),
                 ACE_TEXT ("bind error")));
    
        return -1;
    }

    int size = 262144; // 256 * 1024 = 262144
    
    if (ACE_OS::setsockopt(m_handler, SOL_SOCKET,
                                    SO_RCVBUF,
                                    (char *) &size,
                                    sizeof (size)) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CDHCPServer::Open, setsockopt SO_RCVBUF error. size = %d\n", size));
        return -1;
    }

    if (ACE_OS::setsockopt(m_handler, SOL_SOCKET,
                                    SO_SNDBUF,
                                    (char *) &size,
                                    sizeof (size)) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CDHCPServer::Open, setsockopt SO_SNDBUF error. size = %d\n", size));
        return -1;
    }

    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPServer::Open, register_handler %x\n",m_reactor));

    // Register with the reactor for input.
    if (m_reactor->register_handler (this,ACE_Event_Handler::READ_MASK) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_ERROR_RETURN ((LM_ERROR,
                 ACE_TEXT ("(%P|%t) CDHCPServer::open: %p\n"),
                 ACE_TEXT ("register_handler for input")),
                -1);
    }

    return 0;
}

//The reactor remove the handle and closes the socket link
void CDHCPServer::Close()
{
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CDHCPServer::close \n")));

    if (m_reactor)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CDHCPServer::remove_handler \n")));
        m_reactor->remove_handler (this,
                                                ACE_Event_Handler::ALL_EVENTS_MASK |
                                                ACE_Event_Handler::DONT_CALL);
    }
    
    if (m_handler != ACE_INVALID_HANDLE)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CDHCPServer::closesocket \n"))); 
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
    }

}

//Get Local Addr
int CDHCPServer::GetLocalAddr (ACE_INET_Addr &address) const
{
    address = m_localaddr;

    return 0;
}

//Get Local Addr Byte
uint32_t CDHCPServer::GetLocalAddrByte() 
{ 
    //返回网络顺序
    uint32_t ipaddr = m_localaddr.get_ip_address();
    return htonl(ipaddr);
}


//Receive Packet and return status
ssize_t
CDHCPServer::RcvPacket(ACE_HANDLE fd,void * buf,
                       size_t n,
                       ACE_Addr & addr,
                       int flags) const
{
    int addr_len = addr.get_size ();
    ssize_t status = ACE_OS::recvfrom (fd,
                                     (char *) buf,
                                     n,
                                     flags,
                                     (sockaddr *) addr.get_addr (),
                                     (int*) &addr_len);
    addr.set_size (addr_len);

    return status;
}

//handle for input
int CDHCPServer::handle_input (ACE_HANDLE fd)
{
    ACE_INET_Addr addrRecv;
    ACE_INET_Addr localAddr;
    static char szBuf[1024*16];

    GetLocalAddr(localAddr);

    ssize_t rval_recv = RcvPacket(fd, szBuf, sizeof(szBuf), addrRecv);
    ACE_TCHAR remote_str[80]={0};
    ACE_TCHAR local_str[80]={0};
    addrRecv.addr_to_string (remote_str, 80);
    localAddr.addr_to_string(local_str,80);

    ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%P|%t) CDHCPServer::handle_input - ")
              ACE_TEXT ("activity occurred on handle %d!,%s,local=%s\n"),
              m_handler,
              remote_str,
              local_str));
    
    ACE_DEBUG ((LM_INFO,
            ACE_TEXT ("(%P|%t) CDHCPServer::handle_input - ")
            ACE_TEXT ("message from %d bytes received.\n"),
            rval_recv));

    if ((rval_recv == -1) || (rval_recv == 0))
    {
          ACE_ERROR_RETURN ((LM_ERROR,
                             ACE_TEXT ("(%P|%t) CDHCPServer::handle_input - ")
                             ACE_TEXT ("closing daemon (fd = %d)\n"),
                             this->get_handle ()),
                            0); 
    }

    HandleDHCP(szBuf, rval_recv);

    return 0;
}

//Get handle
ACE_HANDLE CDHCPServer::get_handle (void) const
{
    return m_handler;
}

//Close handle
int CDHCPServer::handle_close (ACE_HANDLE handle,
                        ACE_Reactor_Mask close_mask)
{
    return 0;
}

//Get Server IP
std::string CDHCPServer::GetServerIP()
{
    return m_ip;
}

//Get Server Port 
uint16_t CDHCPServer::GetServerPort()
{
    return BOOTP_REQUEST_PORT;
}

//Discover handle,Bulid session and set discovery authrequest,set network config
void CDHCPServer::HandleDiscover(struct dhcp_packet *request)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPServer::HandleDiscover\n")); 
    
    DHCPNetworkConfig config;

    uint64_t sessionid = GetSessionID(request);
    CDHCPSession *dhcpsession = FindSession(sessionid);
    if (!dhcpsession)
    {
        CDHCPSession *psession = new CDHCPSession(sessionid, *this);
        if (psession == NULL)
        {
            ACE_DEBUG ((LM_ERROR, "CDHCPServer::HandleDiscover, failed to new CDHCPSession. sessionid=%0#x\n", sessionid));
            free_packet(request);
            return;
        }
        
        if (AddSession(sessionid, psession) == -1)
        {
            ACE_DEBUG ((LM_ERROR, "CDHCPServer::HandleDiscover, failed to AddSession. sessionid=%0#x\n", sessionid));
            free_packet(request);
            delete psession;
            return;
        }
        
        psession->SetDiscoverRequest(request);
        
        Auth_Request authrequest;
        ::memset(&authrequest, 0, sizeof(authrequest));
        authrequest.user_type = USER_TYPE_IPOE;
        ::memcpy(authrequest.mac, request->chaddr, 6);

        ACE_DEBUG ((LM_DEBUG, "CDHCPServer::HandleDiscover, m_ipoe.AuthRequest.\n"));
        m_ipoe.AuthRequest(authrequest);
        
        return;
    }
    else
    {
        dhcpsession->SetDiscoverRequest(request);
        dhcpsession->GetNetworkConfig(config);
    }

    DHCPNetworkConfig zeroConfig;
    // If the second or later DHCP discovery is received earlier than authen response, we'll drop the packet.
    if (::memcmp(&config, &zeroConfig, sizeof(config)) == 0)
    {
        ACE_DEBUG ((LM_INFO, 
                             "CDHCPServer::HandleDiscover, AM haven't response yet, so we have to drop the pkt. sessionid=%0#x\n", 
                             sessionid));
        free_packet(request);
        return;
    }
    
    dhcpsession->HandleDiscover(config);
    
}

// Request handle
void CDHCPServer::HandleRequest(struct dhcp_packet *request)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPServer::HandleRequest\n")); 

    CDHCPSession *dhcpsession = FindSession(GetSessionID(request));
    if (!dhcpsession)
    {
        //如果Session不存在，NAK客户端，然后客户端会重新发现
        ACE_DEBUG ((LM_INFO, "No such session(%#x), nak it to restart DHCP negotiation.\n", GetSessionID(request)));
        NakRequest(request);
        return;
    }

    dhcpsession->HandleRequest(request);
}

//Nak Request
int CDHCPServer::NakRequest(struct dhcp_packet *request)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPServer::NakRequest\n"));

    struct dhcp_packet *response = (struct dhcp_packet*)malloc(sizeof(struct dhcp_packet));
    if (NULL == response)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc response.\n"));
        free_packet(request);
        return -1;        
    }
    
    memset(response, 0, sizeof(struct dhcp_packet));

    response->op = BOOT_REPLY;
    response->htype = request->htype;
    response->hlen = request->hlen;
    response->hops = 1;
    memcpy(response->xid, request->xid, 4);
    memcpy(response->yiaddr, request->yiaddr, 4); // Commented by mazhh: unnecessary as yiaddr is assigned by server?
    memcpy(response->flags, request->flags, 2);
    memcpy(response->chaddr, request->chaddr, 16); 

    //options
    //message type
    struct dhcp_option *packet_type = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == packet_type)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc packet_type.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    memset(packet_type, 0, sizeof(struct dhcp_option));
    packet_type->code = DHO_DHCP_MESSAGE_TYPE;
    packet_type->value = (char *)malloc(1);
    if (NULL == packet_type->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc packet_type->value.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    *packet_type->value = DHCP_NAK;
    packet_type->length = 1;
    response->options = packet_type;

    //server identifier
    struct dhcp_option *server_identifier = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == server_identifier)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc server_identifier.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    memset(server_identifier, 0, sizeof(struct dhcp_option));
    server_identifier->code = DHO_DHCP_SERVER_IDENTIFIER;
    server_identifier->value = (char *)malloc(4);
    if (NULL == server_identifier->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc server_identifier->value.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    uint32_t ipaddr = GetLocalAddrByte();
    ::memcpy(server_identifier->value, &ipaddr, 4);

    server_identifier->length = 4;
    packet_type->next = server_identifier;
    
    bool isbroadcast = is_broadcast(request);
    uint32_t requestip = get_request_ip(request);
    if (isbroadcast)
    {
        SendBroadcastReply(response);
    }
    else
    {
        SendDHCP(response, htonl(requestip));
    }

    free_packet(response);
    free_packet(request);
    
    return 0;
}

//Release Handle
void CDHCPServer::HandleRelease(struct dhcp_packet *request)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPServer::HandleRelease\n")); 

    CDHCPSession *dhcpsession = FindSession(GetSessionID(request));
    if (!dhcpsession)
    {
        ACE_DEBUG ((LM_INFO, "No such session(%#x), drop the packet.\n", GetSessionID(request)));
        free_packet(request);
        return;
    }

    dhcpsession->HandleRelease(request);
}

//Handle Inform
void CDHCPServer::HandleInform(struct dhcp_packet *request)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPServer::HandleInform\n")); 

    CDHCPSession *dhcpsession = FindSession(GetSessionID(request));
    if (!dhcpsession)
    {
        ACE_DEBUG ((LM_DEBUG,
                             "(%P|%t) CDHCPServer::HandleInform can not find dhcp session(%#x), drop it.\n",
                             GetSessionID(request)));
        free_packet(request);
        return;
    }

    dhcpsession->HandleInform(request);
}

//Decline Handle 
void CDHCPServer::HandleDecline(struct dhcp_packet *request)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPServer::HandleDecline\n")); 

    CDHCPSession *dhcpsession = FindSession(GetSessionID(request));
    if (!dhcpsession)
    {
        ACE_DEBUG ((LM_DEBUG,
                             "(%P|%t) CDHCPServer::HandleDecline can not find dhcp session(%#x), drop it.\n",
                             GetSessionID(request)));

        free_packet(request);
        return;
    }

    dhcpsession->HandleDecline(request);
}

// Curently, after we process DHCP discovery or request, we start a (release time + 10)-second timer.
// While receiving another DHCP discovery or request from the same client in [release time + 1, release time + 10)
// time range, we see them as not exceeding release time, and we don't nak them and notify session manager to
// delete the user.
void CDHCPServer::HandleDHCP(const char *msg, size_t msgsize)
{
    struct dhcp_packet *request = marshall(msg, 0, msgsize);
    if (NULL == request)
    {
        ACE_DEBUG ((LM_ERROR, "CDHCPServer::HandleDHCP, NULL request.\n"));
        return;
    }

    uint32_t serverid = get_server_id(request);
    if ((serverid != 0) && (ntohl(serverid) != m_localaddr.get_ip_address()))
    {
        ACE_DEBUG ((LM_INFO, 
                            "CDHCPServer::HandleDHCP, server identifier not directed to us. Drop the pkt. "
                            "server identifier in the pkt in = %0#x, local address = %0#x\n",
                            ntohl(serverid),
                            m_localaddr.get_ip_address()));
        free_packet(request);
        return;
    }

    if (BOOT_REQUEST != request->op)
    {
        ACE_DEBUG ((LM_INFO, "CDHCPServer::HandleDHCP, op(%d) is NOT BOOT_REQUEST.\n", request->op));
        free_packet(request);
        return;
    }

    //get the dhcp message type
    char type = '\0';
    struct dhcp_option *option = request->options;
    while (NULL != option)
    {
        if (DHO_DHCP_MESSAGE_TYPE == option->code)
        {
            type = *option->value;
            break;
        }

        option = option->next;
    }

    if ('\0' == type)
    {
        ACE_DEBUG ((LM_ERROR, "CDHCPServer::HandleDHCP, DHCP message type error. type = 0\n"));
        free_packet(request);
        return;
    }
    
    switch (type)
    {
        case DHCP_DISCOVER:
            HandleDiscover(request);
            break;
        case DHCP_RELEASE:
            HandleRelease(request);
            break;
        case DHCP_INFORM:
            HandleInform(request);
            break;
        case DHCP_REQUEST:
            HandleRequest(request);
            break;
        case DHCP_DECLINE:
            HandleDecline(request);
            break;
        default:
            ACE_DEBUG ((LM_ERROR, "CDHCPServer::HandleDHCP, unsupported DHCP message type(%d)\n", type));
            free_packet(request);
            break;
    }
    
    return;
}

//Add Session
int CDHCPServer::AddSession(uint64_t sessionid, CDHCPSession *psession)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, -1);

    ACE_DEBUG ((LM_DEBUG, "CDHCPServer::AddSession, sessionid=%#x\n", sessionid));
    
    CCmAutoPtr<CDHCPSession> session(psession);
    DHCPSESSIONType::iterator it = m_DhcpSessions.find(sessionid);
    if (it != m_DhcpSessions.end())
    {
        ACE_DEBUG ((LM_ERROR, "Session with id %#x already exists.\n", sessionid));
        return -1;
    }

    m_DhcpSessions[sessionid] = session;
    return 0;
}

//Find Session
CDHCPSession *CDHCPServer::FindSession(uint64_t sessionid)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, NULL);

    DHCPSESSIONType::iterator it = m_DhcpSessions.find(sessionid);
    if (it == m_DhcpSessions.end())
    {
        return NULL;
    }
    CCmAutoPtr<CDHCPSession> &session = it->second;
    return session.Get();
}

//Remove Session
int CDHCPServer::RemoveSession(uint64_t sessionid)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, -1);

    ACE_DEBUG ((LM_DEBUG, "CDHCPServer::RemoveSession, sessionid=%#x\n", sessionid));

    DHCPSESSIONType::iterator it = m_DhcpSessions.find(sessionid);
    if (it == m_DhcpSessions.end())
    {
        ACE_DEBUG ((LM_INFO, "No such session.\n"));
        return -1;
    }
    
    m_DhcpSessions.erase(it);

    return 0;
}

//Clear All Sessions
void CDHCPServer::ClearAllSession()
{
    ACE_DEBUG ((LM_DEBUG, "CDHCPServer::ClearAllSession\n"));
    
    DHCPSESSIONType::iterator it = m_DhcpSessions.begin();
    while (it != m_DhcpSessions.end())
    {
        it = m_DhcpSessions.erase(it);
    } 
}

//Get Session Id
uint64_t CDHCPServer::GetSessionID(struct dhcp_packet *request)
{
    uint64_t sessionid = 0;
    memcpy(&sessionid, request->chaddr, 6);
    return sessionid;
}

uint64_t CDHCPServer::GetSessionID(struct DHCPNetworkConfig *pnetconfig)
{
    uint64_t sessionid = 0;
    memcpy(&sessionid, pnetconfig->hardware_address, 6);
    return sessionid;
}

uint64_t CDHCPServer::GetSessionID(const Auth_Response &reponse)
{
    uint64_t sessionid = 0;
    memcpy(&sessionid, reponse.mac, AUTHMGR_MAC_LENGTH);
    return sessionid;  
}

uint64_t CDHCPServer::GetSessionID(const Sm_Kick_User* kickInfo)
{
    uint64_t sessionid = 0;

    memcpy(&sessionid, kickInfo->mac, SESSION_MGR_MAC_SIZE);
    return sessionid;
}

//Send Broadcast Reply
int CDHCPServer::SendBroadcastReply(struct dhcp_packet *response)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPServer::SendBroadcastReply\n"));

    return SendDHCP(response, INADDR_BROADCAST);
}

//Auth Response Handle
int CDHCPServer::HandleAuthResponse(const Auth_Response &response)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPServer::HandleAuthResponse\n"));

    uint64_t sessionid = GetSessionID(response);
    CDHCPSession *session = FindSession(sessionid);
    if (session == NULL)
    {
        ACE_DEBUG ((LM_ERROR, "(%P|%t) CDHCPServer::HandleAuthResponse, can not find session(%#x)\n", sessionid));
        return -1;
    }

    DHCPNetworkConfig config;
    GetConfigFromResponse(response, config);
    session->HandleDiscover(config);
    return 0;
}

//Get configuration from the response 
int CDHCPServer::GetConfigFromResponse(const Auth_Response &response, DHCPNetworkConfig &config)
{
    //config地址都是网络顺序
    uint32_t addr = response.subscriberIP;
    uint32_t dns1 = response.primaryDNS;
    uint32_t dns2 = response.secondaryDNS;
    uint32_t gateway = response.gatewayaddr;
    uint32_t netmask = response.subscriberIPMask;

    ::memcpy(config.hardware_address, response.mac, 6);
    config.ip_address = htonl(addr);
    config.router = htonl(gateway);
    config.netmask = htonl(netmask);
    config.dns1 = htonl(dns1);
    config.dns2 = htonl(dns2);
    config.leasetime = htonl(response.leaseTime);
    
    return -1;
}

// clienip should be in host byte order.
//Send DHCP
int CDHCPServer::SendDHCP(struct dhcp_packet *response, uint32_t clientip)
{
    static char buffer[1024*2];
    ::memset(buffer, 0, sizeof buffer);
    
    size_t dhcplen = 0;
    ACE_TCHAR addr_str[BUFSIZ]={0};
    ACE_INET_Addr inetaddr;
    
    struct sockaddr_in peeraddr = {0};
    peeraddr.sin_family = AF_INET;
    peeraddr.sin_port = htons(BOOTP_REPLAY_PORT);
    peeraddr.sin_addr.s_addr = htonl(clientip);
    inetaddr.set_addr (&peeraddr, sizeof(peeraddr));

    dhcplen = serialize(response, buffer, sizeof buffer);

    inetaddr.addr_to_string (addr_str, sizeof addr_str);

    ssize_t sendsize =  ACE_OS::sendto (m_handler,
                                                     (char const *)buffer,
                                                     dhcplen,
                                                     0,
                                                     (sockaddr const *)&peeraddr,
                                                     sizeof(peeraddr));

    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPServer::SendDHCP,sendsize=%d,peeraddr=%s\n",sendsize,addr_str));

    return 0;
}

//Get Remote Addr
int
CDHCPServer::GetRemoteAddr (ACE_Addr &sa) const
{
  int len = sa.get_size ();
  sockaddr *addr = reinterpret_cast<sockaddr *> (sa.get_addr ());

  if (ACE_OS::getpeername (this->get_handle (),
                           addr,
                           &len) == -1)
    return -1;

  sa.set_size (len);
  sa.set_type (addr->sa_family);
  return 0;
}

//Get Local Addr 
int
CDHCPServer::GetLocalAddr (ACE_Addr &sa) const
{
  int len = sa.get_size ();
  sockaddr *addr = reinterpret_cast<sockaddr *> (sa.get_addr ());

  if (ACE_OS::getsockname (this->get_handle (),
                           addr,
                           &len) == -1)
    return -1;

  sa.set_type (addr->sa_family);
  sa.set_size (len);
  return 0;
}

//Add User Request
int CDHCPServer::AddUserRequest(const Session_User_Ex &user)
{
    return m_ipoe.AddUserRequest(user);
}

// Delete User Request    
int CDHCPServer::DeleteUserRequest(const Session_Offline &user)
{
    return m_ipoe.DeleteUserRequest(user);
}


