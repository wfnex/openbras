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

#include "CL2TPTunnel.h"
#include "CL2TPLAC.h"
#include "CL2TPManager.h"
CL2TPTunnel::CL2TPTunnel(CL2TPLAC &smgr,
        const ACE_INET_Addr &remote_addr,
        uint16_t localtid,
        uint16_t peertid)
        :m_smgr(smgr)
        ,m_remote_addr(remote_addr)
        ,m_ourtid(localtid)
        ,m_peertid(peertid)
        ,m_rws(0)
        ,m_control_seq_num(0)
        ,m_control_rec_seq_num(0)
        ,m_ourfc(0)
        ,m_ourtb(0)
        ,m_currentstate(0)
        ,m_sessionidbase(1)
        ,m_isconnected(false)
        ,m_helltimeoutcount(4)
        ,m_pTunnelSink(NULL)
        ,m_requestcount(0)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPTunnel::CL2TPTunnel,localtid=%d,peertid=%d\n",localtid,peertid)); 

    m_ourfc = ASYNC_FRAMING | SYNC_FRAMING;
    m_ourtb = (((_u64) rand ()) << 32) | ((_u64) rand ());
    ACE_Time_Value interval(2);
    ACE_Reactor::instance()->schedule_timer(this, 0, interval, interval);
    InitSocket();

}

CL2TPTunnel::~CL2TPTunnel()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPTunnel::~CL2TPTunnel\n")); 
    ACE_Reactor::instance()->cancel_timer(this);
    UnInitSocket();
}

//Add Reference
uint32_t CL2TPTunnel::AddReference()
{
    return CReferenceControl::AddReference();
}
//Release Reference
uint32_t CL2TPTunnel::ReleaseReference()
{
    return CReferenceControl::ReleaseReference();
}

//Get handle
ACE_HANDLE CL2TPTunnel::get_handle (void) const
{
    return m_handler;
}

//Close Handle
int CL2TPTunnel::handle_close (ACE_HANDLE handle,
                        ACE_Reactor_Mask close_mask)
{
    ACE_UNUSED_ARG(handle);
    ACE_UNUSED_ARG(close_mask);
    return -1;
}

//Open Tunnel
int CL2TPTunnel::Open(IL2TPTunnelSink *psink)
{
    m_pTunnelSink = psink;
    return 0;
}

//Make Incoming Call
int CL2TPTunnel::MakeIncomingCall(L2tpSessionProxy &proxy,bool bUserProxy)
{
    uint16_t localcid = GetIncreaseCid();
    
    CCmAutoPtr<CL2TPSession> session(new CL2TPSession(*this,localcid,0,
        proxy,
        bUserProxy));
    if (session.Get() == NULL)
    {
        return -1;
    }
    
    AddSession(localcid, session); 

    return session->StartCall();
}


//Disconnect
int CL2TPTunnel::Disconnect()
{
    char responsebuffer[MAX_RECV_SIZE]={0};

    CL2TPControllMessage request(responsebuffer,sizeof(responsebuffer));
    //struct buffer *Responsebuf = new_outgoing();
    request.add_message_type_avp (StopCCN);
    request.add_tunnelid_avp (m_ourtid);
    request.add_result_code_avp(RESULT_ERROR,0,(char*)"disconnect",strlen("disconnect"));
    size_t responsesize = request.add_control_hdr(GetPeerTID(),0,GetIncreaseNS(), GetControlRecSeqNum());
    int result = SendData(responsebuffer,responsesize);
    return result;    
}

//Timeout Handle
int CL2TPTunnel::handle_timeout (const ACE_Time_Value &current_time,
                          const void *act)
{
    ACE_UNUSED_ARG(act);
    ACE_UNUSED_ARG(current_time);

    if (m_requestcount>10)
    {
        m_requestcount = 0;
        m_smgr.OnTunnelResult(-1,NULL);
        m_smgr.RemoveChannel(m_remote_addr);
    }
    else
    {
        StartConnectRequest();
    }

    return 0;
}

//Add Session
int CL2TPTunnel::AddSession(uint16_t callid,CCmAutoPtr<CL2TPSession> &session)
{
    SESSIONType::iterator it = m_sessions.find(callid);
    if (it != m_sessions.end())
    {
        return -1;
    }

    m_sessions[callid] = session;

    return 0;
}

//Find Session
CL2TPSession *CL2TPTunnel::FindSession(uint16_t callid)
{
    SESSIONType::iterator it = m_sessions.find(callid);
    if (it != m_sessions.end())
    {
        CCmAutoPtr<CL2TPSession> &session = it->second;
        return session.Get();
    }

    return NULL;
}

//Remove Session
int CL2TPTunnel::RemoveSession(uint16_t callid)
{
    m_sessions.erase(callid);
    return 0;
}

//Get Increase Call Id
uint16_t CL2TPTunnel::GetIncreaseCid()
{
    return m_sessionidbase++;
}

//Get Increase NS
uint16_t CL2TPTunnel::GetIncreaseNS()
{
    return m_control_seq_num++;
}

//DeCrease NS
void CL2TPTunnel::DeCreaseNS()
{
    m_control_seq_num--;
}

//InCrease NR
void CL2TPTunnel::InCreaseNR()
{
    m_control_rec_seq_num=m_control_rec_seq_num+1;
}

//Send Data
int CL2TPTunnel::SendData(const char *data,size_t datasize)
{
    ACE_TCHAR remote_str[80]={0};

    m_remote_addr.addr_to_string (remote_str, 80);

    ssize_t result=ACE_OS::sendto (m_handler,
                         (char const *)data,
                         datasize,
                         0,
                         (sockaddr const *) m_remote_addr.get_addr (),
                         m_remote_addr.get_size ());

    ACE_DEBUG ((LM_DEBUG,
            ACE_TEXT ("(%P|%t)  CL2TPTunnel::TunnelSendData ")
            ACE_TEXT ("to %s,fd=%d,datasize=%d,result=%d\n"),
            remote_str,
            get_handle(),
            datasize,
            result));
    if (result<0)
    {
        return -1;
    }
    return 0;

}

//Init Socket
int CL2TPTunnel::InitSocket()
{
    
    ACE_DEBUG ((LM_DEBUG, "CL2TPTunnel::StartConnect\n"));
    
    ACE_TCHAR localaddr_str[BUFSIZ]={0};
    m_remote_addr.addr_to_string (localaddr_str, sizeof localaddr_str);
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPTunnel::StartConnect, peeraddr=%s\n", localaddr_str));
    
    m_handler = ACE_OS::socket (AF_INET, SOCK_DGRAM, 0);
    if (m_handler == ACE_INVALID_HANDLE)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CL2TPTunnel::StartConnect, handle error\n"));
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
          ACE_DEBUG ((LM_ERROR,"(%P|%t) CL2TPTunnel::StartConnect, setsockopt SO_REUSEADDR error\n"));
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
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CL2TPTunnel::StartConnect, setsockopt SO_RCVBUF error. size = %d\n", size));
        return -1;
    }

    if (ACE_OS::setsockopt(m_handler, SOL_SOCKET,
                                    SO_SNDBUF,
                                    (char *) &size,
                                    sizeof (size)) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CL2TPTunnel::StartConnect, setsockopt SO_SNDBUF error. size = %d\n", size));
        return -1;
    }

    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPTunnel::StartConnect, register_handler\n"));

    // Register with the reactor for input.
    if (ACE_Reactor::instance()->register_handler (this,ACE_Event_Handler::READ_MASK) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_ERROR_RETURN ((LM_ERROR,
                 ACE_TEXT ("(%P|%t) CL2TPTunnel::StartConnect: %p\n"),
                 ACE_TEXT ("register_handler for input")),
                -1);
    }

    return 0;
}

//UnInit Socket
int CL2TPTunnel::UnInitSocket()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPTunnel::StopConnect\n"));

    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CL2TPTunnel::StopConnect \n")));

    if (ACE_Reactor::instance())
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CL2TPTunnel::StopConnect remove_handler\n")));
        ACE_Reactor::instance()->remove_handler (this,
                                                ACE_Event_Handler::ALL_EVENTS_MASK |
                                                ACE_Event_Handler::DONT_CALL);
    }
    
    if (m_handler != ACE_INVALID_HANDLE)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CL2TPTunnel::StopConnect close socket \n"))); 
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
    }

    return 0;
}

//input handle
int CL2TPTunnel::handle_input (ACE_HANDLE fd)
{
    ACE_INET_Addr addrRecv;
    //ACE_INET_Addr localAddr;
    CL2TPControllMessage controllmsg;
    CL2TPDataMessage datamessage;
    static char buffer[MAX_RECV_SIZE];
    //GetLocalAddr(localAddr);
    int addr_len = addrRecv.get_size ();
    ssize_t rval_recv = ACE_OS::recvfrom(fd, buffer, sizeof(buffer),0, (sockaddr *)addrRecv.get_addr(),(int*)&addr_len);
    addrRecv.set_size (addr_len);
    ACE_TCHAR remote_str[80]={0};
    addrRecv.addr_to_string (remote_str, 80);

    ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%P|%t) CL2TPLAC::handle_input - ")
              ACE_TEXT ("activity occurred on handle %d!,%s\n"),
              m_handler,
              remote_str));
    
    ACE_DEBUG ((LM_INFO,
            ACE_TEXT ("(%P|%t) CL2TPLAC::handle_input - ")
            ACE_TEXT ("message from %d bytes received.\n"),
            rval_recv));

    if ((rval_recv == -1) || (rval_recv == 0))
    {
          ACE_ERROR_RETURN ((LM_ERROR,
                             ACE_TEXT ("(%P|%t) CL2TPLAC::handle_input - ")
                             ACE_TEXT ("closing daemon (fd = %d)\n"),
                             this->get_handle ()),
                            0); 
    }
    //buf->len = rval_recv;
    //fix_hdr (buf->start);
    //extract (buf->start, &tunnel, &call);
    struct unaligned_u16 *ver = (struct unaligned_u16*)buffer;
    uint16_t version = ntohs(ver[0].s);
    if (CTBIT (version))
    {
        ACE_DEBUG ((LM_DEBUG, "CL2TPLAC::handle_input controll message\n"));
        /* We have a control packet */
        if (controllmsg.Decode(buffer,rval_recv,addrRecv) == 0)
        {
            //int type=get_message_type(buf);
            //int result = 0;
            HandleControllMessage(controllmsg);
        }
        else
        {
            ACE_DEBUG ((LM_ERROR, "CL2TPLAC::handle_input controll message decode error\n"));
        }
    }
    else
    {
        ACE_DEBUG ((LM_DEBUG, "CL2TPLAC::handle_input data message\n"));
        if (datamessage.Decode(buffer,rval_recv,addrRecv) == 0)
        {
            HandlePayLoadMessage(datamessage);
        }
        else
        {
            ACE_DEBUG ((LM_ERROR, "CL2TPLAC::handle_input data message decode error\n"));
        }
    }
    return 0;
}

//Check Control Message
int CL2TPTunnel::CheckControlMessage(const CL2TPControllMessage & msg,CL2TPTunnel *t, CL2TPSession *session)
{
    /*
     * Check if this is a valid control
     * or not.  Returns 0 on success
     */
#if 0
    if (t == NULL)
    {
        return -1;
    }

    uint16_t seqnum = msg.GetNS();
    uint16_t recseqnum = t->GetControlRecSeqNum();
    if (seqnum != t->GetControlRecSeqNum())
    {
            ACE_DEBUG ((LM_ERROR, 
                 "%s: Received out of order control packet on tunnel %d (got %d, expected %d)\n",
                 __FUNCTION__, t->GetPeerTID(), seqnum, t->GetControlRecSeqNum()));
        if (((seqnum < recseqnum) && 
            ((recseqnum - seqnum) < 32768)) ||
            ((seqnum > recseqnum) &&
            ((recseqnum - seqnum) > 32768)))
        {
            /*
               * Woopsies, they sent us a message we should have already received
               * so we should send them a ZLB so they know
               * for sure that we already have it.
             */
            ACE_DEBUG ((LM_ERROR,"%s: Sending an updated ZLB in reponse\n",
                 __FUNCTION__));

            ControlZLB(t,session);
        }
        else
        {
            ACE_DEBUG ((LM_ERROR,"%s: error \n",
                 __FUNCTION__));
            return -1;
        }
    }
    else
    {
        m_control_rec_seq_num=seqnum;
        t->InCreaseNR();
    }
#endif
    if (t == NULL)
    {
        return -1;
    }

    m_control_rec_seq_num = msg.GetNS();
    if (!msg.IsZLB())
    {
        m_control_rec_seq_num=m_control_rec_seq_num+1;
    }

    return 0;
}

//PayLoad Message Handle
int CL2TPTunnel::HandlePayLoadMessage(const CL2TPDataMessage & datamsg)
{
    ACE_DEBUG ((LM_DEBUG, "CL2TPLAC::HandlePayLoadMessage\n"));

    if (datamsg.GetTID() != m_ourtid)
    {
        return -1;
    }

    CL2TPSession *psession = FindSession(datamsg.GetCID());
    if (psession == NULL)
    {
        return -1;
    }

    return psession->HandlePayLoad(datamsg.GetPayload(), datamsg.GetPayloadSize());
}

//Controll Message Handle
int CL2TPTunnel::HandleControllMessage(const CL2TPControllMessage &controllmsg)
{
    //m_control_rec_seq_num++;
    int result = -1;
    ACE_DEBUG ((LM_DEBUG, "CL2TPLAC::HandleControllMessage, type=%d\n",controllmsg.GetMessageType()));
    controllmsg.Dump();

    if (controllmsg.IsZLB())
    {
        ACE_DEBUG ((LM_DEBUG, "CL2TPLAC::HandleControllMessage, ZLB message\n"));
        HandleZLB(controllmsg);
        return 0;
    }

    switch(controllmsg.GetMessageType())
    {
        case SCCRQ:
            result = HandleStartControlConnectionRequest(controllmsg);
            break;
        case SCCRP:
            result = HandleStartControlConnectionReply(controllmsg);
            break;
        case SCCCN:
            result = HandleStartControlConnectionConnected(controllmsg);
            break;
        case StopCCN:
            result = HandleStopControlConnectionNotification(controllmsg);
            break;
        case Hello:
            result = HandleHello(controllmsg);
            break;
        case OCRQ:
            result = HandleSessionControllMessage(controllmsg);
            break;
        case OCRP:
            result = HandleSessionControllMessage(controllmsg);
            break;
        case OCCN:
            result = HandleSessionControllMessage(controllmsg);
            break;
        case ICRQ:
            result = HandleSessionControllMessage(controllmsg);
            break;
        case ICRP:
            result = HandleSessionControllMessage(controllmsg);
            break;
        case ICCN:
            result = HandleSessionControllMessage(controllmsg);
            break;
        case CDN:
            result = HandleSessionControllMessage(controllmsg);
            break;
    
        default:
            break;
    };

    return result;
}


//Session Controll Message Handle
int CL2TPTunnel::HandleSessionControllMessage(const CL2TPControllMessage &controllmsg)
{
    if (CheckControlMessage(controllmsg,this) == -1)
    {
        return -1;
    }

    CL2TPSession *psession =FindSession(controllmsg.GetCID());
    if (psession == NULL)
    {
        ACE_DEBUG ((LM_ERROR, "CL2TPTunnel::HandleSessionControllMessage, canot find session=%d\n",controllmsg.GetCID()));

        return -1;
    }
    return psession->HandleControllMessage(controllmsg);
}

// ZLB Handle
int CL2TPTunnel::HandleZLB(const CL2TPControllMessage &msg)
{
    ACE_UNUSED_ARG(msg);

    m_helltimeoutcount = 0;


    return 0;
}

//Start Control Connection Request Handle
int CL2TPTunnel::HandleStartControlConnectionRequest(const CL2TPControllMessage &msg)
{
    ACE_UNUSED_ARG(msg);
    return 0;
#if 0
    uint16_t localtid = m_tunnelidbase++;
    ACE_INET_Addr peeraddr = msg.GetPeerAddr();
    CAWAutoPtr<CL2TPTunnel> tunnel(new CL2TPTunnel(*this,m_localaddr,peeraddr,localtid, msg.GetAssignedTID()));
    if (AddChannel(localtid, tunnel) == -1)
    {
        ACE_DEBUG ((LM_ERROR,ACE_TEXT ("(%P|%t) CL2TPLAC::HandleStartControlConnectionRequest add channel\n")));

        return -1;
    }

    if (CheckControlMessage(msg,tunnel.Get()) == -1)
    {
        return -1;
    }

    char responsebuffer[MAX_RECV_SIZE]={0};

    CL2TPControllMessage response(responsebuffer,sizeof(responsebuffer));
    //struct buffer *Responsebuf = new_outgoing();
    response.add_message_type_avp (SCCRP);
    response.add_protocol_avp (OUR_L2TP_VERSION);
    response.add_frame_caps_avp (m_ourfc);
    response.add_bearer_caps_avp (0);

    response.add_firmware_avp (FIRMWARE_REV);
    response.add_hostname_avp ("wfnex");
    response.add_vendor_avp ("wfnex.com");
    response.add_tunnelid_avp (localtid);
    response.add_avp_rws (8);
    size_t responsesize = response.add_control_hdr(msg.GetAssignedTID(),0,tunnel->GetIncreaseNS(), tunnel->GetControlRecSeqNum());
    int result = SendData(responsebuffer,responsesize, msg.GetPeerAddr());
    return result;
#endif
    return 0;
}

//Start Control Connection Reply Handle
int CL2TPTunnel::HandleStartControlConnectionReply(const CL2TPControllMessage &msg)
{
    ACE_Reactor::instance()->cancel_timer(this);
    m_requestcount = 0;

    if (msg.GetTID() != m_ourtid)
    {
        return -1;
    }
    if (CheckControlMessage(msg,this) == -1)
    {
        return -1;
    }

    m_peertid = msg.GetAssignedTID();
    
    char requestbuffer[MAX_RECV_SIZE]={0};

    CL2TPControllMessage request(requestbuffer,sizeof(requestbuffer));
    //struct buffer *Responsebuf = new_outgoing();
    request.add_message_type_avp (SCCCN);
    size_t requestsize = request.add_control_hdr(m_peertid,0,GetIncreaseNS(), GetControlRecSeqNum());
    int result = SendData(requestbuffer,requestsize);

    m_isconnected = true;

    m_smgr.OnTunnelResult(0,this);

    return result;
}

//Start Control Connection Connected Handle
int CL2TPTunnel::HandleStartControlConnectionConnected(const CL2TPControllMessage &msg)
{
    ACE_UNUSED_ARG(msg);
    return 0;
#if 0
    CL2TPTunnel *tunnel = FindChannel(msg.GetTID());
    if (tunnel == NULL)
    {
        return -1;
    }
    if (CheckControlMessage(msg,tunnel) == -1)
    {
        return -1;
    }

    tunnel->ControlConnected();
    
    return ControlZLB(tunnel);
#endif
}

//Stop Control Connection Notification Handle
int CL2TPTunnel::HandleStopControlConnectionNotification(const CL2TPControllMessage &msg)
{
    if (msg.GetTID() != m_ourtid)
    {
        ACE_DEBUG ((LM_ERROR, "CL2TPTunnel::HandleStopControlConnectionNotification not outid\n"));
        return -1;
    }

    if (GetPeerTID() != msg.GetAssignedTID())
    {
        ACE_DEBUG ((LM_ERROR, "CL2TPTunnel::HandleStopControlConnectionNotification not peerid\n"));

        return -1;
    }

    if (CheckControlMessage(msg,this) == -1)
    {
        ACE_DEBUG ((LM_ERROR, "CL2TPTunnel::HandleStopControlConnectionNotification checkmessage error\n"));
        return -1;
    }

    
    int result = ControlZLB(this);

    m_smgr.RemoveChannel(m_remote_addr);

    return result;

}

//Hello Handle
int CL2TPTunnel::HandleHello(const CL2TPControllMessage &msg)
{
    if (msg.GetTID() != m_ourtid)
    {
        return -1;
    }

    if (CheckControlMessage(msg,this) == -1)
    {
        return -1;
    }

    //tunnel->PeerHello();

    return ControlZLB(this);
}

//WAN Error Notify Handle
int CL2TPTunnel::HandleWANErrorNotify(const CL2TPControllMessage &msg)
{
    ACE_UNUSED_ARG(msg);

    //dont handle
    return 0;
}

//Set Link Info Handle
int CL2TPTunnel::HandleSetLinkInfo(const CL2TPControllMessage &msg)
{
    ACE_UNUSED_ARG(msg);
    //dont handle
    return 0;
}

// Control ZLB
int CL2TPTunnel::ControlZLB(CL2TPTunnel *tunnel, CL2TPSession *session)
{
    uint16_t tid = 0;
    uint16_t cid = 0;
    uint16_t ns = 0;
    uint16_t nr = 0;

    if (tunnel == NULL)
    {
        return -1;
    }
    
    tid = tunnel->GetPeerTID();

    if (session)
    {
        cid = session->GetPeerCID();
    }

    ns = tunnel->GetControlSeqNum();
    nr = tunnel->GetControlRecSeqNum();
    
    char zlbbuffer[MAX_RECV_SIZE]={0};
    CL2TPControllMessage zlb(zlbbuffer,sizeof(zlbbuffer));
    size_t responsesize=zlb.add_control_hdr(tid,cid,ns, nr);
    return SendData(zlbbuffer,responsesize);   
}

//Start Connect Request
int CL2TPTunnel::StartConnectRequest()
{
    char buffer[MAX_RECV_SIZE]={0};

    CL2TPControllMessage request(buffer,sizeof(buffer));
    //struct buffer *Responsebuf = new_outgoing();
    request.add_message_type_avp (SCCRQ);
    request.add_protocol_avp (OUR_L2TP_VERSION);
    request.add_frame_caps_avp (m_ourfc);
    request.add_bearer_caps_avp (0);

    request.add_firmware_avp (FIRMWARE_REV);
    request.add_hostname_avp ("wfnex");
    request.add_vendor_avp ("wfnex.com");
    request.add_tunnelid_avp (m_ourtid);
    request.add_avp_rws (8);
    size_t responsesize = request.add_control_hdr(0,0,GetIncreaseNS(), GetControlRecSeqNum());
    m_requestcount++;
    return SendData(buffer,responsesize);    
}

//Session Call Back
void CL2TPTunnel::OnSessionCallBack(int result ,CL2TPSession *psession)
{
    if (m_pTunnelSink)
    {
        m_pTunnelSink->OnCallIndication(result,psession);
    }
}


