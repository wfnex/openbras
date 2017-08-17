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


#include "CDHCPSession.h"
#include "CDHCPServer.h"

CDHCPSession::CDHCPSession(uint64_t sessionid, CDHCPServer &Server)
    : m_Server(Server)
    , m_sessionid(sessionid)
    , m_discovery(NULL)
    , m_remaintimer(0)
    , m_isAddUser(false)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPSession::CDHCPSession\n"));

    //StartLeaseTimer();
}

CDHCPSession::~CDHCPSession()
{
    ACE_DEBUG ((LM_DEBUG, "(%P|%t) CDHCPSession::~CDHCPSession, m_sessionid=%#x\n", m_sessionid));

    if (m_discovery)
    {
        free_packet(m_discovery);
        m_discovery = NULL;
    }
    
    RemoveUser();

    StopLeaseTimer();
}

//Start Lease Timer
void CDHCPSession::StartLeaseTimer(int second)
{
    ACE_DEBUG ((LM_DEBUG, "(%P|%t) CDHCPSession::StartLeaseTimer, second=%d\n", second));

    StopLeaseTimer();

    ACE_Time_Value interval(second+10);
    ACE_Reactor::instance()->schedule_timer(this, 0, interval, interval);
}

//Stop Lease Timer
void CDHCPSession::StopLeaseTimer()
{
    ACE_DEBUG ((LM_DEBUG, "CDHCPSession::StopLeaseTimer\n"));
    ACE_Reactor::instance()->cancel_timer(this);
}

//Timeout handle
int CDHCPSession::handle_timeout (const ACE_Time_Value &current_time,
                            const void *act)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPSession::handle_timeout\n"));

    LeaseTimeOut();
    
    return 0;
}

//Discovery handle
int CDHCPSession::HandleDiscover(const DHCPNetworkConfig &config)
{
    ACE_DEBUG ((LM_DEBUG, "(%P|%t) CDHCPSession::HandleDiscover,start\n"));

    ::memcpy(&m_config, &config, sizeof(config));
    if (m_discovery == NULL)
    {
        ACE_DEBUG ((LM_ERROR, "m_discovery is NULL.\n"));
        return -1;
    }
    
    struct dhcp_packet *response = (struct dhcp_packet*)::malloc(sizeof(struct dhcp_packet));
    if (NULL == response)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc resonse.\n"));
        return -1;
    }
    
    ::memset(response, 0, sizeof(struct dhcp_packet));
    
    response->op = BOOT_REPLY;
    response->htype = m_discovery->htype;
    response->hlen = m_discovery->hlen;
    response->hops = 1;
    memcpy(response->xid, m_discovery->xid, 4);
    memcpy(response->yiaddr, &config.ip_address, 4);
    memcpy(response->flags, m_discovery->flags, 2);
    memcpy(response->chaddr, m_discovery->chaddr, 16);
    
    //options
    //message type
    struct dhcp_option *packet_type = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == packet_type)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc packet_type.\n"));
        free_packet(response);
        return -1;
    }
    memset(packet_type, 0, sizeof(struct dhcp_option));
    packet_type->code = DHO_DHCP_MESSAGE_TYPE;
    packet_type->value = (char *)malloc(1);
    if (NULL == packet_type->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc packet_type->value.\n"));
        free_packet(response);
        return -1;
    }
    *packet_type->value = DHCP_OFFER;
    packet_type->length = 1;
    response->options = packet_type;

    //server identifier
    struct dhcp_option *server_identifier = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == server_identifier)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc server_identifier.\n"));
        free_packet(response);
        return -1;
    }
    memset(server_identifier, 0, sizeof(struct dhcp_option));
    server_identifier->code = DHO_DHCP_SERVER_IDENTIFIER;
    server_identifier->value = (char *)malloc(4);
    if (NULL == server_identifier->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc server_identifier->value.\n"));
        free_packet(response);
        return -1;
    }

    uint32_t ipaddr = m_Server.GetLocalAddrByte();
    ::memcpy(server_identifier->value, &ipaddr, 4);

    server_identifier->length = 4;
    packet_type->next = server_identifier;
    
     //lease time
    struct dhcp_option *lease_time = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == lease_time)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc lease_time.\n"));
        free_packet(response);
        return -1;
    }
    memset(lease_time, 0, sizeof(struct dhcp_option));
    lease_time->code = DHO_DHCP_LEASE_TIME;
    lease_time->value = (char *)malloc(4);
    if (NULL == lease_time->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc lease_time->value.\n"));
        free_packet(response);
        return -1;
    }
    uint32_t lease = config.leasetime;
    memcpy(lease_time->value, &lease, 4);
    lease_time->length = 4;
    server_identifier->next = lease_time;

#if 0
    //renew time
    struct dhcp_option *renew_time = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if(NULL == renew_time)
    {
    free_packet(response);
    return NULL;
    }
    memset(renew_time, 0, sizeof(struct dhcp_option));
    renew_time->code = DHO_DHCP_RENEWAL_TIME;
    renew_time->value = (char *)malloc(4);
    if(NULL == renew_time->value)
    {
    free_packet(response);
    return NULL;
    }
    uint32_t renew = m_Server.GetRenew();
    memcpy(renew_time->value, &renew, 4);
    renew_time->length = 4;
    lease_time->next = renew_time;
#endif

    //router/gateway
    struct dhcp_option *router = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == router)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc router.\n"));
        free_packet(response);
        return -1;
    }
    memset(router, 0, sizeof(struct dhcp_option));
    router->code = DHO_ROUTERS;
    router->value = (char *)malloc(4);
    if (NULL == router->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc router->value.\n"));
        free_packet(response);
        return -1;
    }
    memcpy(router->value, &config.router, 4);
    router->length = 4;
    lease_time->next = router;

    //netmask
    struct dhcp_option *subnet_mask = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == subnet_mask)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc subnet_mask.\n"));
        free_packet(response);
        return -1;
    }
    memset(subnet_mask, 0, sizeof(struct dhcp_option));
    subnet_mask->code = DHO_SUBNET;
    subnet_mask->value = (char *)malloc(4);
    if (NULL == subnet_mask->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc subnet_mask->value.\n"));
        free_packet(response);
        return -1;
    }
    memcpy(subnet_mask->value, &config.netmask, 4);
    subnet_mask->length = 4;
    router->next = subnet_mask;

    //dns
    struct dhcp_option *dns_server = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == dns_server)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc dns_server.\n"));
        free_packet(response);
        return -1;
    }
    memset(dns_server, 0, sizeof(struct dhcp_option));
    dns_server->code = DHO_DOMAIN_NAME_SERVERS;
    dns_server->value = (char *)malloc(8);
    if (NULL == dns_server->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc dns_server->value.\n"));
        free_packet(response);
        return -1;
    }
    memcpy(dns_server->value, &config.dns1, 4);
    memcpy(dns_server->value + 4, &config.dns2, 4);
    dns_server->length = 8;
    subnet_mask->next = dns_server;

    int sendresult = 0;
    
    bool isbroadcast = is_broadcast(m_discovery);
    if (isbroadcast)
    {
        sendresult = m_Server.SendBroadcastReply(response);
    }
    else
    {
        sendresult = m_Server.SendDHCP(response, ntohl(config.ip_address));
    }

    StartLeaseTimer(ntohl(config.leasetime));
    free_packet(response);
    
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPSession::HandleDiscover end,sendresult=%d\n",sendresult));
    
    return 0;
}

//Set Discovery Request
void CDHCPSession::SetDiscoverRequest(struct dhcp_packet *request)
{
    if (m_discovery)
    {
        free_packet(m_discovery);
        m_discovery = NULL;
    }

    m_discovery = request;
}

//Get Network Config
void CDHCPSession::GetNetworkConfig(DHCPNetworkConfig &config)
{
    ::memcpy(&config, &m_config, sizeof(DHCPNetworkConfig));
}

//Request handle
int CDHCPSession::HandleRequest(struct dhcp_packet *request)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPSession::HandleRequest\n")); 

    DHCPNetworkConfig config;

    GetNetworkConfig(config);

    char type = DHCP_ACK;
    char requested_address[4] = {0};

    if (0 != memcmp(requested_address, request->ciaddr, 4))
    {
        memcpy(requested_address, request->ciaddr, 4);
    }
    else
    {
        struct dhcp_option *option = request->options;
        while (NULL != option)
        {
            if (DHO_DHCP_REQUESTED_ADDRESS == option->code && option->length >= 4)
            {
                memcpy(requested_address, option->value, 4);
                break;
            }

            option = option->next;
        }
    }

    uint32_t reqAddr = 0;
    ::memcpy(&reqAddr, requested_address, 4);

    if (0 != memcmp(&config.ip_address, requested_address, 4))
    {
        ACE_DEBUG ((LM_INFO, 
                            "IP the client requested is not equal to the one configured. Nak it.\n"
                            "config.ip_address=%#x, requested_address=%#x",
                            config.ip_address,
                            reqAddr));
        type = DHCP_NAK;
    }

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
    memcpy(response->yiaddr, requested_address, 4);  // Commented by mazhh: fill in with requested_address or config.ip_address,
                                                                              //even in the response of type Nak? This is not important in Nak.
    memcpy(response->flags, request->flags, 2);
    memcpy(response->chaddr, request->chaddr, 16);

#if 0
    if(DHCP_ACK != type)
    {
        free_packet(request);
        free_packet(response);
        return response;
    }
#endif
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
    *packet_type->value = type;
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
    uint32_t ipaddr = m_Server.GetLocalAddrByte();
    ::memcpy(server_identifier->value,&ipaddr,4);

    server_identifier->length = 4;
    packet_type->next = server_identifier;

    //lease time
    struct dhcp_option *lease_time = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == lease_time)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc lease_time.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    memset(lease_time, 0, sizeof(struct dhcp_option));
    lease_time->code = DHO_DHCP_LEASE_TIME;
    lease_time->value = (char *)malloc(4);
    if (NULL == lease_time->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc lease_time->value.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    uint32_t lease = config.leasetime;
    memcpy(lease_time->value, &lease, 4);
    lease_time->length = 4;
    server_identifier->next = lease_time;

    #if 0
    //renew time
    struct dhcp_option *renew_time = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if(NULL == renew_time)
    {
        free_packet(response);
        free_packet(request);
        return;

    }
    memset(renew_time, 0, sizeof(struct dhcp_option));
    renew_time->code = DHO_DHCP_RENEWAL_TIME;
    renew_time->value = (char *)malloc(4);
    if(NULL == renew_time->value)
    {
        free_packet(response);
        free_packet(request);
        return;

    }
    uint32_t renew = GetRenew();
    memcpy(renew_time->value, &renew, 4);
    renew_time->length = 4;
    lease_time->next = renew_time;
    #endif

    //router/gateway
    struct dhcp_option *router = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == router)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc router.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    memset(router, 0, sizeof(struct dhcp_option));
    router->code = DHO_ROUTERS;
    router->value = (char *)malloc(4);
    if (NULL == router->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc router->value.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    memcpy(router->value, &config.router, 4);
    router->length = 4;
    lease_time->next = router;

    //netmask
    struct dhcp_option *subnet_mask = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == subnet_mask)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc subnet_mask.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    memset(subnet_mask, 0, sizeof(struct dhcp_option));
    subnet_mask->code = DHO_SUBNET;
    subnet_mask->value = (char *)malloc(4);
    if (NULL == subnet_mask->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc subnet_mask->value.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    memcpy(subnet_mask->value, &config.netmask, 4);
    subnet_mask->length = 4;
    router->next = subnet_mask;

    //dns
    struct dhcp_option *dns_server = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == dns_server)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc dns_server.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    memset(dns_server, 0, sizeof(struct dhcp_option));
    dns_server->code = DHO_DOMAIN_NAME_SERVERS;
    dns_server->value = (char *)malloc(8);
    if (NULL == dns_server->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc dns_server->value.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    memcpy(dns_server->value, &config.dns1, 4);
    memcpy(dns_server->value + 4, &config.dns2, 4);
    dns_server->length = 8;
    subnet_mask->next = dns_server;

    bool isbroadcast = is_broadcast(request);
    if (isbroadcast)
    {
        m_Server.SendBroadcastReply(response);
    }
    else
    {
        m_Server.SendDHCP(response, ntohl(reqAddr));
    }

    free_packet(response);
    free_packet(request);

    if (type == DHCP_ACK)
    {
        StartLeaseTimer(ntohl(config.leasetime));
        AddUser();
    }

    // When type == DHCP_NAK, the session will be deleted by the timer scheduled in CDHCPSession::HandleDiscover().
    
    return 0;
}

// We see DHCP inform packet as the same as DHCP request, except that when we acked DHCP request, we would 
// notify session manager to add user .
int CDHCPSession::HandleInform(struct dhcp_packet *request)
{
    DHCPNetworkConfig config;
    GetNetworkConfig(config);

    ACE_DEBUG ((LM_DEBUG, "CDHCPSession::HandleInform, request=%#x\n", request));
    
    struct dhcp_packet *response = (struct dhcp_packet*)::malloc(sizeof(struct dhcp_packet));
    if (NULL == response)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc response.\n"));
        free_packet(request);
        return -1;        
    }
    
    ::memset(response, 0, sizeof(struct dhcp_packet));

    response->op = BOOT_REPLY;
    response->htype = request->htype;
    response->hlen = request->hlen;
    response->hops = 1;
    memcpy(response->xid, request->xid, 4);
    memcpy(response->yiaddr, &config.ip_address, 4);
    memcpy(response->flags, request->flags, 2);
    memcpy(response->chaddr, request->chaddr, 16);

    //options
    //message type
    struct dhcp_option *packet_type = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == packet_type)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc packet_type option.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    memset(packet_type, 0, sizeof(struct dhcp_option));
    packet_type->code = DHO_DHCP_MESSAGE_TYPE;
    packet_type->value = (char *)malloc(1);
    if (NULL == packet_type->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc value in packet_type.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    *packet_type->value = DHCP_ACK;
    packet_type->length = 1;
    response->options = packet_type;

    //server identifier
    struct dhcp_option *server_identifier = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == server_identifier)
    {
        ACE_DEBUG ((LM_ERROR, "Faild to malloc server_identifer option.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    memset(server_identifier, 0, sizeof(struct dhcp_option));
    server_identifier->code = DHO_DHCP_SERVER_IDENTIFIER;
    server_identifier->value = (char *)malloc(4);
    if (NULL == server_identifier->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc value in server_identifier.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    uint32_t ipaddr = m_Server.GetLocalAddrByte();
    ::memcpy(server_identifier->value, &ipaddr, 4);

    server_identifier->length = 4;
    packet_type->next = server_identifier;

    //lease time
    struct dhcp_option *lease_time = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == lease_time)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc lease_time option.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    memset(lease_time, 0, sizeof(struct dhcp_option));
    lease_time->code = DHO_DHCP_LEASE_TIME;
    lease_time->value = (char *)malloc(4);
    if (NULL == lease_time->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc value in lease_time.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    uint32_t lease = config.leasetime;
    memcpy(lease_time->value, &lease, 4);
    lease_time->length = 4;
    server_identifier->next = lease_time;

    #if 0
    //renew time
    struct dhcp_option *renew_time = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if(NULL == renew_time)
    {
        free_packet(response);
        free_packet(request);
        return;

    }
    memset(renew_time, 0, sizeof(struct dhcp_option));
    renew_time->code = DHO_DHCP_RENEWAL_TIME;
    renew_time->value = (char *)malloc(4);
    if(NULL == renew_time->value)
    {
        free_packet(response);
        free_packet(request);
        return;

    }
    uint32_t renew = m_Server.GetRenew();
    memcpy(renew_time->value, &renew, 4);
    renew_time->length = 4;
    lease_time->next = renew_time;
    #endif
    
    //router/gateway
    struct dhcp_option *router = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == router)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc router option.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    ::memset(router, 0, sizeof(struct dhcp_option));
    router->code = DHO_ROUTERS;
    router->value = (char *)malloc(4);
    if (NULL == router->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc value in router.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    memcpy(router->value, &config.router, 4);
    router->length = 4;
    lease_time->next = router;

    //netmask
    struct dhcp_option *subnet_mask = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == subnet_mask)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc subnet_mask option.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    ::memset(subnet_mask, 0, sizeof(struct dhcp_option));
    subnet_mask->code = DHO_SUBNET;
    subnet_mask->value = (char *)malloc(4);
    if (NULL == subnet_mask->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc value in subnet_mask.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    memcpy(subnet_mask->value, &config.netmask, 4);
    subnet_mask->length = 4;
    router->next = subnet_mask;

    //dns
    struct dhcp_option *dns_server = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
    if (NULL == dns_server)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc dns_server option.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    ::memset(dns_server, 0, sizeof(struct dhcp_option));
    dns_server->code = DHO_DOMAIN_NAME_SERVERS;
    dns_server->value = (char *)malloc(8);
    if (NULL == dns_server->value)
    {
        ACE_DEBUG ((LM_ERROR, "Failed to malloc value in dns_server.\n"));
        free_packet(response);
        free_packet(request);
        return -1;
    }
    memcpy(dns_server->value, &config.dns1, 4);
    memcpy(dns_server->value + 4, &config.dns2, 4);
    dns_server->length = 8;
    subnet_mask->next = dns_server;

    int sendresult = 0;
    bool isbroadcast = is_broadcast(request);
    if (isbroadcast)
    {
        sendresult = m_Server.SendBroadcastReply(response);
    }
    else
    {
        sendresult = m_Server.SendDHCP(response, ntohl(config.ip_address));
    }

    free_packet(response);
    free_packet(request);

    StartLeaseTimer(ntohl(config.leasetime));

    return 0;
}

//Decline handle
int CDHCPSession::HandleDecline(struct dhcp_packet *request)
{
    ACE_DEBUG ((LM_DEBUG, "CDHCPSession::HandleDecline\n"));
    
    DestroySession();
    free_packet(request);
    return 0;
}

//Add User
void CDHCPSession::AddUser()
{
    ACE_DEBUG ((LM_DEBUG, "CDHCPSession::AddUser\n"));
    
    if (!m_isAddUser)
    {
        Session_User_Ex user;
        ::memset(&user, 0, sizeof(user));
        user.session = 0;
        ::memcpy(user.mac, m_config.hardware_address, 6);
        user.user_type = USER_TYPE_IPOE;
        user.access_type = 0;
        user.auth_type = 0;
        user.auth_state = 0;
        /*user.user_name*/
        //user.domain
        user.vrf = 0;
        user.user_ip = ntohl(m_config.ip_address);
        user.primary_dns = ntohl(m_config.dns1);
        user.secondary_dns = ntohl(m_config.dns2);
        user.pool_id = 0;
        user.intern_vlan = 0;
        user.extern_vlan = 0;

        ACE_DEBUG ((LM_DEBUG, "m_Server.AddUserRequest\n"));
        m_Server.AddUserRequest(user);
        
        m_isAddUser = true;
    }
}

// Commented by mazhh: is it necessary to add a flag to avoid duplicated deletion as m_isAddUser in AddUser()?
void CDHCPSession::RemoveUser()
{
    ACE_DEBUG ((LM_DEBUG, "CDHCPSession::RemoveUser\n"));
    
    Session_Offline user;
    ::memset(&user, 0, sizeof(user));
    ::memcpy(user.mac, m_config.hardware_address, 6);
    user.session = 0;
    user.user_type = USER_TYPE_IPOE;

    ACE_DEBUG ((LM_DEBUG, "m_Server.DeleteUserRequest\n"));
    m_Server.DeleteUserRequest(user);
}

//Release handle
void CDHCPSession::HandleRelease(struct dhcp_packet *request)
{
    ACE_DEBUG ((LM_DEBUG, "CDHCPSession::HandleRelease\n"));
    
    DHCPNetworkConfig config;

    GetNetworkConfig(config); 

    //1).ciaddr是否是config里的ipaddress相等
    DestroySession();
    free_packet(request);
}

//Remove Session
void CDHCPSession::DestroySession()
{
    m_Server.RemoveSession(m_sessionid);
}

//Lease Timeout handle
void CDHCPSession::LeaseTimeOut()
{
    DestroySession();
}


