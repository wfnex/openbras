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


#include "CChapProtocol.h"
#include <string.h>
#include "random.h"

CChapProtocolSvr::CChapProtocolSvr(IAuthenSvrSink *psink)
    : m_psink(psink)
    , m_timeout_time(3)
    , m_max_transmits(10)
    , m_rechallenge_time(0)
{
}

CChapProtocolSvr::~CChapProtocolSvr()
{
    ACE_DEBUG ((LM_DEBUG, "CChapProtocolSvr::~CChapProtocolSvr\n"));
    CancelTimer();
}

// for class CPPPProtocol
void CChapProtocolSvr::Init()
{
    // Note: 目前只支持MD5，不支持MS-CHAP和MS-CHAPV2。所以没有考虑扩展性，PPP2.4.7这样处理:
    // 将3种类型的MD抽象成struct chap_digest_type，然后挂在chap_digests链表。用OOP思想，即是将chap_digest_type抽象成类，
    // 也可挂在list。
	//chap_md5_init();
#ifdef CHAPMS
    chapms_init();
#endif    
}

//Input handle
void CChapProtocolSvr::Input(unsigned char *packet ,size_t size)
{
    unsigned char code, id;
    int len;

    ACE_DEBUG ((LM_INFO, "CChapProtocolSvr::Input, size=%u\n", size));

    if (size < CHAP_HDRLEN)
    {
        ACE_DEBUG ((LM_ERROR, "size too short.\n"));
        return;
    }

    GETCHAR(code, packet);
    GETCHAR(id, packet);
    GETSHORT(len, packet);

    if (len < CHAP_HDRLEN || len > size)
    {
        ACE_DEBUG ((LM_ERROR, "invalid length(%u)\n", len));
        return;
    }
    
    len -= CHAP_HDRLEN;

    switch (code) 
    {
        case CHAP_RESPONSE:
        {
            chap_handle_response(id, packet, len);
            break;
        }
        case CHAP_CHALLENGE:
        case CHAP_FAILURE:
        case CHAP_SUCCESS:
        default:
        {
            ACE_DEBUG ((LM_ERROR, "Invalid code(%#x)\n", code));
            break;
        }
    }    
}

//check auth status 
void CChapProtocolSvr::Protrej()
{
    if (m_server.flags & TIMEOUT_PENDING) 
    {
        m_server.flags &= ~TIMEOUT_PENDING;
        CancelTimer();
    }
    if (m_server.flags & AUTH_STARTED) 
    {
        m_server.flags = 0;
        m_psink->OnAuthenResult(0, PPP_CHAP);
    }
}

/*
 * chap_lowerup - we can start doing stuff now.
 */
void CChapProtocolSvr::LowerUp()
{
    m_server.flags |= LOWERUP;
    if (m_server.flags & AUTH_STARTED)
    {
        ACE_Time_Value current_time; // current_time is NOT used in handle_timeout().
        handle_timeout (current_time);
    }
}

void CChapProtocolSvr::LowerDown()
{
    if (m_server.flags & TIMEOUT_PENDING)
    {
        CancelTimer();
    }
    m_server.flags = 0;
}

void CChapProtocolSvr::Open()
{
    return;
}

void CChapProtocolSvr::Close(char *reason)
{
    return;
}

// for class ACE_Event_Handler
/*
 * chap_timeout - It's time to send another challenge to the peer.
 * This could be either a retransmission of a previous challenge,
 * or a new challenge to start re-authentication.
 */
// TBD !!! Is retransmission conform to the security considerations of RFC 1334?

//Timeout Handle
int CChapProtocolSvr::handle_timeout (const ACE_Time_Value &current_time, const void *act)
{
    ACE_DEBUG ((LM_DEBUG, "CChapProtocolSvr::handle_timeout\n"));

    m_server.flags &= ~TIMEOUT_PENDING;

    if ((m_server.flags & CHALLENGE_VALID) == 0) 
    {
    m_server.challenge_xmits = 0;
    chap_generate_challenge();
    m_server.flags |= CHALLENGE_VALID;
    } 
    else if (m_server.challenge_xmits >= m_max_transmits) 
    {
    m_server.flags &= ~CHALLENGE_VALID;
    m_server.flags |= AUTH_DONE | AUTH_FAILED;

    ACE_DEBUG((LM_DEBUG, 
               "the number of transmitting challenges exceeds %d times\n", 
               m_max_transmits));
    m_psink->OnAuthenResult(0, PPP_CHAP);        

    return 0;
    }

    m_psink->OnAuthenOutput(m_server.challenge, m_server.challenge_pktlen);
    ++m_server.challenge_xmits;
    m_server.flags |= TIMEOUT_PENDING;

    return 0;
}

//Cancel Timer
void CChapProtocolSvr::CancelTimer()
{
    ACE_DEBUG((LM_DEBUG, "CChapProtocolSvr::CancelTimer\n"));
    ACE_Reactor::instance()->cancel_timer(this);    
}

//Start Timer
void CChapProtocolSvr::StartTimer(int seconds)
{
    ACE_DEBUG((LM_DEBUG, "CChapProtocolSvr::StartTimer, seconds=%d\n", seconds));

    CancelTimer();
    
    ACE_Time_Value delay(seconds);
    ACE_Reactor::instance()->schedule_timer(this, 0, delay, delay);     
}

/*
 * chap_handle_response - check the response to our challenge.
 */
void
CChapProtocolSvr::chap_handle_response(int id, unsigned char *pkt, int len)
{
    int response_len;
    unsigned char *response;
    char *name = NULL;	/* initialized to shut gcc up */

    ACE_DEBUG ((LM_DEBUG, "CChapProtocolSvr::chap_handle_response, id=%d, len=%d\n", id, len));

    if ((m_server.flags & LOWERUP) == 0)
    {
    ACE_DEBUG ((LM_ERROR, "lower still down\n"));
    return;
    }

    if (id != m_server.challenge[PPP_HDRLEN+1])
    {
    ACE_DEBUG ((LM_ERROR, 
                "id dismatch, id=%d, m_server.challenge[PPP_HDRLEN+1]=%u\n",
                id,
                m_server.challenge[PPP_HDRLEN+1]));
    return;
    }

    if (len < 2)
    {
        ACE_DEBUG ((LM_ERROR, "invalid len(%d)\n", len));
        return;
    }
    
    if (m_server.flags & CHALLENGE_VALID) 
    {
        response = pkt;
        GETCHAR(response_len, pkt);
        len -= (response_len + 1);	/* length of name */
        name = (char *)pkt + response_len;
        if (len < 0)
        return;

        if (m_server.flags & TIMEOUT_PENDING) {
        m_server.flags &= ~TIMEOUT_PENDING;
        CancelTimer();
    }

        m_username = std::string(name, len);

        if (m_psink)
        {
            Auth_Request authReq;
            ::memset(&authReq, 0, sizeof authReq);

            // TBD!!!! 目前域名也存放在username中，domain字段空着。

            authReq.authtype = PPP_CHAP;
            authReq.chapid = id;
            size_t cpyLen = (len < AUTHMGR_MAX_USERNAME_SIZE - 1) ? len : (AUTHMGR_MAX_USERNAME_SIZE - 1);
            ::strncpy(authReq.username, name, cpyLen);

            cpyLen = (response_len <= AUTHMGR_MAX_PASSWD_SIZE) ? response_len : AUTHMGR_MAX_PASSWD_SIZE;
            ::memcpy(authReq.userpasswd, (const CHAR *)(response + 1), cpyLen);

            BYTE challengeLen = *(m_server.challenge + PPP_HDRLEN + CHAP_HDRLEN);
            if (challengeLen != AUTHMGR_MAX_CHALLENGE_SIZE)
            {
                ACE_DEBUG ((LM_ERROR, "challenge length not equal %u\n", AUTHMGR_MAX_CHALLENGE_SIZE));
                return;
            }
            //cpyLen = (challengeLen < AUTHMGR_MAX_CHALLENGE_SIZE) ? challengeLen : AUTHMGR_MAX_CHALLENGE_SIZE;
            cpyLen = AUTHMGR_MAX_CHALLENGE_SIZE;
            ::memcpy(authReq.challenge, m_server.challenge + PPP_HDRLEN + CHAP_HDRLEN + 1, cpyLen);
            
            m_psink->SendAuthRequest2AM(authReq);
        }
    } 
    else if ((m_server.flags & AUTH_DONE) == 0)
    {
        ACE_DEBUG ((LM_INFO, "authentication already done.\n"));
        return;
    }
}

// Receive results from AM
void CChapProtocolSvr::ResponseAuthenResult(int result, std::string &reason)
{
    BYTE *p = NULL;
    SWORD32 mlen = 0;
    SWORD32 len = 0;
    
    ACE_DEBUG ((LM_DEBUG, 
                "CChapProtocolSvr::ResponseAuthenResult, result=%d, reason=%s\n",
                result,
                reason.c_str()));
    
    if (result != 0) 
    {
        m_server.flags |= AUTH_FAILED;
        ACE_DEBUG ((LM_ERROR, "Peer %s failed CHAP authentication\n", m_username.c_str()));
    }

    static BYTE outpacket_buf[PPP_MRU+PPP_HDRLEN]; /* buffer for outgoing packet */

    /* send the response */
    p = outpacket_buf;
    MAKEHEADER(p, PPP_CHAP);
    mlen = (reason.size() < PPP_MRU - CHAP_HDRLEN) ? reason.size() : (PPP_MRU - CHAP_HDRLEN);
    len = CHAP_HDRLEN + mlen;
    p[0] = (m_server.flags & AUTH_FAILED)? CHAP_FAILURE : CHAP_SUCCESS;
    p[1] = m_server.challenge[PPP_HDRLEN+1];
    p[2] = len >> 8;
    p[3] = len;
    if (mlen > 0)
    {
        memcpy(p + CHAP_HDRLEN, reason.c_str(), mlen);
    }

    if (m_psink)
    {
        m_psink->OnAuthenOutput(outpacket_buf, PPP_HDRLEN + len);
    }
    
    if (m_server.flags & CHALLENGE_VALID) 
    {
        m_server.flags &= ~CHALLENGE_VALID;

        if (m_server.flags & AUTH_FAILED) 
        {
            m_psink->OnAuthenResult(0, PPP_CHAP);
        } 
        else 
        {
            if ((m_server.flags & AUTH_DONE) == 0)
            {
                m_psink->OnAuthenResult(1, PPP_CHAP);
            }
            if (m_rechallenge_time) 
            {
                m_server.flags |= TIMEOUT_PENDING;
                StartTimer(m_rechallenge_time);
            }
        }
        m_server.flags |= AUTH_DONE;
    }
}

/*
 * chap_generate_challenge - generate a challenge string and format
 * the challenge packet in ss->challenge_pkt.
 */
void
CChapProtocolSvr::chap_generate_challenge()
{
    int clen = 1, nlen, len;
    unsigned char *p;

    p = m_server.challenge;
    MAKEHEADER(p, PPP_CHAP);
    p += CHAP_HDRLEN;
    chap_md5_generate_challenge(p);
    clen = *p;
    nlen = strlen(m_server.name);
    memcpy(p + 1 + clen, m_server.name, nlen);

    len = CHAP_HDRLEN + 1 + clen + nlen;
    m_server.challenge_pktlen = PPP_HDRLEN + len;

    p = m_server.challenge + PPP_HDRLEN;
    p[0] = CHAP_CHALLENGE;
    p[1] = ++m_server.id;
    p[2] = len >> 8;
    p[3] = len;
}

void
CChapProtocolSvr::chap_md5_generate_challenge(unsigned char *cp)
{
    int clen;

    //clen = (int)(drand48() * (MD5_MAX_CHALLENGE - MD5_MIN_CHALLENGE))
    //	+ MD5_MIN_CHALLENGE;
    clen = (int)AUTHMGR_MAX_CHALLENGE_SIZE;
    *cp++ = clen;
    random_bytes(cp, clen);
}

/*
 * chap_auth_peer - Start authenticating the peer.
 * If the lower layer is already up, we start sending challenges,
 * otherwise we wait for the lower layer to come up.
 */
void CChapProtocolSvr::chap_auth_peer(std::string &our_name, int digest_code)
{
    //struct chap_digest_type *dp;
    ACE_DEBUG ((LM_DEBUG, 
                "CChapProtocolSvr::chap_auth_peer, our_name=%s, digest_code=%d\n",
                our_name.c_str(),
                digest_code));

    if (m_server.flags & AUTH_STARTED) 
    {
        ACE_DEBUG ((LM_ERROR, "CHAP: peer authentication already started!\n"));
        return;
    }

    // Currently, we only support MD5.
    /*for (dp = chap_digests; dp != NULL; dp = dp->next)
    if (dp->code == digest_code)
    break;
    if (dp == NULL)
    fatal("CHAP digest 0x%x requested but not available",
      digest_code);
    */
    //ss->digest = dp;

#define BRAS_CHAP_MAXNAMELEN	256	/* max length of hostname or name for auth */

    static CHAR host_name[BRAS_CHAP_MAXNAMELEN];   /* Our name for authentication purposes */
    ::strncpy(host_name, our_name.c_str(), BRAS_CHAP_MAXNAMELEN - 1);
    host_name[BRAS_CHAP_MAXNAMELEN - 1] = 0;

    m_server.name = host_name;
    /* Start with a random ID value */
    m_server.id = (unsigned char)(drand48() * 256);
    m_server.flags |= AUTH_STARTED;
    if (m_server.flags & LOWERUP)
    {
    	ACE_Time_Value current_time; // current_time is NOT used in handle_timeout();
        handle_timeout(current_time);
    }
}


