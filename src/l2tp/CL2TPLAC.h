/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
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

