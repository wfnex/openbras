/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
**********************************************************************/
#include "CL2TPLAC.h"
#include "CL2TPTunnel.h"
#include "CL2TPManager.h"
#include "CL2TPSession.h"


CL2TPLAC::CL2TPLAC(CL2TPManager &mgr)
    :m_l2tpmgr(mgr)
    ,m_tunnelidbase(1)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPLAC::CL2TPLAC\n")); 
}

CL2TPLAC::~CL2TPLAC()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPLAC::~CL2TPLAC\n")); 
}
uint32_t CL2TPLAC::AddReference()
{
    return CReferenceControl::AddReference();
}
uint32_t CL2TPLAC::ReleaseReference()
{
    return CReferenceControl::ReleaseReference();
}

int CL2TPLAC::AddChannel(const ACE_INET_Addr &peeraddr, 
        CCmAutoPtr<CL2TPTunnel> &channel)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, -1);

    CPairInetAddr addrPair(m_localaddr, peeraddr);

    UDPTransportsType::iterator it = m_L2tpServers.find(addrPair);
    if (it != m_L2tpServers.end())
    {
        return -1;
    }

    UDPTransportsType::value_type nodeNew(addrPair, channel);
    m_L2tpServers.insert(nodeNew);

    return 0;
}

CL2TPTunnel *CL2TPLAC::FindChannel(const ACE_INET_Addr &peeraddr)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, NULL);
    CPairInetAddr addrPair(m_localaddr, peeraddr);

    UDPTransportsType::iterator it = m_L2tpServers.find(addrPair);
    if (it == m_L2tpServers.end())
    {
        return NULL;
    }
    CCmAutoPtr<CL2TPTunnel> &channel = it->second;
    return channel.Get();
}

int CL2TPLAC::RemoveChannel(const ACE_INET_Addr &peeraddr)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, -1);

    CPairInetAddr addrPair(m_localaddr, peeraddr);

    UDPTransportsType::iterator it = m_L2tpServers.find(addrPair);
    if (it == m_L2tpServers.end())
    {
        return -1;
    }
    m_L2tpServers.erase(it);
    return 0;
}

int CL2TPLAC::RemoveChannel(CCmAutoPtr<CL2TPTunnel> &channel)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, -1);
    ACE_INET_Addr peeraddr = channel->GetPeerAddress();
    CPairInetAddr addrPair(m_localaddr, peeraddr);
    UDPTransportsType::iterator it = m_L2tpServers.find(addrPair);
    if (it == m_L2tpServers.end())
    {
        return -1;
    }
    m_L2tpServers.erase(it);
    return 0;
}


int CL2TPLAC::MakeTunnel(IL2TPTunnelIndication *psink,
        const ACE_INET_Addr &serveraddr)
{
    m_psink = psink;
    CL2TPTunnel *pchannel = FindChannel(serveraddr);
    if (pchannel == NULL)
    {
        uint16_t localtid = m_tunnelidbase++;
        CCmAutoPtr<CL2TPTunnel> tunnel(new CL2TPTunnel(*this,serveraddr,localtid,0));
        if (tunnel.Get() == NULL)
        {
            return -1;
        }
        if (AddChannel(serveraddr, tunnel) == -1)
        {
            ACE_DEBUG ((LM_ERROR,ACE_TEXT ("(%P|%t) CL2TPLAC::MakeTunnel add channel\n")));

            return -1;
        }
        tunnel->StartConnectRequest();
    }
    else
    {
        if (pchannel->IsConnected())
        {
            psink->OnTunnelIndication(0,pchannel);
        }
    }

    return 0;
}


void CL2TPLAC::OnTunnelResult(int result, CL2TPTunnel *ptunnel)
{
    if (m_psink)
    {
        m_psink->OnTunnelIndication(result,ptunnel);
    }
}



