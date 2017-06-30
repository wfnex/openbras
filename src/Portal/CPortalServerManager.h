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

#ifndef CPORTALSERVERMANAGER_H
#define CPORTALSERVERMANAGER_H
#include "aceinclude.h"
#include <stdint.h>
#include <unordered_map>
#include "CReferenceControl.h"

class CPortalServerChannel;
class CPairInetAddr
{
public:
    CPairInetAddr()
        : m_dwIpSrc(0)
        , m_dwIpDst(0)
        , m_wPortSrc(0)
        , m_wPortDst(0)
    {
    }

    CPairInetAddr(const ACE_INET_Addr &aSrc, const ACE_INET_Addr &aDst)
        : m_dwIpSrc(((struct sockaddr_in *)aSrc.get_addr())->sin_addr.s_addr)
        , m_dwIpDst(((struct sockaddr_in *)aDst.get_addr())->sin_addr.s_addr)
        , m_wPortSrc(aSrc.get_port_number())
        , m_wPortDst(aDst.get_port_number())
    {
    }

    int GetHashValue() const 
    {
        // this hash function is copied from linux kernel
        // source code whose flie name is "net/ipv4/Tcp_ipv4.c".
        int h = ((m_dwIpSrc ^ m_wPortSrc) ^ (m_dwIpDst ^ m_wPortDst));
        h ^= h>>16;
        h ^= h>>8;
        return h;
    }

    bool operator == (const CPairInetAddr &aRight) const 
    {
        return m_dwIpSrc == aRight.m_dwIpSrc && 
                m_dwIpDst == aRight.m_dwIpDst && 
                m_wPortSrc == aRight.m_wPortSrc && 
                m_wPortDst == aRight.m_wPortDst;
    }

public:
    uint32_t m_dwIpSrc;
    uint32_t m_dwIpDst;
    uint16_t m_wPortSrc;
    uint16_t m_wPortDst;
};

struct InetAddrHash
{
    size_t operator()(const CPairInetAddr &addr) const
    {
        return addr.GetHashValue();
    }
};

typedef std::unordered_map<CPairInetAddr, CCmAutoPtr<CPortalServerChannel>,InetAddrHash> UDPTransportsType;

class CPortalManager;
class CPortalServerManager  : public ACE_Event_Handler
{
public:
    CPortalServerManager(CPortalManager &mgr);
    ~CPortalServerManager();
    virtual int StartListen(const ACE_INET_Addr &localaddr);
    virtual int StopListen();


    //ACE_Event_Handler interface，子类可以重载这些函数
    virtual int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE);
    virtual ACE_HANDLE get_handle (void) const;
    virtual int handle_close (ACE_HANDLE handle,
                            ACE_Reactor_Mask close_mask);

    int GetLocalAddr (ACE_INET_Addr &address) const;
public:
    int AddChannel(const ACE_INET_Addr &peeraddr, 
        const ACE_INET_Addr &localaddr,
        CCmAutoPtr<CPortalServerChannel> &channel);
    CPortalServerChannel *FindChannel(const ACE_INET_Addr &peeraddr, 
        const ACE_INET_Addr &localaddr);
    int RemoveChannel(const ACE_INET_Addr &peeraddr, 
        const ACE_INET_Addr &localaddr);
    int RemoveChannel(CCmAutoPtr<CPortalServerChannel> &channel);

    void Dump();

    CPortalManager &GetPortalManager();
private:
    ACE_Thread_Mutex m_mutex;
    UDPTransportsType m_PortalServers;
    ACE_HANDLE m_handler;
    ACE_INET_Addr m_localaddr;
    CPortalManager &m_portalmgr;
};


#endif//CPORTALSERVERMANAGER_H

