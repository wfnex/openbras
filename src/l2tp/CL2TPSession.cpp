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


#include "CL2TPSession.h"
#include "CL2TPTunnel.h"
#include "CL2TPManager.h"
CL2TPSession::CL2TPSession(CL2TPTunnel &tunnel, uint16_t localcid, uint16_t peercid, L2tpSessionProxy &proxy, bool isProxy)
    :m_tunnel(tunnel)
    ,m_localcid(localcid)
    ,m_peercid(peercid)
    ,m_fbit(0)
    ,m_ourfbit(0)
    ,m_lbit(LBIT)
    ,m_isconnected(false)
    ,m_bisProxy(isProxy)
    ,m_psink(NULL)
{
    ::memcpy((char*)&m_proxy,(char *)&proxy,sizeof(m_proxy));
    
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPSession::CL2TPSession, localcid=%d,peercid=%d\n",localcid,peercid)); 
}
CL2TPSession::~CL2TPSession()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPSession::~CL2TPSession\n")); 
}

//Add Reference
uint32_t CL2TPSession::AddReference()
{
    return CReferenceControl::AddReference();
}

//Release Reference
uint32_t CL2TPSession::ReleaseReference()
{
    return CReferenceControl::ReleaseReference();
}

void CL2TPSession::OpenWithSink(IL2TPSessionSink *psink)
{
    m_psink = psink;
}

//Send Data
int CL2TPSession::SendData(const char *data, size_t datasize)
{
    ACE_DEBUG ((LM_DEBUG, 
                "CL2TPSession::SendPayLoad, size=%d\n",
                datasize));

    char *payload = NULL;
    static char buffer[MAX_RECV_SIZE];
    
    size_t totalsize =sizeof (struct payload_hdr_mini)+datasize;
    struct payload_hdr_mini *p = (struct payload_hdr_mini *)buffer;

    if (totalsize>sizeof(buffer))
    {
        return -1;
    }

    p->ver = htons (LBIT | VER_L2TP);
    p->length = htons ((_u16)totalsize);

    p->tid = htons (m_tunnel.GetPeerTID());
    p->cid = htons (m_peercid);
    payload = buffer+sizeof (struct payload_hdr_mini);

    ::memcpy(payload, data, datasize);
    m_tunnel.SendData(buffer,totalsize);
    return 0;

}

//Get Local Call Id
uint16_t CL2TPSession::GetLocalCID() const
{
    return m_localcid;
}

//Get Peer Call Id
uint16_t CL2TPSession::GetPeerCID() const
{
    return m_peercid;
}

//Get Local Tunnel Id
uint16_t CL2TPSession::GetLocalTID() const
{
    return m_tunnel.GetLocalTID();
}

//Get Peer Tunnel Id
uint16_t CL2TPSession::GetPeerTID() const
{
    return m_tunnel.GetPeerTID();
}

//disconnect
void CL2TPSession::Disconnect()
{
    
}

//payload handle
int CL2TPSession::HandlePayLoad(const char *data, size_t datasize)
{
    if (m_psink)
    {
        m_psink->OnDataRecv(data,datasize);
    }
    return 0;
}

//Call Connevted 
void CL2TPSession::CallConnected()
{
    m_isconnected=true;
}

//Start Call
int CL2TPSession::StartCall()
{
    char requestbuffer[MAX_RECV_SIZE]={0};
    CL2TPControllMessage requesticrq(requestbuffer,sizeof(requestbuffer));
    requesticrq.add_message_type_avp (ICRQ);
    requesticrq.add_callid_avp (m_localcid);
    requesticrq.add_serno_avp(0);
    requesticrq.add_bearer_avp(0);

    if (m_bisProxy)
    {
        requesticrq.add_init_lcp_req_avp(m_proxy.sInitRcvLcpConf,m_proxy.wInitRcvLcpConfLen);
        requesticrq.add_last_lcp_send_avp(m_proxy.sLastSendLcpConf,m_proxy.wLastSendLcpConfLen);
        requesticrq.add_last_lcp_recv_avp(m_proxy.sLastRcvLcpConf,m_proxy.wLastRcvLcpConfLen);
        requesticrq.add_proxy_auth_type_avp(m_proxy.wAuthenType);
        requesticrq.add_proxy_auth_name_avp(m_proxy.sAuthenName,m_proxy.wAuthenNameLen);

        requesticrq.add_proxy_auth_challenge_avp(m_proxy.sAuthenChallenge,m_proxy.wAuthenChallengeLen);
        requesticrq.add_proxy_auth_id_avp(m_proxy.wAuthenID);
    }

    size_t icrqsize = requesticrq.add_control_hdr(GetPeerTID(),0,m_tunnel.GetIncreaseNS(), m_tunnel.GetControlRecSeqNum());
    return m_tunnel.SendData(requestbuffer,icrqsize);
}

//Outgoing Call Request handle
int CL2TPSession::HandleOutgoingCallRequest(const CL2TPControllMessage &msg)
{
    ACE_UNUSED_ARG(msg);
    return -1;

}
//Outgoing Call Reply handle
int CL2TPSession::HandleOutgoingCallReply(const CL2TPControllMessage &msg)
{
    ACE_UNUSED_ARG(msg);
    return -1;

}

//Outgoing Call Connected handle
int CL2TPSession::HandleOutgoingCallConnected(const CL2TPControllMessage &msg)
{
    ACE_UNUSED_ARG(msg);
    return -1;

}

//Incoming Call Request handle
int CL2TPSession::HandleIncomingCallRequest(const CL2TPControllMessage &msg)
{
    ACE_UNUSED_ARG(msg);
    return -1;
#if 0
    CL2TPTunnel *tunnel = FindChannel(msg.GetTID());
    if (tunnel == NULL)
    {
        return -1;
    }

    uint16_t localcid = tunnel->GetIncreaseCid();
    
    CCmAutoPtr<CL2TPSession> session(new CL2TPSession(*tunnel,localcid,msg.GetAssignedCID()));
    if (session.Get() == NULL)
    {
        return -1;
    }
    
    tunnel->AddSession(localcid, session);

    if (CheckControlMessage(msg,tunnel, session.Get()) == -1)
    {
        return -1;
    }

    char responsebuffer[MAX_RECV_SIZE]={0};
    CL2TPControllMessage response(responsebuffer,sizeof(responsebuffer));
    response.add_message_type_avp (ICRP);
    response.add_callid_avp (localcid);

    size_t responsesize = response.add_control_hdr(tunnel->GetPeerTunnelID(),msg.GetAssignedCID(),tunnel->GetIncreaseNS(), tunnel->GetControlRecSeqNum());
    return SendData(responsebuffer,responsesize,msg.GetPeerAddr());
#endif
    return 0;
}

//Incoming Call Reply handle
int CL2TPSession::HandleIncomingCallReply(const CL2TPControllMessage &msg)
{

    m_peercid = msg.GetAssignedCID();

    char responsebuffer[MAX_RECV_SIZE]={0};
    CL2TPControllMessage response(responsebuffer,sizeof(responsebuffer));
    response.add_message_type_avp (ICCN);
    response.add_txspeed_avp(1000000000);
    //response.add_frame_avp(1000000000);
    ACE_DEBUG ((LM_DEBUG, "CL2TPSession::HandleIncomingCallReply, peercid=%d\n",m_peercid));

    size_t responsesize = response.add_control_hdr(GetPeerTID(),m_peercid,m_tunnel.GetIncreaseNS(), m_tunnel.GetControlRecSeqNum());
    m_tunnel.SendData(responsebuffer,responsesize);

    m_tunnel.OnSessionCallBack(0,this);
    return 0;
}

//Incoming Call Connected handle
int CL2TPSession::HandleIncomingCallConnected(const CL2TPControllMessage &msg)
{
    ACE_UNUSED_ARG(msg);
    return 0;
#if 0
    CL2TPTunnel *tunnel = FindChannel(msg.GetTID());
    if (tunnel == NULL)
    {
        return -1;
    }

    CL2TPSession *psession = tunnel->FindSession(msg.GetCID());
    if (psession == NULL)
    {
        return -1;
    }
    
    if (CheckControlMessage(msg,tunnel, psession) == -1)
    {
        return -1;
    }

    psession->CallConnected();
    
    return ControlZLB(tunnel,psession);
#endif
    return 0;
}

//Call Disconnect Notify handle
int CL2TPSession::HandleCallDisconnectNotify(const CL2TPControllMessage &msg)
{
    m_tunnel.ControlZLB(&m_tunnel,this);

    if (m_psink)
    {
        m_psink->OnSessionDisconnect(0,this);
    }
    m_tunnel.RemoveSession(msg.GetCID());
    return 0;
}

//Controll Message Handle
int CL2TPSession::HandleControllMessage(const CL2TPControllMessage &controllmsg)
{
    //m_control_rec_seq_num++;
    int result = -1;
    ACE_DEBUG ((LM_DEBUG, "CL2TPSession::HandleControllMessage, type=%d\n",controllmsg.GetMessageType()));

    switch(controllmsg.GetMessageType())
    {
        case OCRQ:
            result = HandleOutgoingCallRequest(controllmsg);
            break;
        case OCRP:
            result = HandleOutgoingCallReply(controllmsg);
            break;
        case OCCN:
            result = HandleOutgoingCallConnected(controllmsg);
            break;
        case ICRQ:
            result = HandleIncomingCallRequest(controllmsg);
            break;
        case ICRP:
            result = HandleIncomingCallReply(controllmsg);
            break;
        case ICCN:
            result = HandleIncomingCallConnected(controllmsg);
            break;
        case CDN:
            result = HandleCallDisconnectNotify(controllmsg);
            break;
    
        default:
            break;
    };

    return result;
}

