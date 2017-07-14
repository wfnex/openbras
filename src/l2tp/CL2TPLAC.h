/***********************************************************************
 * 
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
#ifndef CL2TPLAC_H
#define CL2TPLAC_H
#include "IL2TPInterface.h"
#include "CReferenceControl.h"
#include <stdint.h>
#include <unordered_map>
#include "CL2TPMessage.h"
class CL2TPTunnel;
class CL2TPSession;
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

typedef std::unordered_map<CPairInetAddr,CCmAutoPtr<CL2TPTunnel>,InetAddrHash> UDPTransportsType;

class CL2TPManager;
class CL2TPLAC  : public IL2TPLAC, public CReferenceControl
{
public:
    CL2TPLAC(CL2TPManager &mgr);
    virtual ~CL2TPLAC();
    virtual uint32_t AddReference();
    virtual uint32_t ReleaseReference();

    virtual int MakeTunnel(IL2TPTunnelIndication *psink,
        const ACE_INET_Addr &serveraddr);

    int AddChannel(const ACE_INET_Addr &peeraddr, 
        CCmAutoPtr<CL2TPTunnel> &channel);
    CL2TPTunnel *FindChannel(const ACE_INET_Addr &peeraddr);
    int RemoveChannel(const ACE_INET_Addr &peeraddr);
    int RemoveChannel(CCmAutoPtr<CL2TPTunnel> &channel);
    CL2TPManager &GetL2TPManager(){return m_l2tpmgr;}
    void OnTunnelResult(int result, CL2TPTunnel *ptunnel);
private:
    ACE_Thread_Mutex m_mutex;
    UDPTransportsType m_L2tpServers;
    CL2TPManager &m_l2tpmgr;
    uint16_t m_tunnelidbase;
    IL2TPTunnelIndication *m_psink;
    ACE_INET_Addr m_localaddr;
};


#endif//CL2TPSERVERMANAGER_H

