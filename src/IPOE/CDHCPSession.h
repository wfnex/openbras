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

#ifndef CDHCPTRANSPORT_H
#define CDHCPTRANSPORT_H

#include "aceinclude.h"
#include "CReferenceControl.h"
#include "DHCPPacket.h"

#define LEASE_TIME_OUT (uint32_t)(60)

class CDHCPServer;

struct DHCPNetworkConfig
{
    DHCPNetworkConfig()
    {
        ::memset(this, 0, sizeof(*this));
    }
    
    char hardware_address[6];
    uint32_t ip_address; // In network byte order
    uint32_t router;        // Relay router/gateway address. In network byte order.
    uint32_t netmask;    // In network byte order
    uint32_t dns1;          // Primary DNS address. In network byte order
    uint32_t dns2;         // Secondary DNS address. In network byte order
    uint32_t leasetime;   // In network byte order
};

class CDHCPSession : public CReferenceControl, public ACE_Event_Handler
{
public:
    CDHCPSession(uint64_t sessionid, CDHCPServer &Server);
    virtual ~CDHCPSession();

    // Design about the lease timer:
    // StartLeaseTimer() is called by HandleDiscovery() and HandleRequest().
    // Firstly, if we don't receive DHCP REQUEST targeted at us in (lease time + 10) seconds after we received DISCOVERY,
    // the session will be deleted. 
    // Secondly, if we receive REQUEST targeted at us in (lease time + 10) seconds, we canel the lease timer scheduled in 
    // Handlediscoery(), and then schedule it to deal with REQUEST properly.
    void StartLeaseTimer(int second);
    void StopLeaseTimer();

    // For interface ACE_Event_Handler
    virtual int handle_timeout (const ACE_Time_Value &current_time,
                                const void *act = 0);
    
    int HandleDiscover(const DHCPNetworkConfig &config);
    void SetDiscoverRequest(struct dhcp_packet *request);
    void GetNetworkConfig(DHCPNetworkConfig &config);
    int HandleRequest(struct dhcp_packet *request);
    int HandleInform(struct dhcp_packet *request);
    int HandleDecline(struct dhcp_packet *request);
    void HandleRelease(struct dhcp_packet *request);
    void DestroySession();
    void LeaseTimeOut();
    void RemoveUser();
    void AddUser();

private:
    CDHCPServer &m_Server;
    uint64_t m_sessionid;
    struct dhcp_packet* m_discovery;
    DHCPNetworkConfig m_config;
    uint32_t m_remaintimer;
    bool m_isAddUser;
};

#endif//CDHCPTransport

