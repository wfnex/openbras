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
#include "CSession.h"
#include "CPPPOE.h"
#include <string>

CSession::CSession(CPPPOE &pppoe, WORD16 session)
    : m_pppoe(pppoe)
    , m_ppp(this)
    , m_sessionid(session)
    , m_requestedMtu(ETH_PPPOE_MTU)
    , m_sessionToBeDeleted(false)
    , m_time2waitClientPADT(5)
{
    ACE_DEBUG((LM_DEBUG, "CSession::CSession(), session id = %#x\n", session));
    ::memset(m_clientEth, 0, sizeof m_clientEth);
}

CSession::~CSession()
{
    ACE_DEBUG((LM_DEBUG, "CSession::~CSession(), session id = %#x\n", m_sessionid));
	m_pppoe.FreeId(m_sessionid);
    
    CancelTimer();
}

int CSession::Init()
{
    ACE_DEBUG((LM_DEBUG, "CSession::Init(), session id=%#x\n", m_sessionid));
    
    m_ppp.Init();
    m_ppp.StartFms();
    
    return 0;
}

//Input Session Data to Packet
int CSession::ProcessSessData(const PPPoEPacket *packet, ssize_t size)    
{
    // already check whether packet is NULL
    
    BYTE myAddr[ETH_ALEN];
    ::memset(myAddr, 0, sizeof myAddr);
    GetPppoe().GetEtherIntf().GetIntfMac(myAddr);

    if (memcmp(packet->ethHdr.h_dest, myAddr, ETH_ALEN))
    {
        ACE_DEBUG((LM_ERROR, 
                   "CSession::ProcessSessData(), PPPoE session data with session id %#x, but not directed to us.\n"
                   "dest in the pkt: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x; "
                   "my mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
                   ntohs(packet->session),
                   packet->ethHdr.h_dest[0], packet->ethHdr.h_dest[1], packet->ethHdr.h_dest[2],
                   packet->ethHdr.h_dest[3], packet->ethHdr.h_dest[4], packet->ethHdr.h_dest[5],
                   myAddr[0], myAddr[1], myAddr[2], myAddr[3], myAddr[4], myAddr[5]));
        
        return -1;
    }

    if (memcmp(packet->ethHdr.h_source, m_clientEth, ETH_ALEN))
    {
        ACE_DEBUG((LM_ERROR, 
                   "CSession::ProcessSessData(), PPPoE session data with session id %#x, but not from the original client.\n" 
                   "source in the pkt: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x; "
                   "client's mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
                   ntohs(packet->session),
                   packet->ethHdr.h_source[0], packet->ethHdr.h_source[1], packet->ethHdr.h_source[2],
                   packet->ethHdr.h_source[3], packet->ethHdr.h_source[4], packet->ethHdr.h_source[5],
                   m_clientEth[0], m_clientEth[1], m_clientEth[2], m_clientEth[3], m_clientEth[4], m_clientEth[5]));

        return -1;
    }

    if (m_sessionToBeDeleted)
    {
        if (ntohs(packet->length) < sizeof(WORD16))
        {
            ACE_DEBUG ((LM_ERROR, "CSession::ProcessSessData, pkt too short. length=%u\n", ntohs(packet->length)));
            return -1;
        }
        
        WORD16 protocol = *((WORD16 *)packet->payload);
        if (PPP_LCP == protocol || PPP_IPCP == protocol || PPP_PAP == protocol || PPP_CHAP == protocol)
        {
            ACE_DEBUG ((LM_INFO, 
                        "CSession::ProcessSessData, this session is to be deleted, so don't process pkt(%#x).\n", 
                        protocol));
            return 0;
        }
    }

    static BYTE buffer[4*1024];
    buffer[0] = 0xff;
    buffer[1] = 0x03;
    if (ntohs(packet->length) > sizeof(buffer) - 2)
    {
        ACE_DEBUG((LM_DEBUG, "CSession::ProcessSessData(), pkt too long(%u)\n", ntohs(packet->length)));
        return 0;
    }
    ::memcpy(buffer + 2, packet->payload, ntohs(packet->length));
    
    GetPPP().Input(buffer, ntohs(packet->length) + 2);

    return 0;
}

//LCP is Down
void CSession::OnLCPDown(const std::string reason)
{
    ACE_DEBUG ((LM_DEBUG, "CSession::OnLCPDown, reason=%s\n", reason.c_str()));
    m_pppoe.OnLCPDown(m_sessionid, reason);
}

// packet为PPP报文(带FF 03的),在其前面封装ethernet、pppoe报头后,sendPakcet
void CSession::OnPPPOutput(unsigned char *packet, size_t size)
{
    ACE_DEBUG((LM_DEBUG, "CSession::OnPPPOutput(), sessionId=%u, size=%u\n", m_sessionid, size));

    if (NULL == packet)
    {
        ACE_DEBUG ((LM_ERROR, "packet is NULL.\n"));
        return;
    }

#ifdef DEBUG_PKT
    static CHAR dumpStr[2048];
    ::memset(dumpStr, 0, sizeof(dumpStr));
    
    for (size_t i = 0; i < size; ++i)
    {
        snprintf(dumpStr + strlen(dumpStr), 4, "%.2x ", packet[i]);
    }
    ACE_DEBUG ((LM_DEBUG, "dump packet:\n%s\n", dumpStr));
#endif

    if (size < PPP_HDRLEN)
    {
        ACE_DEBUG ((LM_ERROR, "size too short.\n"));
        return;
    }
    
    if (m_sessionToBeDeleted)
    {
        WORD16 protocol = *((WORD16 *)(packet + PPP_ADDR_CTRL_LEN));
        protocol = ntohs(protocol);
        if (PPP_LCP == protocol || PPP_IPCP == protocol || PPP_PAP == protocol || PPP_CHAP == protocol)
        {
            ACE_DEBUG ((LM_INFO, "This session is to be deleted, so don't send pkt(%#x).\n", protocol));
            return;
        }
    }
    
    PPPoEPacket pppPkt;
    size_t sizeWithoutAC = size - PPP_ADDR_CTRL_LEN;
    
    memcpy(pppPkt.ethHdr.h_dest, m_clientEth, ETH_ALEN);
    GetPppoe().GetEtherIntf().GetIntfMac(pppPkt.ethHdr.h_source);
    pppPkt.ethHdr.h_proto = htons(ETH_PPPOE_SESSION);
    pppPkt.ver = 1;
    pppPkt.type = 1;
    pppPkt.code = CODE_SESS;
    pppPkt.session = htons(m_sessionid);
    pppPkt.length = htons(sizeWithoutAC);

    if (sizeWithoutAC > ETH_JUMBO_LEN)
    {
        ACE_DEBUG((LM_ERROR, "CSession::OnPPPOutput(), ppp packet too long(%u)\n", sizeWithoutAC));
        return;
    }

    memcpy(pppPkt.payload, packet + PPP_ADDR_CTRL_LEN, sizeWithoutAC);
    
    // SendPacket
    GetPppoe().GetPppoeSessionHndl().SendPacket((const char *)&pppPkt, sizeWithoutAC + HDR_SIZE);
}

void CSession::OnPPPPayload(unsigned char *packet, size_t size)
{
    // log
    return;
}

//Auth Request
void CSession::OnAuthRequest(Auth_Request &authReq)
{
    ACE_DEBUG ((LM_DEBUG, 
                "CSession::OnAuthRequest, sessionId=%#x, username=%s, passwd=%s\n",
                m_sessionid, authReq.username, authReq.userpasswd));
    
    authReq.session = m_sessionid;
    m_pppoe.OnPPPOEAuthRequest(authReq);
}

//PPPOE Auth Response                                                                   
void CSession::OnPPPOEAuthResponse(WORD32 result, std::string reason)
{
    ACE_DEBUG((LM_DEBUG, "CSession::OnPPPOEAuthResponse, result=%u, reason=%s\n", result, reason.c_str()));
    m_ppp.OnAuthResponse(result, reason);
}

//Add Subscriber
SWORD32 CSession::OnAddSubscriber(Session_User_Ex &sInfo)
{
    ACE_DEBUG ((LM_DEBUG, "CSession::OnAddSubscriber, session=%#X\n", m_sessionid));
    sInfo.user_ip = m_pppoe.GetIp();
    sInfo.session = m_sessionid;
    ::memcpy(sInfo.mac, m_clientEth, ETH_ALEN);
    sInfo.user_type = USER_TYPE_PPPOE;
    
    return m_pppoe.AddSubscriber(sInfo);
}

//Delete Subscriber
SWORD32 CSession::OnDelSubscriber()
{
    ACE_DEBUG ((LM_DEBUG, "CSession::OnDelSubscriber, session=%#X\n", m_sessionid));
    
    Session_Offline sInfo;
    ::memset(&sInfo, 0, sizeof sInfo);
    sInfo.user_type = USER_TYPE_PPPOE;
    sInfo.session = m_sessionid;
    ::memcpy(sInfo.mac, m_clientEth, ETH_ALEN);
    
    return m_pppoe.DelSubscriber(sInfo);
}

//Timeout Handle
int CSession::handle_timeout (const ACE_Time_Value &current_time, const void *act)
{
    ACE_DEBUG ((LM_DEBUG, "CSession::handle_timeout\n"));
    
    // Remove session
    ACE_DEBUG((LM_DEBUG, 
               "CSession::handle_timeout(), WFNEX-BRAS: remove session(%#x)\n",
               m_sessionid));
    
    if (-1 == GetPppoe().RemoveSession(m_sessionid))
    {
        ACE_DEBUG((LM_ERROR, "Failed to remove session(%#x)\n", m_sessionid));
        return -1;
    }

    return 0;
}

//Cancel Timer
void CSession::CancelTimer()
{
    ACE_DEBUG((LM_DEBUG, "CSession::CancelTimer\n"));
    ACE_Reactor::instance()->cancel_timer(this);
}

//Start One Off Timer
void CSession::StartOneOffTimer()
{
    ACE_DEBUG((LM_DEBUG, 
               "CSession::StartOneOffTimer, m_time2waitClientPADT=%d, m_sessionid=%u\n", 
               m_time2waitClientPADT,
               m_sessionid));

    CancelTimer();

    ACE_Time_Value delay(m_time2waitClientPADT);
    ACE_Reactor::instance()->schedule_timer(this, 0, delay); 
}


