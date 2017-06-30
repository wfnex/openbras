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

#ifndef CDHCPSERVER_H
#define CDHCPSERVER_H

#include "aceinclude.h"
#include <stdint.h>
#include <unordered_map>
#include "CDHCPSession.h"
#include "IAuthManager.h"
#include "CDHCPServerBCR.h"

class CIPOEModule;

// Design Notice:
// This class uses two sockets, m_broadcasthandler and m_handler. The former is for broadcast UDP, and the latter for
// unicast. The former is for receiving broadcast DHCP packets. When sending packets, we then call the latter, both 
// broadcast and unicast, which ensures that the source IP address in IP packet is set as the latter socket bound into. 
// When there are more than one interface at the DHCP server, this is of help. As if we use the former to send DHCP 
// packets, it is the operatine system that choose an IP address from the route table to fill the source IP address in the 
// IP packet.
class CDHCPServer : public ACE_Event_Handler
{
public:
    CDHCPServer(CIPOEModule &ipoe);
    virtual ~CDHCPServer();
    
    virtual int StartListen(const std::string &serverip,ACE_Reactor *preactor);
    virtual int StopListen();

    //打开和关闭通道
    virtual int Open(ACE_Reactor *reactor);
    virtual void Close();
    ssize_t RcvPacket(ACE_HANDLE fd, void * buf,
                       size_t n,
                       ACE_Addr & addr,
                       int flags = 0) const;


    //ACE_Event_Handler interface，子类可以重载这些函数
    virtual int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE);
    virtual ACE_HANDLE get_handle (void) const;
    virtual int handle_close (ACE_HANDLE handle,
                            ACE_Reactor_Mask close_mask);

public:
    int GetLocalAddr (ACE_INET_Addr &address) const;
    uint32_t GetLocalAddrByte() ;
    uint16_t GetServerPort();
    std::string GetServerIP();
    int AddSession(uint64_t sessionid,CDHCPSession *psession);
    CDHCPSession *FindSession(uint64_t sessionid);
    int RemoveSession(uint64_t sessionid);
    void ClearAllSession();
    uint64_t GetSessionID(struct dhcp_packet *request);
    uint64_t GetSessionID(struct DHCPNetworkConfig *pnetconfig);
    uint64_t GetSessionID(const Auth_Response &reponse);
    uint64_t GetSessionID(const Sm_Kick_User* kickInfo);
    int SendBroadcastReply(struct dhcp_packet *response);
    int HandleAuthResponse(const Auth_Response &response);
    int SendDHCP(struct dhcp_packet *response, uint32_t clientip);
    int GetConfigFromResponse(const Auth_Response &response, DHCPNetworkConfig &config);
    int AddUserRequest(const Session_User_Ex &user);
    int DeleteUserRequest(const Session_Offline &user);
    
protected:
    void HandleDecline(struct dhcp_packet *request);
    void HandleInform(struct dhcp_packet *request);
    void HandleRelease(struct dhcp_packet *request);
    void HandleRequest(struct dhcp_packet *request);
    void HandleDiscover(struct dhcp_packet *request);
    void HandleDHCP(const char *msg, size_t msgsize);
    int NakRequest(struct dhcp_packet *request);
    int GetRemoteAddr(ACE_Addr &sa) const;
    int GetLocalAddr (ACE_Addr &sa) const;
    
protected:
    typedef std::unordered_map<uint64_t, CCmAutoPtr<CDHCPSession>> DHCPSESSIONType;
    DHCPSESSIONType m_DhcpSessions;
    //ACE_SOCK_Dgram_Bcast m_broadcast;
    CIPOEModule &m_ipoe;
    ACE_HANDLE m_handler;
    ACE_INET_Addr m_localaddr;
    ACE_Reactor * m_reactor;
    std::string m_ip;
    uint16_t m_port;
    ACE_Thread_Mutex m_mutex;  // Mutex for m_DhcpSessions
    CDHCPServerBCR m_broadcasthandler;
};

#endif//CDHCPSERVER_H


