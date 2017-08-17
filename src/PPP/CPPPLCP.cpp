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


#include "CPPPLCP.h"
#include "random.h"
#include "CPPP.h"

CPPPLCP::CPPPLCP(IPPPLCPSink *psink)
    :m_psink(psink)
    ,m_fsm(this,PPP_LCP,"LCP")
    ,m_lcp_echos_pending(0)
    ,m_lcp_echo_number(0)
    ,m_lcp_echo_timer_running(0)
    ,m_magicnumber(0)
    ,m_lcp_echo_fails(5)
    ,m_lcp_echo_interval(20)  // 参考华为ME60
    ,m_peer_mru(PPP_MRU)
    ,m_authType(BRAS_DEFAULT_AUTHTYPE)
{
    ::memset(&m_lcp_wantoptions, 0, sizeof(m_lcp_wantoptions));
    ::memset(&m_lcp_gotoptions, 0, sizeof(m_lcp_gotoptions));
    ::memset(&m_lcp_allowoptions, 0, sizeof(m_lcp_allowoptions));
    ::memset(&m_lcp_hisoptions, 0, sizeof(m_lcp_hisoptions));
}

CPPPLCP::~CPPPLCP()
{
    ACE_DEBUG ((LM_DEBUG, "CPPPLCP::~CPPPLCP\n"));
    CancelEchoTimer();
}

void CPPPLCP::Init()
{
    lcp_options *wo = &m_lcp_wantoptions;
    lcp_options *ao = &m_lcp_allowoptions;

    ACE_DEBUG ((LM_DEBUG, "CPPPLCP::Init\n"));
    
    m_fsm.Init();

    BZERO(wo, sizeof(*wo));
    wo->neg_mru = 1;
    //wo->mru = DEFMRU;
    wo->mru = 1492;
    wo->neg_asyncmap = 0;
    wo->neg_magicnumber = 1;
    wo->neg_pcompression = 0;
    wo->neg_accompression = 0;
    
    switch (m_authType)
    {
        case PPP_PAP:
        {
            wo->neg_upap = 1;
            break;
        }
        case PPP_CHAP:
        {
            wo->neg_chap = 1;
            wo->chap_mdtype = MDTYPE_ALL;
            break;
        }
        default:
        {
            ACE_DEBUG ((LM_ERROR, "wo Unsupported auth type(%d)\n", m_authType));
            break;
        }
    }

    BZERO(ao, sizeof(*ao));
    ao->neg_mru = 1;
    //ao->mru = MAXMRU;
    ao->mru = 1492;
    ao->neg_asyncmap = 1;
    
    switch (m_authType)
    {
        case PPP_CHAP:
        {
            ao->neg_chap = 1;
            ao->chap_mdtype = MDTYPE_ALL;
            break;
        }
        case PPP_PAP:
        {
            ao->neg_upap = 1;
            break;
        }
        default:
        {
            ACE_DEBUG ((LM_ERROR, "ao Unsupported auth type(%u)\n", m_authType));
            break;
        }
    }
    
    ao->neg_eap = 0;
    ao->neg_magicnumber = 1;
    ao->neg_pcompression = 0;
    ao->neg_accompression = 0;
}

//Input packet
void CPPPLCP::Input(unsigned char *packet ,size_t size)
{
    m_fsm.Input(packet, size);
}

/*
 * lcp_protrej - A Protocol-Reject was received.
 */
void CPPPLCP::Protrej()
{
    /*
     * Can't reject LCP!
     */
    ACE_DEBUG((LM_INFO, "CPPPLCP::Protrej, Received Protocol-Reject for LCP!\n"));
    m_fsm.ProtReject();
}

//The lower layer is up
void CPPPLCP::LowerUp()
{
    // 在多个客户同时连接的情况下，此函数其实是没有办法区分出session的，所以此trace可能无效。
    ACE_DEBUG((LM_INFO, "CPPPLCP::LowerUp\n"));
    
    m_fsm.LowerUp();
}

//The lower layer is down
void CPPPLCP::LowerDown()
{
    // 在多个客户同时连接的情况下，此函数其实是没有办法区分出session的，所以此trace可能无效。
    ACE_DEBUG((LM_INFO, "CPPPLCP::LowerDown\n"));

    m_fsm.LowerDown();
}

//open fsm
void CPPPLCP::Open()
{
    // 在多个客户同时连接的情况下，此函数其实是没有办法区分出session的，所以此trace可能无效。
    ACE_DEBUG((LM_INFO, "CPPPLCP::Open\n"));

    lcp_options *wo = &m_lcp_wantoptions;

    m_fsm.DisableFlag(OPT_PASSIVE | OPT_SILENT);
    if (wo->passive)
        m_fsm.EnableFlag(OPT_PASSIVE);
    if (wo->silent)
        m_fsm.EnableFlag(OPT_SILENT);

    m_fsm.Open();
}

//Close fsm
void CPPPLCP::Close(char *reason)
{
    // 在多个客户同时连接的情况下，此函数其实是没有办法区分出session的，所以此trace可能无效。
    ACE_DEBUG((LM_INFO, "CPPPLCP::Close, reason=%s\n", reason));
    
    m_fsm.Close(reason);
}

//Timeout Handle
int CPPPLCP::handle_timeout (const ACE_Time_Value &current_time, const void *act)
{
    EchoTimeout();
    
    return 0;
}

// echo Timeout
void CPPPLCP::EchoTimeout()
{
    SendEchoRequest();
}

//receive echo reply
void CPPPLCP::RcvEchoReply(int id, u_char *inp, int len)
{
    uint32_t magic = 0;

    /* Check the magic number - don't count replies from ourselves. */
    if (len < 4) 
    {
        ACE_DEBUG((LM_ERROR, "CPPPLCP::RcvEchoReply, invalid length(%d)\n", len));
        return;
    }
    
    GETLONG(magic, inp);
    
    if (magic == m_magicnumber) 
    {
        ACE_DEBUG((LM_ERROR, "CPPPLCP::RcvEchoReply, receive our echo request. link loop? \n"));
        return;
    }

    /* Reset the number of outstanding echo frames */
    m_lcp_echos_pending = 0;
}


/*
 * Time to shut down the link because there is nothing out there.
 */
void CPPPLCP::LcpLinkFailure()
{
    ACE_DEBUG((LM_DEBUG, "CPPPLCP::LcpLinkFailure()\n"));
    
    if (m_fsm.State() == OPENED) 
    {
        Close("Peer not responding");
    }
}

//Send Echo Request
void CPPPLCP::SendEchoRequest()
{
    uint32_t lcp_magic;
    unsigned char pkt[4]={0}, *pktp;
    
    if (m_lcp_echo_fails != 0) 
    {
        if (m_lcp_echos_pending >= m_lcp_echo_fails) 
        {
            ACE_DEBUG((LM_DEBUG, 
                       "CPPPLCP::SendEchoRequest, peer unanswered our %d echo request, so lcp link fail.\n", 
                       m_lcp_echo_fails));
            
            LcpLinkFailure();
            m_lcp_echos_pending = 0;
        }
    }
    
    /*
     * Make and send the echo request frame.
     */
    if (m_fsm.State() == OPENED) 
    {
        lcp_magic = m_magicnumber;
        pktp = pkt;
        PUTLONG(lcp_magic, pktp);
        m_fsm.SendData(ECHOREQ, m_lcp_echo_number++ & 0xFF, pkt, pktp - pkt);
        ++m_lcp_echos_pending;
    }

}

/*
 * lcp_echo_lowerup - Start the timer for the LCP frame
 */
void CPPPLCP::EchoLowerUp ()
{
    /* Clear the parameters for generating echo frames */
    m_lcp_echos_pending      = 0;
    m_lcp_echo_number        = 0;
    m_lcp_echo_timer_running = 0;
  
    /* If a timeout interval is specified then start the timer */
    if (m_lcp_echo_interval != 0)
    {
        LcpEchoCheck ();
    }
}

/*
 * lcp_echo_lowerdown - Stop the timer for the LCP frame
 */
void CPPPLCP::EchoLowerDown ()
{
    if (m_lcp_echo_timer_running != 0) 
    {
        CancelEchoTimer();
        m_lcp_echo_timer_running = 0;
    }
}

//Cancel Echo Timer
void CPPPLCP::CancelEchoTimer()
{
    ACE_DEBUG((LM_DEBUG, "CPPPLCP::CancelEchoTimer\n"));
    
    ACE_Reactor::instance()->cancel_timer(this);
}

//Check Lcp Echo status
void
CPPPLCP::LcpEchoCheck ()
{
    SendEchoRequest ();
    if (m_fsm.State() != OPENED)
    {
        return;
    }

    /*
     * Start the timer for the next interval.
     */
    StartEchoTimer(m_lcp_echo_interval);
    m_lcp_echo_timer_running = 1;
}

//Start Echo Timer
void CPPPLCP::StartEchoTimer(int interval)
{
    ACE_DEBUG((LM_DEBUG, "CPPPLCP::StartEchoTimer, interval=%d\n", interval));

    CancelEchoTimer();

    ACE_Time_Value delay(interval);
    ACE_Reactor::instance()->schedule_timer(this, 0, delay, delay); 
}


//fsm
void CPPPLCP::Resetci(CPPPFsm *pfsm)
{
    lcp_options *wo = &m_lcp_wantoptions;
    lcp_options *go = &m_lcp_gotoptions;

    wo->magicnumber = magic();
    m_magicnumber = wo->magicnumber;  // Added by mazhh. 保存最近一次生成的magicNumber，用在echo request中。
    wo->numloops = 0;
    ::memcpy(go, wo, sizeof(lcp_options));
    go->neg_mrru = 0;
    go->neg_ssnhf = 0;
    go->neg_endpoint = 0;
    m_peer_mru = PPP_MRU;
}

int CPPPLCP::Cilen(CPPPFsm *pfsm)
{
    lcp_options *go = &m_lcp_gotoptions;

#define LENCIVOID(neg)	((neg) ? CILEN_VOID : 0)
#define LENCICHAP(neg)	((neg) ? CILEN_CHAP : 0)
#define LENCISHORT(neg)	((neg) ? CILEN_SHORT : 0)
#define LENCILONG(neg)	((neg) ? CILEN_LONG : 0)
#define LENCILQR(neg)	((neg) ? CILEN_LQR: 0)
#define LENCICBCP(neg)	((neg) ? CILEN_CBCP: 0)
    /*
     * NB: we only ask for one of CHAP, UPAP, or EAP, even if we will
     * accept more than one.  We prefer EAP first, then CHAP, then
     * PAP.
     */
    return (LENCISHORT(go->neg_mru && go->mru != DEFMRU) +
        LENCILONG(go->neg_asyncmap && go->asyncmap != 0xFFFFFFFF) +
        LENCISHORT(go->neg_eap) +
        LENCICHAP(!go->neg_eap && go->neg_chap) +
        LENCISHORT(!go->neg_eap && !go->neg_chap && go->neg_upap) +
        LENCILQR(go->neg_lqr) +
        LENCICBCP(go->neg_cbcp) +
        LENCILONG(go->neg_magicnumber) +
        LENCIVOID(go->neg_pcompression) +
        LENCIVOID(go->neg_accompression) +
        LENCISHORT(go->neg_mrru) +
        LENCIVOID(go->neg_ssnhf));

}

void CPPPLCP::Addci(CPPPFsm *pfsm, u_char *ucp, int *lenp)
{
    lcp_options *go = &m_lcp_gotoptions;
    u_char *start_ucp = ucp;

#define ADDCIVOID(opt, neg) \
    if (neg) { \
    PUTCHAR(opt, ucp); \
    PUTCHAR(CILEN_VOID, ucp); \
    }
#define ADDCISHORT(opt, neg, val) \
    if (neg) { \
    PUTCHAR(opt, ucp); \
    PUTCHAR(CILEN_SHORT, ucp); \
    PUTSHORT(val, ucp); \
    }
#define ADDCICHAP(opt, neg, val) \
    if (neg) { \
    PUTCHAR((opt), ucp); \
    PUTCHAR(CILEN_CHAP, ucp); \
    PUTSHORT(PPP_CHAP, ucp); \
    PUTCHAR((CHAP_DIGEST(val)), ucp); \
    }
#define ADDCILONG(opt, neg, val) \
    if (neg) { \
    PUTCHAR(opt, ucp); \
    PUTCHAR(CILEN_LONG, ucp); \
    PUTLONG(val, ucp); \
    }
#define ADDCILQR(opt, neg, val) \
    if (neg) { \
    PUTCHAR(opt, ucp); \
    PUTCHAR(CILEN_LQR, ucp); \
    PUTSHORT(PPP_LQR, ucp); \
    PUTLONG(val, ucp); \
    }
#define ADDCICHAR(opt, neg, val) \
    if (neg) { \
    PUTCHAR(opt, ucp); \
    PUTCHAR(CILEN_CHAR, ucp); \
    PUTCHAR(val, ucp); \
    }
#define ADDCIENDP(opt, neg, class, val, len) \
    if (neg) { \
    int i; \
    PUTCHAR(opt, ucp); \
    PUTCHAR(CILEN_CHAR + len, ucp); \
    PUTCHAR(class, ucp); \
    for (i = 0; i < len; ++i) \
        PUTCHAR(val[i], ucp); \
    }

    ADDCISHORT(CI_MRU, go->neg_mru && go->mru != DEFMRU, go->mru);
    ADDCILONG(CI_ASYNCMAP, go->neg_asyncmap && go->asyncmap != 0xFFFFFFFF,
          go->asyncmap);
    ADDCISHORT(CI_AUTHTYPE, go->neg_eap, PPP_EAP);
    ADDCICHAP(CI_AUTHTYPE, !go->neg_eap && go->neg_chap, go->chap_mdtype);
    ADDCISHORT(CI_AUTHTYPE, !go->neg_eap && !go->neg_chap && go->neg_upap,
           PPP_PAP);
    ADDCILQR(CI_QUALITY, go->neg_lqr, go->lqr_period);
    ADDCICHAR(CI_CALLBACK, go->neg_cbcp, CBCP_OPT);
    ADDCILONG(CI_MAGICNUMBER, go->neg_magicnumber, go->magicnumber);
    ADDCIVOID(CI_PCOMPRESSION, go->neg_pcompression);
    ADDCIVOID(CI_ACCOMPRESSION, go->neg_accompression);
    ADDCISHORT(CI_MRRU, go->neg_mrru, go->mrru);
    ADDCIVOID(CI_SSNHF, go->neg_ssnhf);

    if (ucp - start_ucp != *lenp) {
        /* this should never happen, because peer_mtu should be 1500 */
        //error("Bug in lcp_addci: wrong length");
    }

}

int CPPPLCP::Ackci(CPPPFsm *pfsm, u_char *p, int len)
{
    lcp_options *go = &m_lcp_gotoptions;
    u_char cilen, citype, cichar;
    u_short cishort;
    uint32_t cilong;

    /*
     * CIs must be in exactly the same order that we sent.
     * Check packet length and CI length at each step.
     * If we find any deviations, then this packet is bad.
     */
#define ACKCIVOID(opt, neg) \
    if (neg) { \
    if ((len -= CILEN_VOID) < 0) \
        goto bad; \
    GETCHAR(citype, p); \
    GETCHAR(cilen, p); \
    if (cilen != CILEN_VOID || \
        citype != opt) \
        goto bad; \
    }
#define ACKCISHORT(opt, neg, val) \
    if (neg) { \
    if ((len -= CILEN_SHORT) < 0) \
        goto bad; \
    GETCHAR(citype, p); \
    GETCHAR(cilen, p); \
    if (cilen != CILEN_SHORT || \
        citype != opt) \
        goto bad; \
    GETSHORT(cishort, p); \
    if (cishort != val) \
        goto bad; \
    }
#define ACKCICHAR(opt, neg, val) \
    if (neg) { \
    if ((len -= CILEN_CHAR) < 0) \
        goto bad; \
    GETCHAR(citype, p); \
    GETCHAR(cilen, p); \
    if (cilen != CILEN_CHAR || \
        citype != opt) \
        goto bad; \
    GETCHAR(cichar, p); \
    if (cichar != val) \
        goto bad; \
    }
#define ACKCICHAP(opt, neg, val) \
    if (neg) { \
    if ((len -= CILEN_CHAP) < 0) \
        goto bad; \
    GETCHAR(citype, p); \
    GETCHAR(cilen, p); \
    if (cilen != CILEN_CHAP || \
        citype != (opt)) \
        goto bad; \
    GETSHORT(cishort, p); \
    if (cishort != PPP_CHAP) \
        goto bad; \
    GETCHAR(cichar, p); \
    if (cichar != (CHAP_DIGEST(val))) \
      goto bad; \
    }
#define ACKCILONG(opt, neg, val) \
    if (neg) { \
    if ((len -= CILEN_LONG) < 0) \
        goto bad; \
    GETCHAR(citype, p); \
    GETCHAR(cilen, p); \
    if (cilen != CILEN_LONG || \
        citype != opt) \
        goto bad; \
    GETLONG(cilong, p); \
    if (cilong != val) \
        goto bad; \
    }
#define ACKCILQR(opt, neg, val) \
    if (neg) { \
    if ((len -= CILEN_LQR) < 0) \
        goto bad; \
    GETCHAR(citype, p); \
    GETCHAR(cilen, p); \
    if (cilen != CILEN_LQR || \
        citype != opt) \
        goto bad; \
    GETSHORT(cishort, p); \
    if (cishort != PPP_LQR) \
        goto bad; \
    GETLONG(cilong, p); \
    if (cilong != val) \
      goto bad; \
    }
#define ACKCIENDP(opt, neg, class, val, vlen) \
    if (neg) { \
    int i; \
    if ((len -= CILEN_CHAR + vlen) < 0) \
        goto bad; \
    GETCHAR(citype, p); \
    GETCHAR(cilen, p); \
    if (cilen != CILEN_CHAR + vlen || \
        citype != opt) \
        goto bad; \
    GETCHAR(cichar, p); \
    if (cichar != class) \
        goto bad; \
    for (i = 0; i < vlen; ++i) { \
        GETCHAR(cichar, p); \
        if (cichar != val[i]) \
        goto bad; \
    } \
    }

    ACKCISHORT(CI_MRU, go->neg_mru && go->mru != DEFMRU, go->mru);
    ACKCILONG(CI_ASYNCMAP, go->neg_asyncmap && go->asyncmap != 0xFFFFFFFF,
          go->asyncmap);
    ACKCISHORT(CI_AUTHTYPE, go->neg_eap, PPP_EAP);
    ACKCICHAP(CI_AUTHTYPE, !go->neg_eap && go->neg_chap, go->chap_mdtype);
    ACKCISHORT(CI_AUTHTYPE, !go->neg_eap && !go->neg_chap && go->neg_upap,
           PPP_PAP);
    ACKCILQR(CI_QUALITY, go->neg_lqr, go->lqr_period);
    ACKCICHAR(CI_CALLBACK, go->neg_cbcp, CBCP_OPT);
    ACKCILONG(CI_MAGICNUMBER, go->neg_magicnumber, go->magicnumber);
    ACKCIVOID(CI_PCOMPRESSION, go->neg_pcompression);
    ACKCIVOID(CI_ACCOMPRESSION, go->neg_accompression);
    ACKCISHORT(CI_MRRU, go->neg_mrru, go->mrru);
    ACKCIVOID(CI_SSNHF, go->neg_ssnhf);

    /*
     * If there are any remaining CIs, then this packet is bad.
     */
    if (len != 0)
    {
        goto bad;
    }
    
    return (1);
    
bad:
    ACE_DEBUG((LM_ERROR, "CPPPLCP::Ackci, received bad Ack!\n"));
    return (0);

}

int  CPPPLCP::Nakci(CPPPFsm *pfsm, u_char *p, int len, int treat_as_reject)
{
    lcp_options *go = &m_lcp_gotoptions;
    lcp_options *wo = &m_lcp_wantoptions;
    u_char citype, cichar, *next;
    u_short cishort;
    uint32_t cilong;
    lcp_options no;       /* options we've seen Naks for */
    lcp_options tryop;    /* options to request next time */
    int looped_back = 0;
    int cilen;

    BZERO(&no, sizeof(no));
    tryop = *go;
    
    /*
     * Any Nak'd CIs must be in exactly the same order that we sent.
     * Check packet length and CI length at each step.
     * If we find any deviations, then this packet is bad.
     */
#define NAKCIVOID(opt, neg) \
    if (go->neg && \
        len >= CILEN_VOID && \
        p[1] == CILEN_VOID && \
        p[0] == opt) { \
        len -= CILEN_VOID; \
        INCPTR(CILEN_VOID, p); \
        no.neg = 1; \
        tryop.neg = 0; \
    }
#define NAKCICHAP(opt, neg, code) \
    if (go->neg && \
        len >= CILEN_CHAP && \
        p[1] == CILEN_CHAP && \
        p[0] == opt) { \
        len -= CILEN_CHAP; \
        INCPTR(2, p); \
        GETSHORT(cishort, p); \
        GETCHAR(cichar, p); \
        no.neg = 1; \
        code \
    }
#define NAKCICHAR(opt, neg, code) \
    if (go->neg && \
        len >= CILEN_CHAR && \
        p[1] == CILEN_CHAR && \
        p[0] == opt) { \
        len -= CILEN_CHAR; \
        INCPTR(2, p); \
        GETCHAR(cichar, p); \
        no.neg = 1; \
        code \
    }
#define NAKCISHORT(opt, neg, code) \
    if (go->neg && \
        len >= CILEN_SHORT && \
        p[1] == CILEN_SHORT && \
        p[0] == opt) { \
        len -= CILEN_SHORT; \
        INCPTR(2, p); \
        GETSHORT(cishort, p); \
        no.neg = 1; \
        code \
    }
#define NAKCILONG(opt, neg, code) \
    if (go->neg && \
        len >= CILEN_LONG && \
        p[1] == CILEN_LONG && \
        p[0] == opt) { \
        len -= CILEN_LONG; \
        INCPTR(2, p); \
        GETLONG(cilong, p); \
        no.neg = 1; \
        code \
    }
#define NAKCILQR(opt, neg, code) \
    if (go->neg && \
        len >= CILEN_LQR && \
        p[1] == CILEN_LQR && \
        p[0] == opt) { \
        len -= CILEN_LQR; \
        INCPTR(2, p); \
        GETSHORT(cishort, p); \
        GETLONG(cilong, p); \
        no.neg = 1; \
        code \
    }
#define NAKCIENDP(opt, neg) \
    if (go->neg && \
        len >= CILEN_CHAR && \
        p[0] == opt && \
        p[1] >= CILEN_CHAR && \
        p[1] <= len) { \
        len -= p[1]; \
        INCPTR(p[1], p); \
        no.neg = 1; \
        tryop.neg = 0; \
    }
    
    /*
     * NOTE!  There must be no assignments to individual fields of *go in
     * the code below.  Any such assignment is a BUG!
     */
    /*
     * We don't care if they want to send us smaller packets than
     * we want.  Therefore, accept any MRU less than what we asked for,
     * but then ignore the new value when setting the MRU in the kernel.
     * If they send us a bigger MRU than what we asked, accept it, up to
     * the limit of the default MRU we'd get if we didn't negotiate.
     */
    if (go->neg_mru && go->mru != DEFMRU) {
        NAKCISHORT(CI_MRU, 
                   neg_mru,
                   if (cishort <= wo->mru || cishort <= DEFMRU)
                       tryop.mru = cishort;
                   );
    }

    /*
     * Add any characters they want to our (receive-side) asyncmap.
     */
    if (go->neg_asyncmap && go->asyncmap != 0xFFFFFFFF) {
        NAKCILONG(CI_ASYNCMAP, 
                  neg_asyncmap,
                  tryop.asyncmap = go->asyncmap | cilong;
                  );
    }

    /*
     * If they've nak'd our authentication-protocol, check whether
     * they are proposing a different protocol, or a different
     * hash algorithm for CHAP.
     */
    if ((go->neg_chap || go->neg_upap || go->neg_eap)
        && len >= CILEN_SHORT
        && p[0] == CI_AUTHTYPE && p[1] >= CILEN_SHORT && p[1] <= len) {
        cilen = p[1];
        len -= cilen;
        no.neg_chap = go->neg_chap;
        no.neg_upap = go->neg_upap;
        no.neg_eap = go->neg_eap;
        INCPTR(2, p);
        GETSHORT(cishort, p);
        if (cishort == PPP_PAP && cilen == CILEN_SHORT) {
            /* If we were asking for EAP, then we need to stop that. */
            if (go->neg_eap)
            {
                tryop.neg_eap = 0;
            }
            /* If we were asking for CHAP, then we need to stop that. */
            else if (go->neg_chap)
            {
                tryop.neg_chap = 0;
            }
            /*
             * If we weren't asking for CHAP or EAP, then we were asking for
             * PAP, in which case this Nak is bad.
             */
            else
            {
                goto bad;
            }
        } else if (cishort == PPP_CHAP && cilen == CILEN_CHAP) {
            GETCHAR(cichar, p);
            /* Stop asking for EAP, if we were. */
            if (go->neg_eap) {
                tryop.neg_eap = 0;
                /* Try to set up to use their suggestion, if possible */
                if (CHAP_CANDIGEST(go->chap_mdtype, cichar))
                {
                    tryop.chap_mdtype = CHAP_MDTYPE_D(cichar);
                }
            } else if (go->neg_chap) {
                /*
                 * We were asking for our preferred algorithm, they must
                 * want something different.
                 */
                if (cichar != CHAP_DIGEST(go->chap_mdtype)) {
                    if (CHAP_CANDIGEST(go->chap_mdtype, cichar)) {
                        /* Use their suggestion if we support it ... */
                        tryop.chap_mdtype = CHAP_MDTYPE_D(cichar);
                    } else {
                        /* ... otherwise, try our next-preferred algorithm. */
                        tryop.chap_mdtype &= ~(CHAP_MDTYPE(tryop.chap_mdtype));
                        if (tryop.chap_mdtype == MDTYPE_NONE) /* out of algos */
                        {
                            tryop.neg_chap = 0;
                        }
                    }
                } else {
                    /*
                     * Whoops, they Nak'd our algorithm of choice
                     * but then suggested it back to us.
                     */
                    goto bad;
                } // if (cichar != CHAP_DIGEST(go->chap_mdtype))
            } else {
                /*
                 * Stop asking for PAP if we were asking for it.
                 */
                tryop.neg_upap = 0;
            } // if (go->neg_eap)

        } else {
            /*
             * If we were asking for EAP, and they're Conf-Naking EAP,
             * well, that's just strange.  Nobody should do that.
             */
            if (cishort == PPP_EAP && cilen == CILEN_SHORT && go->neg_eap)
            {
                ACE_DEBUG ((LM_INFO, ACE_TEXT ("CPPPLCP::Nakci, Unexpected Conf-Nak for EAP\n")));
            }

            /*
             * We don't recognize what they're suggesting.
             * Stop asking for what we were asking for.
             */
            if (go->neg_eap)
            {
                tryop.neg_eap = 0;
            }
            else if (go->neg_chap)
            {
                tryop.neg_chap = 0;
            }
            else
            {
                tryop.neg_upap = 0;
            }
            
            p += cilen - CILEN_SHORT;
        } // if (cishort == PPP_PAP && cilen == CILEN_SHORT)
    }

    /*
     * If they can't cope with our link quality protocol, we'll have
     * to stop asking for LQR.  We haven't got any other protocol.
     * If they Nak the reporting period, take their value XXX ?
     */
    NAKCILQR(CI_QUALITY, 
             neg_lqr,
             if (cishort != PPP_LQR)
             tryop.neg_lqr = 0;
             else
             tryop.lqr_period = cilong;
             );

    /*
     * Only implementing CBCP...not the rest of the callback options
     */
    NAKCICHAR(CI_CALLBACK, 
              neg_cbcp,
              tryop.neg_cbcp = 0;
              );

    /*
     * Check for a looped-back line.
     */
    NAKCILONG(CI_MAGICNUMBER, 
              neg_magicnumber,
              tryop.magicnumber = magic();
              looped_back = 1;
              );

    /*
     * Peer shouldn't send Nak for protocol compression or
     * address/control compression requests; they should send
     * a Reject instead.  If they send a Nak, treat it as a Reject.
     */
    NAKCIVOID(CI_PCOMPRESSION, neg_pcompression);
    NAKCIVOID(CI_ACCOMPRESSION, neg_accompression);

    /*
     * Nak for MRRU option - accept their value if it is smaller
     * than the one we want.
     */
    if (go->neg_mrru) {
        NAKCISHORT(CI_MRRU, 
                   neg_mrru,
                   if (treat_as_reject)
                       tryop.neg_mrru = 0;
                   else if (cishort <= wo->mrru)
                       tryop.mrru = cishort;
                   );
    }

    /*
     * Nak for short sequence numbers shouldn't be sent, treat it
     * like a reject.
     */
    NAKCIVOID(CI_SSNHF, neg_ssnhf);

    /*
     * Nak of the endpoint discriminator option is not permitted,
     * treat it like a reject.
     */
    NAKCIENDP(CI_EPDISC, neg_endpoint);

    /*
     * There may be remaining CIs, if the peer is requesting negotiation
     * on an option that we didn't include in our request packet.
     * If we see an option that we requested, or one we've already seen
     * in this packet, then this packet is bad.
     * If we wanted to respond by starting to negotiate on the requested
     * option(s), we could, but we don't, because except for the
     * authentication type and quality protocol, if we are not negotiating
     * an option, it is because we were told not to.
     * For the authentication type, the Nak from the peer means
     * `let me authenticate myself with you' which is a bit pointless.
     * For the quality protocol, the Nak means `ask me to send you quality
     * reports', but if we didn't ask for them, we don't want them.
     * An option we don't recognize represents the peer asking to
     * negotiate some option we don't support, so ignore it.
     */
    while (len >= CILEN_VOID) {
        GETCHAR(citype, p);
        GETCHAR(cilen, p);
        
        if (cilen < CILEN_VOID || (len -= cilen) < 0)
        {
            goto bad;
        }
        
        next = p + cilen - 2;

        switch (citype) {
            case CI_MRU:
            {
                if ((go->neg_mru && go->mru != DEFMRU)
                    || no.neg_mru || cilen != CILEN_SHORT)
                {
                    goto bad;
                }
                
                GETSHORT(cishort, p);
                
                if (cishort < DEFMRU) {
                    tryop.neg_mru = 1;
                    tryop.mru = cishort;
                }
                
                break;
            }
            case CI_ASYNCMAP:
            {
                if ((go->neg_asyncmap && go->asyncmap != 0xFFFFFFFF)
                    || no.neg_asyncmap || cilen != CILEN_LONG)
                {
                    goto bad;
                }
                
                break;
            }
            case CI_AUTHTYPE:
            {
                if (go->neg_chap || no.neg_chap || go->neg_upap || no.neg_upap || go->neg_eap || no.neg_eap)
                {
                    goto bad;
                }
                break;
            }
            case CI_MAGICNUMBER:
            {
                if (go->neg_magicnumber || no.neg_magicnumber || cilen != CILEN_LONG)
                {
                    goto bad;
                }
                break;
            }
            case CI_PCOMPRESSION:
            {
                if (go->neg_pcompression || no.neg_pcompression || cilen != CILEN_VOID)
                {
                    goto bad;
                }
                break;
            }
            case CI_ACCOMPRESSION:
            {
                if (go->neg_accompression || no.neg_accompression || cilen != CILEN_VOID)
                {
                    goto bad;
                }
                break;
            }
            case CI_QUALITY:
            {
                if (go->neg_lqr || no.neg_lqr || cilen != CILEN_LQR)
                {
                    goto bad;
                }
                break;
            }
            case CI_MRRU:
            {
                if (go->neg_mrru || no.neg_mrru || cilen != CILEN_SHORT)
                {
                    goto bad;
                }
                break;
            }
            case CI_SSNHF:
            {
                if (go->neg_ssnhf || no.neg_ssnhf || cilen != CILEN_VOID)
                {
                    goto bad;
                }
                tryop.neg_ssnhf = 1;
                break;
            }
            case CI_EPDISC:
            {
                if (go->neg_endpoint || no.neg_endpoint || cilen < CILEN_CHAR)
                {
                    goto bad;
                }
                break;
            }
        } // switch (citype)
        
        p = next;
    } // while (len >= CILEN_VOID)

    /*
     * OK, the Nak is good.  Now we can update state.
     * If there are any options left we ignore them.
     */
    if (State() != OPENED) {
        if (looped_back) {
            if (++tryop.numloops >= 4) {
                Close("Loopback detected");
            }
        } else {
            tryop.numloops = 0;
        }
        
        *go = tryop;
    }

    return 1;

bad:
    ACE_DEBUG((LM_ERROR, "CPPPLCP::Nakci, received bad Nak!\n"));
    return 0;
}

int  CPPPLCP::Rejci(CPPPFsm *pfsm, u_char *p, int len)
{
    lcp_options *go = &m_lcp_gotoptions;
    u_char cichar;
    u_short cishort;
    uint32_t cilong;
    lcp_options tryop;        /* options to request next time */

    tryop = *go;

    /*
     * Any Rejected CIs must be in exactly the same order that we sent.
     * Check packet length and CI length at each step.
     * If we find any deviations, then this packet is bad.
     */
#define REJCIVOID(opt, neg) \
    if (go->neg && \
        len >= CILEN_VOID && \
        p[1] == CILEN_VOID && \
        p[0] == opt) { \
        len -= CILEN_VOID; \
        INCPTR(CILEN_VOID, p); \
        tryop.neg = 0; \
    }
#define REJCISHORT(opt, neg, val) \
    if (go->neg && \
        len >= CILEN_SHORT && \
        p[1] == CILEN_SHORT && \
        p[0] == opt) { \
        len -= CILEN_SHORT; \
        INCPTR(2, p); \
        GETSHORT(cishort, p); \
        /* Check rejected value. */ \
        if (cishort != val) \
            goto bad; \
        tryop.neg = 0; \
    }
#define REJCICHAP(opt, neg, val) \
    if (go->neg && \
        len >= CILEN_CHAP && \
        p[1] == CILEN_CHAP && \
        p[0] == opt) { \
        len -= CILEN_CHAP; \
        INCPTR(2, p); \
        GETSHORT(cishort, p); \
        GETCHAR(cichar, p); \
        /* Check rejected value. */ \
        if ((cishort != PPP_CHAP) || (cichar != (CHAP_DIGEST(val)))) \
            goto bad; \
        tryop.neg = 0; \
        tryop.neg_eap = tryop.neg_upap = 0; \
    }
#define REJCILONG(opt, neg, val) \
    if (go->neg && \
        len >= CILEN_LONG && \
        p[1] == CILEN_LONG && \
        p[0] == opt) { \
        len -= CILEN_LONG; \
        INCPTR(2, p); \
        GETLONG(cilong, p); \
        /* Check rejected value. */ \
        if (cilong != val) \
            goto bad; \
        tryop.neg = 0; \
    }
#define REJCILQR(opt, neg, val) \
    if (go->neg && \
        len >= CILEN_LQR && \
        p[1] == CILEN_LQR && \
        p[0] == opt) { \
        len -= CILEN_LQR; \
        INCPTR(2, p); \
        GETSHORT(cishort, p); \
        GETLONG(cilong, p); \
        /* Check rejected value. */ \
        if (cishort != PPP_LQR || cilong != val) \
            goto bad; \
        tryop.neg = 0; \
    }
#define REJCICBCP(opt, neg, val) \
    if (go->neg && \
        len >= CILEN_CBCP && \
        p[1] == CILEN_CBCP && \
        p[0] == opt) { \
        len -= CILEN_CBCP; \
        INCPTR(2, p); \
        GETCHAR(cichar, p); \
        /* Check rejected value. */ \
        if (cichar != val) \
            goto bad; \
        tryop.neg = 0; \
    }
#define REJCIENDP(opt, neg, class, val, vlen) \
    if (go->neg && \
        len >= CILEN_CHAR + vlen && \
        p[0] == opt && \
        p[1] == CILEN_CHAR + vlen) { \
        int i; \
        len -= CILEN_CHAR + vlen; \
        INCPTR(2, p); \
        GETCHAR(cichar, p); \
        if (cichar != class) \
            goto bad; \
        for (i = 0; i < vlen; ++i) { \
            GETCHAR(cichar, p); \
            if (cichar != val[i]) \
            goto bad; \
        } \
        tryop.neg = 0; \
    }

    REJCISHORT(CI_MRU, neg_mru, go->mru);
    REJCILONG(CI_ASYNCMAP, neg_asyncmap, go->asyncmap);
    REJCISHORT(CI_AUTHTYPE, neg_eap, PPP_EAP);
    if (!go->neg_eap) {
        REJCICHAP(CI_AUTHTYPE, neg_chap, go->chap_mdtype);
        if (!go->neg_chap) {
            REJCISHORT(CI_AUTHTYPE, neg_upap, PPP_PAP);
        }
    }
    REJCILQR(CI_QUALITY, neg_lqr, go->lqr_period);
    REJCICBCP(CI_CALLBACK, neg_cbcp, CBCP_OPT);
    REJCILONG(CI_MAGICNUMBER, neg_magicnumber, go->magicnumber);
    REJCIVOID(CI_PCOMPRESSION, neg_pcompression);
    REJCIVOID(CI_ACCOMPRESSION, neg_accompression);
    REJCISHORT(CI_MRRU, neg_mrru, go->mrru);
    REJCIVOID(CI_SSNHF, neg_ssnhf);

    /*
     * If there are any remaining CIs, then this packet is bad.
     */
    if (len != 0)
    {
        goto bad;
    }
    
    /*
     * Now we can update state.
     */
    if (State() != OPENED)
    *go = tryop;
    return 1;

bad:
    ACE_DEBUG((LM_ERROR, "CPPPLCP::Rejci, received bad Reject!"));
    return 0;

}

int  CPPPLCP::Reqci(CPPPFsm *pfsm, u_char *inp, int *lenp, int reject_if_disagree)
{
    static u_char nak_buffer[PPP_MRU];

    lcp_options *go = &m_lcp_gotoptions;
    lcp_options *ho = &m_lcp_hisoptions;
    lcp_options *ao = &m_lcp_allowoptions;
    u_char *cip, *next;     /* Pointer to current and next CIs */
    int cilen, citype, cichar;  /* Parsed len, type, char value */
    u_short cishort;        /* Parsed short value */
    uint32_t cilong;       /* Parse long value */
    int rc = CONFACK;       /* Final packet return code */
    int orc;            /* Individual option return code */
    u_char *p;          /* Pointer to next char to parse */
    u_char *rejp;       /* Pointer to next char in reject frame */
    u_char *nakp;       /* Pointer to next char in Nak frame */
    int l = *lenp;      /* Length left */
    
    /*
    * Reset all his options.
    */
    BZERO(ho, sizeof(*ho));
    
    /*
    * Process all his options.
    */
    next = inp;
    nakp = nak_buffer;
    rejp = inp;
    
    while (l) {
        orc = CONFACK;          /* Assume success */
        cip = p = next;         /* Remember begining of CI */
        if (l < 2 ||            /* Not enough data for CI header or */
            p[1] < 2 ||         /*  CI length too small or */
            p[1] > l) {         /*  CI length too big? */
            ACE_DEBUG((LM_ERROR, "CPPPLCP::Reqci: bad CI length!\n"));
            orc = CONFREJ;      /* Reject bad CI */
            cilen = l;          /* Reject till end of packet */
            l = 0;          /* Don't loop again */
            citype = 0;
            goto endswitch;
        }
        
        GETCHAR(citype, p);     /* Parse CI type */
        GETCHAR(cilen, p);      /* Parse CI length */
        l -= cilen;         /* Adjust remaining length */
        next += cilen;          /* Step to next CI */

        switch (citype) {       /* Check CI type */
            case CI_MRU:
                if (!ao->neg_mru ||     /* Allow option? */
                    cilen != CILEN_SHORT) { /* Check CI length */
                    orc = CONFREJ;      /* Reject CI */
                    break;
                }
                GETSHORT(cishort, p);   /* Parse MRU */

                /*
                 * He must be able to receive at least our minimum.
                 * No need to check a maximum.  If he sends a large number,
                 * we'll just ignore it.
                 */
                if (cishort < MINMRU) {
                    orc = CONFNAK;      /* Nak CI */
                    PUTCHAR(CI_MRU, nakp);
                    PUTCHAR(CILEN_SHORT, nakp);
                    PUTSHORT(MINMRU, nakp); /* Give him a hint */
                    break;
                }
                ho->neg_mru = 1;        /* Remember he sent MRU */
                ho->mru = cishort;      /* And remember value */
                break;

            case CI_ASYNCMAP:
                if (!ao->neg_asyncmap ||
                    cilen != CILEN_LONG) {
                    orc = CONFREJ;
                    break;
                }
                GETLONG(cilong, p);

                /*
                 * Asyncmap must have set at least the bits
                 * which are set in lcp_allowoptions[unit].asyncmap.
                 */
                if ((ao->asyncmap & ~cilong) != 0) {
                    orc = CONFNAK;
                    PUTCHAR(CI_ASYNCMAP, nakp);
                    PUTCHAR(CILEN_LONG, nakp);
                    PUTLONG(ao->asyncmap | cilong, nakp);
                    break;
                }
                ho->neg_asyncmap = 1;
                ho->asyncmap = cilong;
                break;

            case CI_AUTHTYPE:
                if (cilen < CILEN_SHORT ||
                    !(ao->neg_upap || ao->neg_chap || ao->neg_eap)) {
                    /*
                     * Reject the option if we're not willing to authenticate.
                     */
                    orc = CONFREJ;
                    break;
                }
                GETSHORT(cishort, p);

                /*
                 * Authtype must be PAP, CHAP, or EAP.
                 *
                 * Note: if more than one of ao->neg_upap, ao->neg_chap, and
                 * ao->neg_eap are set, and the peer sends a Configure-Request
                 * with two or more authenticate-protocol requests, then we will
                 * reject the second request.
                 * Whether we end up doing CHAP, UPAP, or EAP depends then on
                 * the ordering of the CIs in the peer's Configure-Request.
                     */

                if (cishort == PPP_PAP) {
                    /* we've already accepted CHAP or EAP */
                    if (ho->neg_chap || ho->neg_eap ||
                        cilen != CILEN_SHORT) {
                        ACE_DEBUG((LM_DEBUG, "CPPPLCP::Reqci: rcvd AUTHTYPE PAP, rejecting...\n"));
                        orc = CONFREJ;
                        break;
                    }
                    if (!ao->neg_upap) {    /* we don't want to do PAP */
                        orc = CONFNAK;  /* NAK it and suggest CHAP or EAP */
                        PUTCHAR(CI_AUTHTYPE, nakp);
                        if (ao->neg_eap) {
                            PUTCHAR(CILEN_SHORT, nakp);
                            PUTSHORT(PPP_EAP, nakp);
                        } else {
                            PUTCHAR(CILEN_CHAP, nakp);
                            PUTSHORT(PPP_CHAP, nakp);
                            PUTCHAR(CHAP_DIGEST(ao->chap_mdtype), nakp);
                        }
                        break;
                    }
                    ho->neg_upap = 1;
                    break;
                } // if (cishort == PPP_PAP)
                
                if (cishort == PPP_CHAP) {
                    /* we've already accepted PAP or EAP */
                    if (ho->neg_upap || ho->neg_eap ||
                        cilen != CILEN_CHAP) {
                        ACE_DEBUG((LM_DEBUG, "CPPPLCP::Reqci: rcvd AUTHTYPE CHAP, rejecting...\n"));
                        orc = CONFREJ;
                        break;
                    }
                    if (!ao->neg_chap) {    /* we don't want to do CHAP */
                        orc = CONFNAK;  /* NAK it and suggest EAP or PAP */
                        PUTCHAR(CI_AUTHTYPE, nakp);
                        PUTCHAR(CILEN_SHORT, nakp);
                        if (ao->neg_eap) {
                            PUTSHORT(PPP_EAP, nakp);
                        } else {
                            PUTSHORT(PPP_PAP, nakp);
                        }
                        break;
                    }
                    GETCHAR(cichar, p); /* get digest type */
                    if (!(CHAP_CANDIGEST(ao->chap_mdtype, cichar))) {
                        /*
                         * We can't/won't do the requested type,
                         * suggest something else.
                         */
                        orc = CONFNAK;
                        PUTCHAR(CI_AUTHTYPE, nakp);
                        PUTCHAR(CILEN_CHAP, nakp);
                        PUTSHORT(PPP_CHAP, nakp);
                        PUTCHAR(CHAP_DIGEST(ao->chap_mdtype), nakp);
                        break;
                    }
                    ho->chap_mdtype = CHAP_MDTYPE_D(cichar); /* save md type */
                    ho->neg_chap = 1;
                    break;
                } // if (cishort == PPP_CHAP)
                
                if (cishort == PPP_EAP) {
                    /* we've already accepted CHAP or PAP */
                    if (ho->neg_chap || ho->neg_upap || cilen != CILEN_SHORT) {
                        ACE_DEBUG((LM_DEBUG, "CPPPLCP::Reqci: rcvd AUTHTYPE EAP, rejecting...\n"));
                        orc = CONFREJ;
                        break;
                    }
                    if (!ao->neg_eap) { /* we don't want to do EAP */
                        orc = CONFNAK;  /* NAK it and suggest CHAP or PAP */
                        PUTCHAR(CI_AUTHTYPE, nakp);
                        if (ao->neg_chap) {
                            PUTCHAR(CILEN_CHAP, nakp);
                            PUTSHORT(PPP_CHAP, nakp);
                            PUTCHAR(CHAP_DIGEST(ao->chap_mdtype), nakp);
                        } else {
                            PUTCHAR(CILEN_SHORT, nakp);
                            PUTSHORT(PPP_PAP, nakp);
                        }
                        break;
                    }
                    ho->neg_eap = 1;
                    break;
                } // if (cishort == PPP_EAP)

                /*
                 * We don't recognize the protocol they're asking for.
                 * Nak it with something we're willing to do.
                 * (At this point we know ao->neg_upap || ao->neg_chap ||
                 * ao->neg_eap.)
                 */
                orc = CONFNAK;
                PUTCHAR(CI_AUTHTYPE, nakp);
                if (ao->neg_eap) {
                    PUTCHAR(CILEN_SHORT, nakp);
                    PUTSHORT(PPP_EAP, nakp);
                } else if (ao->neg_chap) {
                    PUTCHAR(CILEN_CHAP, nakp);
                    PUTSHORT(PPP_CHAP, nakp);
                    PUTCHAR(CHAP_DIGEST(ao->chap_mdtype), nakp);
                } else {
                    PUTCHAR(CILEN_SHORT, nakp);
                    PUTSHORT(PPP_PAP, nakp);
                }
                break;

            case CI_QUALITY:
                if (!ao->neg_lqr ||
                    cilen != CILEN_LQR) {
                    orc = CONFREJ;
                    break;
                }

                GETSHORT(cishort, p);
                GETLONG(cilong, p);

                /*
                 * Check the protocol and the reporting period.
                 * XXX When should we Nak this, and what with?
                 */
                if (cishort != PPP_LQR) {
                    orc = CONFNAK;
                    PUTCHAR(CI_QUALITY, nakp);
                    PUTCHAR(CILEN_LQR, nakp);
                    PUTSHORT(PPP_LQR, nakp);
                    PUTLONG(ao->lqr_period, nakp);
                    break;
                }
                break;

            case CI_MAGICNUMBER:
                if (!(ao->neg_magicnumber || go->neg_magicnumber) ||
                    cilen != CILEN_LONG) {
                    orc = CONFREJ;
                    break;
                }
                GETLONG(cilong, p);

                /*
                 * He must have a different magic number.
                 */
                if (go->neg_magicnumber &&
                    cilong == go->magicnumber) {
                    cilong = magic();   /* Don't put magic() inside macro! */
                    orc = CONFNAK;
                    PUTCHAR(CI_MAGICNUMBER, nakp);
                    PUTCHAR(CILEN_LONG, nakp);
                    PUTLONG(cilong, nakp);
                    break;
                }
                ho->neg_magicnumber = 1;
                ho->magicnumber = cilong;
                break;

            case CI_PCOMPRESSION:
                if (!ao->neg_pcompression ||
                    cilen != CILEN_VOID) {
                    orc = CONFREJ;
                    break;
                }
                ho->neg_pcompression = 1;
                break;

            case CI_ACCOMPRESSION:
                if (!ao->neg_accompression ||
                    cilen != CILEN_VOID) {
                    orc = CONFREJ;
                    break;
                }
                ho->neg_accompression = 1;
                break;

            default:
                ACE_DEBUG((LM_DEBUG, "CPPPLCP::Reqci: rcvd unknown option %d\n", citype));
                orc = CONFREJ;
                break;
        } // switch (citype)

    endswitch:
        if (orc == CONFACK &&       /* Good CI */
            rc != CONFACK)      /*  but prior CI wasnt? */
            continue;           /* Don't send this one */

        if (orc == CONFNAK) {       /* Nak this CI? */
            if (reject_if_disagree  /* Getting fed up with sending NAKs? */
                && citype != CI_MAGICNUMBER) {
                orc = CONFREJ;      /* Get tough if so */
            } else {
                if (rc == CONFREJ)  /* Rejecting prior CI? */
                    continue;       /* Don't send this one */
                rc = CONFNAK;
            }
        }
        if (orc == CONFREJ) {       /* Reject this CI */
            rc = CONFREJ;
            if (cip != rejp)        /* Need to move rejected CI? */
            {
                BCOPY(cip, rejp, cilen); /* Move it */
            }
            INCPTR(cilen, rejp);    /* Update output pointer */
        }
    } // while (l)

    /*
     * If we wanted to send additional NAKs (for unsent CIs), the
     * code would go here.  The extra NAKs would go at *nakp.
     * At present there are no cases where we want to ask the
     * peer to negotiate an option.
     */

    switch (rc) {
        case CONFACK:
            *lenp = next - inp;
            break;
        case CONFNAK:
            /*
             * Copy the Nak'd options from the nak_buffer to the caller's buffer.
             */
            *lenp = nakp - nak_buffer;
            BCOPY(nak_buffer, inp, *lenp);
            break;
        case CONFREJ:
            *lenp = rejp - inp;
            break;
    }

    ACE_DEBUG((LM_DEBUG, "CPPPLCP::Reqci: returning CONF%s.\n", CODENAME(rc)));
    return (rc);            /* Return final code */
}

void CPPPLCP::Up(CPPPFsm *pfsm)
{
    ACE_DEBUG ((LM_DEBUG,"CPPPLCP::Up\n"));

    lcp_options *wo = &m_lcp_wantoptions;
    lcp_options *ho = &m_lcp_hisoptions;
    lcp_options *go = &m_lcp_gotoptions;
    lcp_options *ao = &m_lcp_allowoptions;
    int mtu, mru;

    if (!go->neg_magicnumber)
        go->magicnumber = 0;
    if (!ho->neg_magicnumber)
        ho->magicnumber = 0;

    /*
     * Set our MTU to the smaller of the MTU we wanted and
     * the MRU our peer wanted.  If we negotiated an MRU,
     * set our MRU to the larger of value we wanted and
     * the value we got in the negotiation.
     * Note on the MTU: the link MTU can be the MRU the peer wanted,
     * the interface MTU is set to the lowest of that, the
     * MTU we want to use, and our link MRU.
     */
    mtu = ho->neg_mru? ho->mru: PPP_MRU;
    mru = go->neg_mru? MAX(wo->mru, go->mru): PPP_MRU;

    if (ho->neg_mru)
    m_peer_mru = ho->mru;

    EchoLowerUp();  /* Enable echo messages */

    if (m_psink)
    {
        m_psink->OnLCPUp();
    }
}

void CPPPLCP::Down(CPPPFsm *pfsm)
{
    ACE_DEBUG ((LM_DEBUG,"CPPPLCP::Down\n"));

    EchoLowerDown();

    if (m_psink)
    {
        m_psink->OnLCPDown();
    }

    m_peer_mru = PPP_MRU;
}

void CPPPLCP::Starting(CPPPFsm *pfsm)
{
    ACE_DEBUG ((LM_DEBUG,"CPPPLCP::Starting\n"));

    LowerUp();
}

void CPPPLCP::Finished(CPPPFsm *pfsm)
{
    ACE_DEBUG ((LM_DEBUG,"CPPPLCP::Finished\n"));

    if (m_psink)
    {
        m_psink->OnLCPTerminate();
    }
}

void CPPPLCP::ProtReject(int)
{
    ACE_DEBUG((LM_DEBUG, "CPPPLCP::ProtReject\n"));
}

void CPPPLCP::Retransmit(CPPPFsm *pfsm)
{
    ACE_DEBUG((LM_DEBUG, "CPPPLCP::Retransmit\n"));
}

int CPPPLCP::ExtCode(CPPPFsm *pfsm, int code, int id, u_char *inp, int len)
{
    u_char *magp;

    switch( code ){
        case PROTREJ:
            LCPRcvProtRej(inp, len);
            break;

        case ECHOREQ:
            if (m_fsm.State() != OPENED)
                break;
            magp = inp;
            PUTLONG(m_magicnumber, magp);
            m_fsm.SendData(ECHOREP, id, inp, len);
            break;

        case ECHOREP:
            RcvEchoReply(id, inp, len);
            break;

        case DISCREQ:
        case IDENTIF:
        case TIMEREM:
            break;

        default:
            return 0;
    }
    
    return 1;
}

/*
 * lcp_sprotrej - Send a Protocol-Reject for some protocol.
 */
void
CPPPLCP::SendProtRej(u_char *p, int len)
{
    ACE_DEBUG((LM_DEBUG, "CPPPLCP::SendProtRej, len=%d\n", len));
    
    /*
     * Send back the protocol and the information field of the
     * rejected packet.  We only get here if LCP is in the OPENED state.
     */
    p += 2;
    len -= 2;

    int id = m_fsm.GetId();

    m_fsm.SendData(PROTREJ, id, p, len);
}


/*
 * lcp_rprotrej - Receive an Protocol-Reject.
 *
 * Figure out which protocol is rejected and inform it.
 */
void
CPPPLCP::LCPRcvProtRej(u_char *inp, int len)
{
    u_short prot;

    ACE_DEBUG((LM_DEBUG, "CPPPLCP::LCPRcvProtRej, len=%d\n", len));
    
    if (len < 2) {
        ACE_DEBUG((LM_ERROR, "CPPPLCP::LCPRcvProtRej, Rcvd short Protocol-Reject packet!\n"));
        return;
    }

    GETSHORT(prot, inp);

    /*
     * Protocol-Reject packets received in any state other than the LCP
     * OPENED state SHOULD be silently discarded.
     */
    if (State() != OPENED ) {
        ACE_DEBUG((LM_DEBUG, "CPPPLCP::LCPRcvProtRej, Protocol-Reject discarded: LCP in state %d\n", State()));
        return;
    }
}

//Output Packet
void CPPPLCP::OutputPacket(unsigned char *pkg, size_t size)
{
    if (m_psink)
    {
        m_psink->OnLCPOutput(pkg,size);
    }
}

//check Lcp status
int CPPPLCP::State()
{
    return m_fsm.State();
}


