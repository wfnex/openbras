/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
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

