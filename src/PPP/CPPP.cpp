/*
 * Copyright (c) 2014  Nanjing WFNEX Technology Co., Ltd. All rights reserved.
 */

#include "CPPP.h"
#include <string>
#include <string.h>

CPPP::CPPP(IPPPSink *psink)
    :m_lcp(this)
    ,m_pap(this)
    ,m_chap(this)
    ,m_ipcp(this)
    ,m_pSink(psink)
    ,m_bLinkOk(false)
    ,m_phase(PHASE_DEAD)
    ,m_hostName(BRAS_DEFAULT_HOSTNAME)
{
    ACE_DEBUG ((LM_DEBUG,"CPPP::CPPP\n"));
}

CPPP::~CPPP()
{
    ACE_DEBUG ((LM_DEBUG,"CPPP::~CPPP\n"));
}

void CPPP::Init()
{
    ACE_DEBUG ((LM_DEBUG,"CPPP::Init\n"));

    m_lcp.Init();
    
    switch (GetAuthType())
    {
        case PPP_PAP:
        {
            m_pap.Init();
            break;
        }
        case PPP_CHAP:
        {
            m_chap.Init();
            break;
        }
        default:
        {
            ACE_DEBUG((LM_ERROR, "Unsupported authentication type(%)"));
            break;
        }
    }
    
    m_ipcp.Init();
}

void CPPP::OnAuthResponse(WORD32 result, std::string &reason)
{
    ACE_DEBUG ((LM_DEBUG,
                "CPPP::OnAuthResponse, result=%u, reason=%.*s\n",
                result,
                reason.size(),
                reason.c_str()));

    switch (GetAuthType())
    {
        case PPP_CHAP:
        {
            m_chap.ResponseAuthenResult(result, reason);
            break;
        }
        case PPP_PAP:
        {
            m_pap.ResponseAuthenResult(result, reason);
            break;
        }
        default:
        {
            ACE_DEBUG ((LM_ERROR, "Unsupported authentication type(%d)\n", GetAuthType()));
            return;
        }
    }
}

void CPPP::StartFms()
{
    ACE_DEBUG ((LM_DEBUG,"CPPP::StartFms\n"));

    m_lcp.Open();
    //m_lcp.LowerUp();
}

void CPPP::NewPhase(PPPPhase phase)
{
    m_phase = phase;
}

// 对packet指向的报文格式有要求: 以ff 03打头
void CPPP::Input(unsigned char *packet, size_t len)
{
    u_char *p = packet;
    u_short protocol;

    ACE_DEBUG ((LM_DEBUG,"CPPP::Input, size=%d\n", len));

    if (len < PPP_HDRLEN)
    {
        ACE_DEBUG ((LM_ERROR,"CPPP::Input, size=%d to small\n", len));
        return;
    }

    if ((p[0] == 0xff) && (p[1] == 0x03))
    {
        p += 2; /* Skip address and control */
        GETSHORT(protocol, p);
        len -= PPP_HDRLEN;
    }
    else
    {
        // Commented by mazhh: this branch cannot be entered.
        GETSHORT(protocol, p);
        len -= 2;
    }
    /*
     * Toss all non-LCP packets unless LCP is OPEN.
     */
    if (protocol != PPP_LCP && m_lcp.State() != OPENED)
    {
        ACE_DEBUG ((LM_INFO, ACE_TEXT ("CPPP::Input, Discarded non-LCP packet when LCP not open\n")));
        return;
    }

    /*
     * Until we get past the authentication phase, toss all packets
     * except LCP, LQR and authentication packets.
     */
    if (m_phase <= PHASE_AUTHENTICATE && !(protocol == PPP_LCP 
        || protocol == PPP_LQR
        || protocol == PPP_PAP 
        || protocol == PPP_CHAP 
        ||protocol == PPP_EAP)) 
   {
        ACE_DEBUG ((LM_INFO, "CPPP::Input, discarding proto 0x%x in phase %d\n", protocol, m_phase));
        return;
    }
    
    ACE_DEBUG ((LM_DEBUG, "CPPP::Input, protocol 0x%x received\n", protocol));

    /*
     * Upcall the proper protocol input routine.
     */
    switch (protocol)
    {
        case PPP_LCP:
            m_lcp.Input(p, len);
            return;
        case PPP_IPCP:
            m_ipcp.Input(p, len);
            return;
        case PPP_PAP:
            m_pap.Input(p, len);
            return;
        case PPP_CHAP:
            m_chap.Input(p, len);
            return;
        case PPP_IP:
            m_ipcp.InputData(p, len);
            return;
        default:
            break;
    }

    ACE_DEBUG ((LM_INFO, "Unsupported protocol 0x%x received\n", protocol));

    m_lcp.SendProtRej(p - PPP_HDRLEN, len + PPP_HDRLEN);   
}

// Ported from link_established(), ppp-2.4.7
void CPPP::OnLCPUp()
{
    ACE_DEBUG((LM_DEBUG, "CPPP::OnLCPUp, LCP Opened.\n"));

    lcp_options &wo = m_lcp.GetWantOpt();
    lcp_options &go = m_lcp.GetGotOpt();
    lcp_options &ho = m_lcp.GetHisOpt();

    switch (GetAuthType())
    {
        case PPP_CHAP:
        {
            m_chap.LowerUp();
            break;
        }
        case PPP_PAP:
        {
            m_pap.LowerUp();
            break;
        }
        default:
        {
            ACE_DEBUG ((LM_ERROR, "Unsupported auth type(%u)\n", GetAuthType()));
            break;
        }
    }

    if (!(go.neg_upap || go.neg_chap))// || go->neg_eap))
    {
	    ACE_DEBUG ((LM_ERROR, "peer refused to authenticate: terminating link\n"));
	    m_lcp.Close("peer refused to authenticate");
	    return;
	}

    ACE_DEBUG ((LM_INFO, "advance to AUTHENTICATION phase\n"));
    NewPhase(PHASE_AUTHENTICATE);

    if (go.neg_chap)
    {
        m_chap.chap_auth_peer(m_hostName, CHAP_DIGEST(go.chap_mdtype));
    }
}

void CPPP::OnLCPDown()
{
    ACE_DEBUG((LM_DEBUG, "CPPP::OnLCPDown, close all NCPs, advance to ESTABLISH phase.\n"));
    
    // Close all upper layer protocols.
    m_ipcp.LowerDown();
    m_ipcp.Close("LCP down");

	if (m_phase != PHASE_DEAD)
	{
	    NewPhase(PHASE_ESTABLISH);    
	}

    if (m_pSink)
    {
        std::string reason("LCP Down");
        m_pSink->OnLCPDown(reason);
    }
}

// Note: 本函数移植自ppp-2.4.7的link_terminated().
/*
 * LCP has terminated the link; go to the Dead phase and take the
 * physical layer down.
 */
void CPPP::OnLCPTerminate()
{
    ACE_DEBUG ((LM_DEBUG,"CPPP::OnLCPTerminate\n"));

    if (m_phase == PHASE_DEAD)
    {
    	return;    
    }

    m_lcp.LowerDown();

    NewPhase(PHASE_DEAD);

    if (m_pSink)
    {
        std::string reason("LCP Terminate");
        m_pSink->OnLCPDown(reason);
    }    
}

void CPPP::OnIPCPUp()
{
    ACE_DEBUG((LM_DEBUG, "CPPP::OnIPCPUp\n"));

    Session_User_Ex sInfo;
    ::memset(&sInfo, 0, sizeof sInfo);

    std::string username;

    switch (GetAuthType())
    {
        case PPP_PAP:
        {
            username = m_pap.GetUsername();
            sInfo.auth_type = PPP_PAP;
            
            break;
        }
        case PPP_CHAP:
        {
            username = m_chap.GetUserName();
            sInfo.auth_type = PPP_CHAP;
            
            break;
        }
        default:
        {
            ACE_DEBUG ((LM_ERROR, "Unsupported auth type(%u).\n", GetAuthType()));
            return;
        }
    }
    
    size_t len = (username.size() < SESSION_MGR_MAX_USERNAME_SIZE - 1 ? username.size() : SESSION_MGR_MAX_USERNAME_SIZE - 1);
	::strncpy(sInfo.user_name, username.c_str(), len);
    
    sInfo.user_ip = ntohl(m_ipcp.GetSubscriberIP());
    sInfo.primary_dns = ntohl(m_ipcp.GetPrimaryDNS());
    sInfo.secondary_dns = ntohl(m_ipcp.GetSecondaryDNS());

    SWORD32 result = m_pSink->OnAddSubscriber(sInfo);
    if (-1 == result)
    {
        ACE_DEBUG ((LM_ERROR, "Add subscriber failed.\n"));
    }
}

void CPPP::OnIPCPDown()
{
    ACE_DEBUG((LM_DEBUG, "CPPP::OnIPCPDown\n"));

    if (NULL == m_pSink)
    {
        ACE_DEBUG ((LM_ERROR, "m_pSink NULL\n"));
        return;
    }
    
    SWORD32 result = m_pSink->OnDelSubscriber();
    if (-1 == result)
    {
        ACE_DEBUG ((LM_ERROR, "Delete subscriber failed.\n"));
    }

    // IPCP Down，触发PPPOE向对端发PADT，然后销毁整块CSession内存(这样就不需要shutdown LCP了)。
    std::string reason("IPCP Down");
    m_pSink->OnLCPDown(reason);
}

void CPPP::OnIPCPPayload(unsigned char *packet, size_t size)
{
    if (m_pSink)
    {
        m_pSink->OnPPPPayload(packet,size);
    }
}

void CPPP::InputPayload(unsigned char *packet, size_t size)
{
    if (m_bLinkOk)
    {
        m_ipcp.SendPayload(packet, size);
    }
}

void CPPP::OnLCPOutput(unsigned char *packet, size_t size)
{
    ACE_DEBUG ((LM_DEBUG,"CPPP::OnLCPOutput, size=%u\n", size));

    if (m_pSink)
    {
        m_pSink->OnPPPOutput(packet,size);
    }
}

void CPPP::OnIPCPOutput(unsigned char *packet, size_t size)
{
    ACE_DEBUG ((LM_DEBUG,"CPPP::OnIPCPOutput, size=%u\n", size));
    
    if (m_pSink)
    {
       m_pSink->OnPPPOutput(packet,size);
    }
}

// For interface IAuthenSvrSink
void CPPP::SendAuthRequest2AM(Auth_Request &authReq)
{
    ACE_DEBUG ((LM_INFO, "CPPP::SendAuthRequest2AM, username=%s, passwd=%s\n", authReq.username, authReq.userpasswd));

    if (m_pSink)
    {
        m_pSink->OnAuthRequest(authReq);
    }
}

void CPPP::OnAuthenOutput(unsigned char *packet, size_t size)
{
    ACE_DEBUG ((LM_DEBUG, "CPPP::OnAuthenOutput size=%d\n", size));

    if (m_pSink)
    {
        m_pSink->OnPPPOutput(packet,size);
    }
}

void CPPP::OnAuthenResult(int result, int protocol)
{
    ACE_DEBUG ((LM_DEBUG,"CPPP::OnAuthenResult, result=%d, protocol=%#x\n", result, protocol));

    if (result == 1)
    {
        NewPhase(PHASE_NETWORK);
        m_ipcp.Open();
        //m_ipcp.LowerUp();
    }
    else
    {
        NewPhase(PHASE_TERMINATE);
        std::string reason;
        
        switch (protocol)
        {
            case PPP_PAP:
            {
                reason += "Pap ";
                break;
            }
            case PPP_CHAP:
            {
                reason += "Chap ";
                break;
            }
            default:
            {
                reason += "Unkown Protocol ";
                break;
            }
        }
        reason += "authentication failed";
        m_lcp.Close((char *)reason.c_str());
    }
}


