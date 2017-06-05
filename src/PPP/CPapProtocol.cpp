/*
 * Copyright (c) 2014  Nanjing WFNEX Technology Co., Ltd.  All rights reserved.
 */

#include "CPapProtocol.h"
#include <string.h>

CPapProtocolSvr::CPapProtocolSvr(IAuthenSvrSink *psink)
    : m_psink(psink)
    , m_reqtimeout(0)
    , m_usrReqId(0)
{
}

CPapProtocolSvr::~CPapProtocolSvr()
{
    ACE_DEBUG ((LM_DEBUG, "CPapProtocolSvr::~CPapProtocolSvr"));
    CancelTimer();
}

void CPapProtocolSvr::CancelTimer()
{
    ACE_DEBUG((LM_DEBUG, "CPapProtocolSvr::CancelTimer\n"));
    ACE_Reactor::instance()->cancel_timer(this);
}

void CPapProtocolSvr::StartTimer(int seconds)
{
    ACE_DEBUG((LM_DEBUG, "CPapProtocolSvr::StartTimer, seconds=%d\n", seconds));

    CancelTimer();
    
    ACE_Time_Value delay(seconds);
    ACE_Reactor::instance()->schedule_timer(this, 0, delay, delay); 
}

void CPapProtocolSvr::Init()
{
    ACE_DEBUG((LM_DEBUG, "CPapProtocolSvr::Init\n"));
    m_reqtimeout = UPAP_DEFREQTIME;
}

void CPapProtocolSvr::Input(unsigned char *pkt, size_t pktlen)
{
    ACE_DEBUG ((LM_DEBUG, "CPapProtocolSvr::Input, pktlen=%d\n", pktlen));

    BYTE *inp = NULL;
    BYTE code = 0, id = 0;
    int len = 0;

    /*
     * Parse header (code, id and length).
     * If packet too short, drop it.
     */
    inp = pkt;
    if (pktlen < UPAP_HEADERLEN) 
    {
    	ACE_DEBUG((LM_ERROR, "CPapProtocolSvr::Input, rcvd short header. pktlen=%u\n", pktlen));
    	return;
    }
    
    GETCHAR(code, inp);
    GETCHAR(id, inp);
    GETSHORT(len, inp);
    
    if (len < UPAP_HEADERLEN) 
    {
    	ACE_DEBUG((LM_ERROR, "CPapProtocolSvr::Input, rcvd illegal length(%d).\n", len));
    	return;
    }
    
    if (len > pktlen) 
    {
    	ACE_DEBUG((LM_ERROR, "CPapProtocolSvr::Input, rcvd short packet. len=%d, pktlen=%u\n", len, pktlen));
    	return;
    }
    
    len -= UPAP_HEADERLEN;

    /*
     * Action depends on code.
     */
    switch (code) 
    {
        case UPAP_AUTHREQ:
        {
        	upap_rauthreq(inp, id, len);
        	break;
        }
        case UPAP_AUTHACK:
        case UPAP_AUTHNAK:
        default:				/* XXX Need code reject */
    	{
            ACE_DEBUG((LM_ERROR, "CPapProtocolSvr::Input, invalid pap code(%#x)\n", code));
            break;
        }
    }
}

/*
 * upap_protrej - Peer doesn't speak this protocol.
 *
 * This shouldn't happen.  In any case, pretend lower layer went down.
 */
void CPapProtocolSvr::Protrej()
{
    //if (u->us_serverstate == UPAPSS_LISTEN) {
	ACE_DEBUG((LM_DEBUG, "CPapProtocolSvr::Protrej, PAP authentication of peer failed (protocol-reject)"));
    m_psink->OnAuthenResult(0, PPP_PAP);
    //}
    LowerDown();
}

/*
 * upap_lowerup - The lower layer is up.
 *
 * Start authenticating if pending.
 */
void CPapProtocolSvr::LowerUp()
{
    ACE_DEBUG((LM_DEBUG, "CPapProtocolSvr::LowerUp\n"));
    
    //if (u->us_serverstate == UPAPSS_INITIAL)
	//u->us_serverstate = UPAPSS_CLOSED;
    //else if (u->us_serverstate == UPAPSS_PENDING) {
	//u->us_serverstate = UPAPSS_LISTEN;
	if (m_reqtimeout > 0)
	{
	    StartTimer(m_reqtimeout);
    }
}

/*
 * upap_lowerdown - The lower layer is down.
 *
 * Cancel all timeouts.
 */
void CPapProtocolSvr::LowerDown()
{
    ACE_DEBUG((LM_DEBUG, "CPapProtocolSvr::LowerDown\n"));
    
    //if (u->us_serverstate == UPAPSS_LISTEN && u->us_reqtimeout > 0)
    if (m_reqtimeout > 0)
    {
        CancelTimer();
    }
    
    //u->us_serverstate = UPAPSS_INITIAL;
}

void CPapProtocolSvr::Open()
{
    ACE_DEBUG((LM_DEBUG, "CPapProtocolSvr::Open\n"));
    
    return;
}

void CPapProtocolSvr::Close(char *reason)
{
    ACE_DEBUG((LM_DEBUG, "CPapProtocolSvr::Close\n"));
    
    return;
}

int CPapProtocolSvr::handle_timeout (const ACE_Time_Value &current_time, const void *act)
{
    //if (u->us_serverstate != UPAPSS_LISTEN)
	//return;			/* huh?? */

	ACE_DEBUG((LM_DEBUG, 
            	"CPapProtocolSvr::handle_timeout, we don't rcv PAP authentication req in %d seconds. So authen failed.\n",
            	m_reqtimeout));
    m_psink->OnAuthenResult(0, PPP_PAP);

    //u->us_serverstate = UPAPSS_BADAUTH;

    CancelTimer();
    
    return 0;
}

/*
 * upap_rauth - Receive Authenticate.
 */
void
CPapProtocolSvr::upap_rauthreq(BYTE *inp, int id, int len)
{
    u_char ruserlen, rpasswdlen;
    char *ruser, *rpasswd;

    ACE_DEBUG((LM_DEBUG, "CPapProtocolSvr::upap_rauthreq, id=%#x, len=%d\n", id, len));
    
    // In current implementation, we don't record the status of PAP authentication.
    #if 0
    if (u->us_serverstate < UPAPSS_LISTEN)
	return;

    /*
     * If we receive a duplicate authenticate-request, we are
     * supposed to return the same status as for the first request.
     */
    if (u->us_serverstate == UPAPSS_OPEN) {
	upap_sresp(u, UPAP_AUTHACK, id, "", 0);	/* return auth-ack */
	return;
    }
    if (u->us_serverstate == UPAPSS_BADAUTH) {
	upap_sresp(u, UPAP_AUTHNAK, id, "", 0);	/* return auth-nak */
	return;
    }
    #endif

    /*
     * Parse user/passwd.
     */
    if (len < 1) 
    {
    	ACE_DEBUG((LM_ERROR, "CPapProtocolSvr::upap_rauthreq, rcvd short packet. len=%d\n", len));
    	return;
    }
    GETCHAR(ruserlen, inp);
    len -= sizeof (BYTE) + ruserlen + sizeof (BYTE);
    if (len < 0) 
    {
    	ACE_DEBUG((LM_ERROR, "CPapProtocolSvr::upap_rauthreq, rcvd short packet. len=%d\n", len));
    	return;
    }
    
    ruser = (char *) inp;
    INCPTR(ruserlen, inp);
    GETCHAR(rpasswdlen, inp);
    if (len < rpasswdlen) 
    {
    	ACE_DEBUG((LM_ERROR, 
                   "CPapProtocolSvr::upap_rauthreq, rcvd short packet. len=%d, rpasswdlen=%u\n",
                   len, rpasswdlen));
        
    	return;
    }
    rpasswd = (char *) inp;

    std::string username(ruser, ruserlen);

    m_username = username;
    m_usrReqId = id;

    Auth_Request authReq;
    ::memset(&authReq, 0, sizeof authReq);

    authReq.authtype = PAP;

    size_t cpyLen = (ruserlen < AUTHMGR_MAX_USERNAME_SIZE - 1) ? ruserlen: AUTHMGR_MAX_USERNAME_SIZE - 1;
    strncpy(authReq.username, ruser, cpyLen);
    cpyLen = (rpasswdlen < AUTHMGR_MAX_PASSWD_SIZE - 1) ? rpasswdlen : AUTHMGR_MAX_PASSWD_SIZE - 1;
    strncpy(authReq.userpasswd, rpasswd, cpyLen);    
    
    m_psink->SendAuthRequest2AM(authReq);
    
    BZERO(rpasswd, rpasswdlen);
}

void CPapProtocolSvr::ResponseAuthenResult(int result, std::string &reason)
{
    char rhostname[256] = {0};
    int retcode = UPAP_AUTHNAK;
    int msglen = 0;

    ACE_DEBUG((LM_DEBUG, 
               "CPapProtocolSvr::ResponseAuthenResult, result=%d, reason=%s\n", 
               result,
               reason.c_str()));
    
    if (0 == result)
    {
        retcode = UPAP_AUTHACK;
    }
    else 
    {
        retcode = UPAP_AUTHNAK;
    }
    
    msglen = reason.length();
    if (msglen > 255)
    {
    	msglen = 255;
    }
    upap_sresp(retcode, m_usrReqId, const_cast<CHAR *>(reason.c_str()), msglen);

    /* Null terminate and clean remote name. */
    snprintf(rhostname, sizeof(rhostname), "%.*s", m_username.length(), m_username.c_str());
    rhostname[255] = 0;

    if (retcode == UPAP_AUTHACK) 
    {
    	//u->us_serverstate = UPAPSS_OPEN;
    	ACE_DEBUG((LM_DEBUG, "CPapProtocolSvr::ResponseAuthenResult, PAP peer authentication succeeded for %s\n", rhostname));
        m_psink->OnAuthenResult(1, PPP_PAP);
    } 
    else 
    {
    	//u->us_serverstate = UPAPSS_BADAUTH;
    	ACE_DEBUG((LM_DEBUG, "CPapProtocolSvr::ResponseAuthenResult, PAP peer authentication failed for %q\n", rhostname));
        m_psink->OnAuthenResult(0, PPP_PAP);
    }

    if (m_reqtimeout > 0)
    {
        CancelTimer();
    }
}

/*
 * upap_sresp - Send a response (ack or nak).
 */
void
CPapProtocolSvr::upap_sresp(BYTE code, BYTE id, CHAR *msg, int msglen)
{
    BYTE *outp;
    int outlen;

    ACE_DEBUG((LM_DEBUG,
               "CPapProtocolSvr::upap_sresp, code=%#x, id=%#x, msglen=%d\n",
               code, id, msglen));

    outlen = UPAP_HEADERLEN + sizeof (BYTE) + msglen;
    
    static BYTE outpacket_buf[PPP_MRU+PPP_HDRLEN]; /* buffer for outgoing packet */

    outp = outpacket_buf;
    MAKEHEADER(outp, PPP_PAP);

    PUTCHAR(code, outp);
    PUTCHAR(id, outp);
    PUTSHORT(outlen, outp);
    PUTCHAR(msglen, outp);
    BCOPY(msg, outp, msglen);

    if (m_psink)
    {
        m_psink->OnAuthenOutput(outpacket_buf, outlen + PPP_HDRLEN);    
    }
}


