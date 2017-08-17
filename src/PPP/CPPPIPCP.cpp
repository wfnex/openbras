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


#include "CPPPIPCP.h"


CPPPIPCP::CPPPIPCP(IPPPIPCPSink *psink)
    :m_psink(psink)
    ,m_fsm(this,PPP_IPCP, "IPCP")
    ,m_busepeerdns(false)
    ,m_ask_for_local(true)
    ,m_noremoteip(false)
    ,m_ipcp_is_open(false)
{
    ::memset(&m_ipcp_wantoptions, 0, sizeof(m_ipcp_wantoptions));
    ::memset(&m_ipcp_gotoptions, 0, sizeof(m_ipcp_gotoptions));
    ::memset(&m_ipcp_allowoptions, 0, sizeof(m_ipcp_allowoptions));
    ::memset(&m_ipcp_hisoptions, 0, sizeof(m_ipcp_hisoptions));
}

CPPPIPCP::~CPPPIPCP()
{
}

//Init ipcp options
void CPPPIPCP::Init()
{
    ACE_DEBUG((LM_DEBUG, "CPPPIPCP::Init\n"));
    
    ipcp_options *wo = &m_ipcp_wantoptions;
    ipcp_options *ao = &m_ipcp_allowoptions;

    m_fsm.Init();

    /*
     * Some 3G modems use repeated IPCP NAKs as a way of stalling
     * until they can contact a server on the network, so we increase
     * the default number of NAKs we accept before we start treating
     * them as rejects.
     */
    m_fsm.m_maxnakloops = 100;

    memset(wo, 0, sizeof(*wo));
    memset(ao, 0, sizeof(*ao));

    wo->neg_addr = wo->old_addrs = 1;
    wo->neg_vj = 0;
    //wo->vj_protocol = IPCP_VJ_COMP;
    //wo->maxslotindex = MAX_STATES - 1; /* really max index */
    wo->cflag = 1;

    /* max slots and slot-id compression are currently hardwired in */
    /* ppp_if.c to 16 and 1, this needs to be changed (among other */
    /* things) gmc */

    ao->neg_addr = ao->old_addrs = 1;
    ao->neg_vj = 0;
    //ao->maxslotindex = MAX_STATES - 1;
    ao->cflag = 1;

    /*
     * XXX These control whether the user may use the proxyarp
     * and defaultroute options.
     */
    ao->proxy_arp = 1;
    ao->default_route = 1;
}

//Input packet
void CPPPIPCP::Input(unsigned char *packet ,size_t size)
{
    #ifdef DEBUG_PKT
    ACE_DEBUG ((LM_DEBUG, "CPPPIPCP::Input, size=%u\n", size));
    #endif
    
    m_fsm.Input(packet, size);
}

//A Protocol-Reject was received.
void CPPPIPCP::Protrej()
{
    ACE_DEBUG ((LM_DEBUG, "CPPPIPCP::Protrej\n"));
    m_fsm.LowerDown();
}

//The lower layer is up.
void CPPPIPCP::LowerUp()
{
    ACE_DEBUG ((LM_DEBUG, "CPPPIPCP::LowerUp\n"));
    m_fsm.LowerUp();
}

//The lower layer is down
void CPPPIPCP::LowerDown()
{
    ACE_DEBUG ((LM_DEBUG, "CPPPIPCP::LowerDown\n"));
    m_fsm.LowerDown();
}

void CPPPIPCP::Open()
{
    ACE_DEBUG ((LM_DEBUG,"CPPPIPCP::Open\n"));

    m_ipcp_is_open = true;
    m_fsm.Open();
}

void CPPPIPCP::Close(char *reason)
{
    ACE_DEBUG ((LM_DEBUG, "CPPPIPCP::Close, reason=%s\n", reason));
    m_fsm.Close(reason);
}

//fsm
void CPPPIPCP::Resetci(CPPPFsm *pfsm)
{
    ACE_DEBUG ((LM_DEBUG, "CPPPIPCP::Resetci\n"));
    
    ipcp_options *wo = &m_ipcp_wantoptions;
    ipcp_options *go = &m_ipcp_gotoptions;
    ipcp_options *ao = &m_ipcp_allowoptions;

    wo->req_addr = (wo->neg_addr || wo->old_addrs) && (ao->neg_addr || ao->old_addrs);
    
    if (wo->ouraddr == 0)
    {
        wo->accept_local = 1;
    }
    
    if (wo->hisaddr == 0)
    {
        wo->accept_remote = 1;
    }
    
    wo->req_dns1 = m_busepeerdns;	/* Request DNS addresses from the peer */
    wo->req_dns2 = m_busepeerdns;
    
    *go = *wo;
    
    if (!m_ask_for_local)
    {
        go->ouraddr = 0;
    }
    
    BZERO(&m_ipcp_hisoptions, sizeof(ipcp_options));
}

int CPPPIPCP::Cilen(CPPPFsm *pfsm)
{
    ipcp_options *wo = &m_ipcp_wantoptions;
    ipcp_options *go = &m_ipcp_gotoptions;
    ipcp_options *ho = &m_ipcp_hisoptions;
    
    ACE_DEBUG ((LM_DEBUG,"CPPPIPCP::Cilen(\n"));


#define LENCIADDRS(neg)     (neg ? CILEN_ADDRS : 0)
#define LENCIVJ(neg, old)   (neg ? (old? CILEN_COMPRESS : CILEN_VJ) : 0)
#define LENCIADDR(neg)      (neg ? CILEN_ADDR : 0)
#define LENCIDNS(neg)       LENCIADDR(neg)
#define LENCIWINS(neg)      LENCIADDR(neg)

    /*
     * First see if we want to change our options to the old
     * forms because we have received old forms from the peer.
     */
    if (go->neg_addr && go->old_addrs && !ho->neg_addr && ho->old_addrs)
    go->neg_addr = 0;
    if (wo->neg_vj && !go->neg_vj && !go->old_vj) {
    /* try an older style of VJ negotiation */
    /* use the old style only if the peer did */
    if (ho->neg_vj && ho->old_vj) {
        go->neg_vj = 1;
        go->old_vj = 1;
        go->vj_protocol = ho->vj_protocol;
    }
    }

    return (LENCIADDRS(!go->neg_addr && go->old_addrs) +
        LENCIVJ(go->neg_vj, go->old_vj) +
        LENCIADDR(go->neg_addr) +
        LENCIDNS(go->req_dns1) +
        LENCIDNS(go->req_dns2) +
        LENCIWINS(go->winsaddr[0]) +
        LENCIWINS(go->winsaddr[1])) ;

}

void CPPPIPCP::Addci(CPPPFsm *pfsm, u_char *ucp, int *lenp)
{
    ipcp_options *go = &m_ipcp_gotoptions;
    int len = *lenp;
    
    ACE_DEBUG ((LM_DEBUG,"CPPPIPCP::Addci\n"));

#define ADDCIADDRS(opt, neg, val1, val2) \
    if (neg) { \
    if (len >= CILEN_ADDRS) { \
        u_int32_t l; \
        PUTCHAR(opt, ucp); \
        PUTCHAR(CILEN_ADDRS, ucp); \
        l = ntohl(val1); \
        PUTLONG(l, ucp); \
        l = ntohl(val2); \
        PUTLONG(l, ucp); \
        len -= CILEN_ADDRS; \
    } else \
        go->old_addrs = 0; \
    }

#define ADDCIVJ(opt, neg, val, old, maxslotindex, cflag) \
    if (neg) { \
    int vjlen = old? CILEN_COMPRESS : CILEN_VJ; \
    if (len >= vjlen) { \
        PUTCHAR(opt, ucp); \
        PUTCHAR(vjlen, ucp); \
        PUTSHORT(val, ucp); \
        if (!old) { \
        PUTCHAR(maxslotindex, ucp); \
        PUTCHAR(cflag, ucp); \
        } \
        len -= vjlen; \
    } else \
        neg = 0; \
    }

#define ADDCIADDR(opt, neg, val) \
    if (neg) { \
    if (len >= CILEN_ADDR) { \
        u_int32_t l; \
        PUTCHAR(opt, ucp); \
        PUTCHAR(CILEN_ADDR, ucp); \
        l = ntohl(val); \
        PUTLONG(l, ucp); \
        len -= CILEN_ADDR; \
    } else \
        neg = 0; \
    }

#define ADDCIDNS(opt, neg, addr) \
    if (neg) { \
    if (len >= CILEN_ADDR) { \
        u_int32_t l; \
        PUTCHAR(opt, ucp); \
        PUTCHAR(CILEN_ADDR, ucp); \
        l = ntohl(addr); \
        PUTLONG(l, ucp); \
        len -= CILEN_ADDR; \
    } else \
        neg = 0; \
    }

#define ADDCIWINS(opt, addr) \
    if (addr) { \
    if (len >= CILEN_ADDR) { \
        u_int32_t l; \
        PUTCHAR(opt, ucp); \
        PUTCHAR(CILEN_ADDR, ucp); \
        l = ntohl(addr); \
        PUTLONG(l, ucp); \
        len -= CILEN_ADDR; \
    } else \
        addr = 0; \
    }

    ADDCIADDRS(CI_ADDRS, !go->neg_addr && go->old_addrs, go->ouraddr,
           go->hisaddr);

    ADDCIVJ(CI_COMPRESSTYPE, go->neg_vj, go->vj_protocol, go->old_vj,
        go->maxslotindex, go->cflag);

    ADDCIADDR(CI_ADDR, go->neg_addr, go->ouraddr);

    ADDCIDNS(CI_MS_DNS1, go->req_dns1, go->dnsaddr[0]);

    ADDCIDNS(CI_MS_DNS2, go->req_dns2, go->dnsaddr[1]);

    ADDCIWINS(CI_MS_WINS1, go->winsaddr[0]);

    ADDCIWINS(CI_MS_WINS2, go->winsaddr[1]);
    
    *lenp -= len;
}

int CPPPIPCP::Ackci(CPPPFsm *pfsm, u_char *p, int len)
{
    ipcp_options *go = &m_ipcp_gotoptions;
    u_short cilen, citype, cishort;
    uint32_t cilong;
    u_char cimaxslotindex, cicflag;
    
    ACE_DEBUG((LM_DEBUG, "CPPPIPCP::Ackci, len=%d\n", len));

    /*
     * CIs must be in exactly the same order that we sent...
     * Check packet length and CI length at each step.
     * If we find any deviations, then this packet is bad.
     */

#define ACKCIADDRS(opt, neg, val1, val2) \
    if (neg) { \
    u_int32_t l; \
    if ((len -= CILEN_ADDRS) < 0) \
        goto bad; \
    GETCHAR(citype, p); \
    GETCHAR(cilen, p); \
    if (cilen != CILEN_ADDRS || \
        citype != opt) \
        goto bad; \
    GETLONG(l, p); \
    cilong = htonl(l); \
    if (val1 != cilong) \
        goto bad; \
    GETLONG(l, p); \
    cilong = htonl(l); \
    if (val2 != cilong) \
        goto bad; \
    }

#define ACKCIVJ(opt, neg, val, old, maxslotindex, cflag) \
    if (neg) { \
    int vjlen = old? CILEN_COMPRESS : CILEN_VJ; \
    if ((len -= vjlen) < 0) \
        goto bad; \
    GETCHAR(citype, p); \
    GETCHAR(cilen, p); \
    if (cilen != vjlen || \
        citype != opt)  \
        goto bad; \
    GETSHORT(cishort, p); \
    if (cishort != val) \
        goto bad; \
    if (!old) { \
        GETCHAR(cimaxslotindex, p); \
        if (cimaxslotindex != maxslotindex) \
        goto bad; \
        GETCHAR(cicflag, p); \
        if (cicflag != cflag) \
        goto bad; \
    } \
    }

#define ACKCIADDR(opt, neg, val) \
    if (neg) { \
    u_int32_t l; \
    if ((len -= CILEN_ADDR) < 0) \
        goto bad; \
    GETCHAR(citype, p); \
    GETCHAR(cilen, p); \
    if (cilen != CILEN_ADDR || \
        citype != opt) \
        goto bad; \
    GETLONG(l, p); \
    cilong = htonl(l); \
    if (val != cilong) \
        goto bad; \
    }

#define ACKCIDNS(opt, neg, addr) \
    if (neg) { \
    u_int32_t l; \
    if ((len -= CILEN_ADDR) < 0) \
        goto bad; \
    GETCHAR(citype, p); \
    GETCHAR(cilen, p); \
    if (cilen != CILEN_ADDR || citype != opt) \
        goto bad; \
    GETLONG(l, p); \
    cilong = htonl(l); \
    if (addr != cilong) \
        goto bad; \
    }

#define ACKCIWINS(opt, addr) \
    if (addr) { \
    u_int32_t l; \
    if ((len -= CILEN_ADDR) < 0) \
        goto bad; \
    GETCHAR(citype, p); \
    GETCHAR(cilen, p); \
    if (cilen != CILEN_ADDR || citype != opt) \
        goto bad; \
    GETLONG(l, p); \
    cilong = htonl(l); \
    if (addr != cilong) \
        goto bad; \
    }

    ACKCIADDRS(CI_ADDRS, !go->neg_addr && go->old_addrs, go->ouraddr,
           go->hisaddr);

    ACKCIVJ(CI_COMPRESSTYPE, go->neg_vj, go->vj_protocol, go->old_vj,
        go->maxslotindex, go->cflag);

    ACKCIADDR(CI_ADDR, go->neg_addr, go->ouraddr);

    ACKCIDNS(CI_MS_DNS1, go->req_dns1, go->dnsaddr[0]);

    ACKCIDNS(CI_MS_DNS2, go->req_dns2, go->dnsaddr[1]);

    ACKCIWINS(CI_MS_WINS1, go->winsaddr[0]);

    ACKCIWINS(CI_MS_WINS2, go->winsaddr[1]);

    /*
     * If there are any remaining CIs, then this packet is bad.
     */
    if (len != 0)
    goto bad;
    return (1);

bad:
    ACE_DEBUG ((LM_DEBUG,"CPPPIPCP::Ackci, received bad Ack!"));
    return (0);
}

int CPPPIPCP::Nakci(CPPPFsm *pfsm, u_char *p, int len, int treat_as_reject)
{
    ipcp_options *go = &m_ipcp_gotoptions;
    u_char cimaxslotindex, cicflag;
    u_char citype, cilen, *next;
    u_short cishort;
    uint32_t ciaddr1, ciaddr2, l, cidnsaddr;
    ipcp_options no;        /* options we've seen Naks for */
    ipcp_options tryop;       /* options to request next time */

    BZERO(&no, sizeof(no));
    tryop = *go;
    
    ACE_DEBUG((LM_DEBUG,"CPPPIPCP::Nakci, len=%d, treat_as_reject=%d\n", len, treat_as_reject));

    /*
     * Any Nak'd CIs must be in exactly the same order that we sent.
     * Check packet length and CI length at each step.
     * If we find any deviations, then this packet is bad.
     */
#define NAKCIADDRS(opt, neg, code) \
    if ((neg) && \
        (cilen = p[1]) == CILEN_ADDRS && \
        len >= cilen && \
        p[0] == opt) { \
        len -= cilen; \
        INCPTR(2, p); \
        GETLONG(l, p); \
        ciaddr1 = htonl(l); \
        GETLONG(l, p); \
        ciaddr2 = htonl(l); \
        no.old_addrs = 1; \
        code \
    }
    
#define NAKCIVJ(opt, neg, code) \
    if (go->neg && \
        ((cilen = p[1]) == CILEN_COMPRESS || cilen == CILEN_VJ) && \
        len >= cilen && \
        p[0] == opt) { \
        len -= cilen; \
        INCPTR(2, p); \
        GETSHORT(cishort, p); \
        no.neg = 1; \
            code \
    }
    
#define NAKCIADDR(opt, neg, code) \
    if (go->neg && \
        (cilen = p[1]) == CILEN_ADDR && \
        len >= cilen && \
        p[0] == opt) { \
        len -= cilen; \
        INCPTR(2, p); \
        GETLONG(l, p); \
        ciaddr1 = htonl(l); \
        no.neg = 1; \
        code \
    }
    
#define NAKCIDNS(opt, neg, code) \
    if (go->neg && \
        ((cilen = p[1]) == CILEN_ADDR) && \
        len >= cilen && \
        p[0] == opt) { \
        len -= cilen; \
        INCPTR(2, p); \
        GETLONG(l, p); \
        cidnsaddr = htonl(l); \
        no.neg = 1; \
        code \
    }
    
    /*
     * Accept the peer's idea of {our,his} address, if different
     * from our idea, only if the accept_{local,remote} flag is set.
     */
    NAKCIADDRS(CI_ADDRS, 
               !go->neg_addr && go->old_addrs,
               if (treat_as_reject) {
                   tryop.old_addrs = 0;
               } else {
                   if (go->accept_local && ciaddr1) {
                       /* take his idea of our address */
                       tryop.ouraddr = ciaddr1;
                   }
                   if (go->accept_remote && ciaddr2) {
                       /* take his idea of his address */
                       tryop.hisaddr = ciaddr2;
                   }
               }
               );

    /*
     * Accept the peer's value of maxslotindex provided that it
     * is less than what we asked for.  Turn off slot-ID compression
     * if the peer wants.  Send old-style compress-type option if
     * the peer wants.
     */
    NAKCIVJ(CI_COMPRESSTYPE, 
            neg_vj,
            if (treat_as_reject) {
                tryop.neg_vj = 0;
            } else if (cilen == CILEN_VJ) {
                GETCHAR(cimaxslotindex, p);
                GETCHAR(cicflag, p);
                if (cishort == IPCP_VJ_COMP) {
                    tryop.old_vj = 0;
                    if (cimaxslotindex < go->maxslotindex)
                    tryop.maxslotindex = cimaxslotindex;
                    if (!cicflag)
                    tryop.cflag = 0;
                } else {
                    tryop.neg_vj = 0;
                }
            } else {
                if (cishort == IPCP_VJ_COMP || cishort == IPCP_VJ_COMP_OLD) {
                    tryop.old_vj = 1;
                    tryop.vj_protocol = cishort;
                } else {
                    tryop.neg_vj = 0;
                }
            }
            );

    NAKCIADDR(CI_ADDR, 
              neg_addr,
              if (treat_as_reject) {
                  tryop.neg_addr = 0;
                  tryop.old_addrs = 0;
              } else if (go->accept_local && ciaddr1) {
                  /* take his idea of our address */
                  tryop.ouraddr = ciaddr1;
              }
              );

    NAKCIDNS(CI_MS_DNS1, 
             req_dns1,
             if (treat_as_reject) {
                 tryop.req_dns1 = 0;
             } else {
                 tryop.dnsaddr[0] = cidnsaddr;
             }
             );

    NAKCIDNS(CI_MS_DNS2, 
             req_dns2,
             if (treat_as_reject) {
                 tryop.req_dns2 = 0;
             } else {
                 tryop.dnsaddr[1] = cidnsaddr;
             }
             );

    /*
     * There may be remaining CIs, if the peer is requesting negotiation
     * on an option that we didn't include in our request packet.
     * If they want to negotiate about IP addresses, we comply.
     * If they want us to ask for compression, we refuse.
     * If they want us to ask for ms-dns, we do that, since some
     * peers get huffy if we don't.
     */
    while (len >= CILEN_VOID) {
        GETCHAR(citype, p);
        GETCHAR(cilen, p);
        if ( cilen < CILEN_VOID || (len -= cilen) < 0 )
            goto bad;
        next = p + cilen - 2;

        switch (citype) {
            case CI_COMPRESSTYPE:
                if (go->neg_vj || no.neg_vj || (cilen != CILEN_VJ && cilen != CILEN_COMPRESS))
                {
                    goto bad;
                }
                no.neg_vj = 1;
                break;
            case CI_ADDRS:
                if ((!go->neg_addr && go->old_addrs) || no.old_addrs || cilen != CILEN_ADDRS)
                {
                    goto bad;
                }
                tryop.neg_addr = 0;
                GETLONG(l, p);
                ciaddr1 = htonl(l);
                if (ciaddr1 && go->accept_local)
                {
                    tryop.ouraddr = ciaddr1;
                }
                GETLONG(l, p);
                ciaddr2 = htonl(l);
                if (ciaddr2 && go->accept_remote)
                {
                    tryop.hisaddr = ciaddr2;
                }
                no.old_addrs = 1;
                break;
            case CI_ADDR:
                if (go->neg_addr || no.neg_addr || cilen != CILEN_ADDR)
                {
                    goto bad;
                }
                tryop.old_addrs = 0;
                GETLONG(l, p);
                ciaddr1 = htonl(l);
                if (ciaddr1 && go->accept_local)
                {
                    tryop.ouraddr = ciaddr1;
                }
                if (tryop.ouraddr != 0)
                {
                    tryop.neg_addr = 1;
                }
                no.neg_addr = 1;
                break;
            case CI_MS_DNS1:
                if (go->req_dns1 || no.req_dns1 || cilen != CILEN_ADDR)
                {
                    goto bad;
                }
                GETLONG(l, p);
                tryop.dnsaddr[0] = htonl(l);
                tryop.req_dns1 = 1;
                no.req_dns1 = 1;
                break;
            case CI_MS_DNS2:
                if (go->req_dns2 || no.req_dns2 || cilen != CILEN_ADDR)
                {
                    goto bad;
                }
                GETLONG(l, p);
                tryop.dnsaddr[1] = htonl(l);
                tryop.req_dns2 = 1;
                no.req_dns2 = 1;
                break;
            case CI_MS_WINS1:
            case CI_MS_WINS2:
                if (cilen != CILEN_ADDR)
                {
                    goto bad;
                }
                GETLONG(l, p);
                ciaddr1 = htonl(l);
                if (ciaddr1)
                {
                    tryop.winsaddr[citype == CI_MS_WINS2] = ciaddr1;
                }
                break;
        } // switch (citype)
        p = next;
    } // while (len >= CILEN_VOID)

    /*
     * OK, the Nak is good.  Now we can update state.
     * If there are any remaining options, we ignore them.
     */
    if (State() != OPENED)
    {
        *go = tryop;
    }

    return 1;
        
bad:
    ACE_DEBUG ((LM_DEBUG,"CPPPIPCP::Nakci, received bad Nak!\n"));
    return 0;
}

int CPPPIPCP::Rejci(CPPPFsm *pfsm, u_char *p, int len)
{
    ipcp_options *go = &m_ipcp_gotoptions;
    u_char cimaxslotindex, ciflag, cilen;
    u_short cishort;
    uint32_t cilong;
    ipcp_options tryop;       /* options to request next time */
    
    ACE_DEBUG ((LM_DEBUG, "CPPPIPCP::Rejci, len=%d\n", len));

    tryop = *go;
    /*
     * Any Rejected CIs must be in exactly the same order that we sent.
     * Check packet length and CI length at each step.
     * If we find any deviations, then this packet is bad.
     */
#define REJCIADDRS(opt, neg, val1, val2) \
    if ((neg) && \
        (cilen = p[1]) == CILEN_ADDRS && \
        len >= cilen && \
        p[0] == opt) { \
        u_int32_t l; \
        len -= cilen; \
        INCPTR(2, p); \
        GETLONG(l, p); \
        cilong = htonl(l); \
        /* Check rejected value. */ \
        if (cilong != val1) \
            goto bad; \
        GETLONG(l, p); \
        cilong = htonl(l); \
        /* Check rejected value. */ \
        if (cilong != val2) \
            goto bad; \
        tryop.old_addrs = 0; \
    }

#define REJCIVJ(opt, neg, val, old, maxslot, cflag) \
    if (go->neg && \
        p[1] == (old? CILEN_COMPRESS : CILEN_VJ) && \
        len >= p[1] && \
        p[0] == opt) { \
        len -= p[1]; \
        INCPTR(2, p); \
        GETSHORT(cishort, p); \
        /* Check rejected value. */  \
        if (cishort != val) \
            goto bad; \
        if (!old) { \
           GETCHAR(cimaxslotindex, p); \
           if (cimaxslotindex != maxslot) \
             goto bad; \
           GETCHAR(ciflag, p); \
           if (ciflag != cflag) \
             goto bad; \
        } \
        tryop.neg = 0; \
    }

#define REJCIADDR(opt, neg, val) \
    if (go->neg && \
        (cilen = p[1]) == CILEN_ADDR && \
        len >= cilen && \
        p[0] == opt) { \
        u_int32_t l; \
        len -= cilen; \
        INCPTR(2, p); \
        GETLONG(l, p); \
        cilong = htonl(l); \
        /* Check rejected value. */ \
        if (cilong != val) \
            goto bad; \
        tryop.neg = 0; \
    }

#define REJCIDNS(opt, neg, dnsaddr) \
    if (go->neg && \
        ((cilen = p[1]) == CILEN_ADDR) && \
        len >= cilen && \
        p[0] == opt) { \
        u_int32_t l; \
        len -= cilen; \
        INCPTR(2, p); \
        GETLONG(l, p); \
        cilong = htonl(l); \
        /* Check rejected value. */ \
        if (cilong != dnsaddr) \
            goto bad; \
        tryop.neg = 0; \
    }

#define REJCIWINS(opt, addr) \
    if (addr && \
        ((cilen = p[1]) == CILEN_ADDR) && \
        len >= cilen && \
        p[0] == opt) { \
        u_int32_t l; \
        len -= cilen; \
        INCPTR(2, p); \
        GETLONG(l, p); \
        cilong = htonl(l); \
        /* Check rejected value. */ \
        if (cilong != addr) \
            goto bad; \
        tryop.winsaddr[opt == CI_MS_WINS2] = 0; \
    }

    REJCIADDRS(CI_ADDRS, !go->neg_addr && go->old_addrs,
           go->ouraddr, go->hisaddr);

    REJCIVJ(CI_COMPRESSTYPE, neg_vj, go->vj_protocol, go->old_vj,
        go->maxslotindex, go->cflag);

    REJCIADDR(CI_ADDR, neg_addr, go->ouraddr);

    REJCIDNS(CI_MS_DNS1, req_dns1, go->dnsaddr[0]);

    REJCIDNS(CI_MS_DNS2, req_dns2, go->dnsaddr[1]);

    REJCIWINS(CI_MS_WINS1, go->winsaddr[0]);

    REJCIWINS(CI_MS_WINS2, go->winsaddr[1]);

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
    {
        *go = tryop;
    }
    
    return 1;

bad:
    ACE_DEBUG((LM_DEBUG,"CPPPIPCP::Rejci, received bad Reject!"));
    return 0;
}

int CPPPIPCP::Reqci(CPPPFsm *pfsm, u_char *inp, int *len, int reject_if_disagree)
{
    ipcp_options *wo = &m_ipcp_wantoptions;
    ipcp_options *ho = &m_ipcp_hisoptions;
    ipcp_options *ao = &m_ipcp_allowoptions;

    u_char *cip, *next;     /* Pointer to current and next CIs */
    u_short cilen, citype;  /* Parsed len, type */
    u_short cishort;        /* Parsed short value */
    uint32_t tl, ciaddr1, ciaddr2;/* Parsed address values */
    int rc = CONFACK;       /* Final packet return code */
    int orc;            /* Individual option return code */
    u_char *p;          /* Pointer to next char to parse */
    u_char *ucp = inp;  /* Pointer to current output char */
    int l = *len;       /* Length left */
    u_char maxslotindex, cflag;
    int d;
    
    ACE_DEBUG ((LM_DEBUG, "CPPPIPCP::Reqci, reject_if_disagree=%d\n", reject_if_disagree));

    /*
     * Reset all his options.
     */
    BZERO(ho, sizeof(*ho));
        
    /*
     * Process all his options.
     */
    next = inp;
    while (l) {
        orc = CONFACK;          /* Assume success */
        cip = p = next;         /* Remember begining of CI */
        
        if (l < 2 ||            /* Not enough data for CI header or */
            p[1] < 2 ||         /*  CI length too small or */
            p[1] > l) {         /*  CI length too big? */
            
            ACE_DEBUG ((LM_DEBUG, "CPPPIPCP::Reqci, bad CI length! l=%d, p[1]=%u\n", l, p[1]));
            
            orc = CONFREJ;      /* Reject bad CI */
            cilen = l;          /* Reject till end of packet */
            l = 0;              /* Don't loop again */
            goto endswitch;
        }
        
        GETCHAR(citype, p);     /* Parse CI type */
        GETCHAR(cilen, p);      /* Parse CI length */
        l -= cilen;             /* Adjust remaining length */
        next += cilen;          /* Step to next CI */

        switch (citype) {       /* Check CI type */
            case CI_ADDRS:
                if (!ao->old_addrs || ho->neg_addr ||
                    cilen != CILEN_ADDRS) { /* Check CI length */
                    orc = CONFREJ;          /* Reject CI */
                    break;
                }

                /*
                 * If he has no address, or if we both have his address but
                 * disagree about it, then NAK it with our idea.
                 * In particular, if we don't know his address, but he does,
                 * then accept it.
                 */
                GETLONG(tl, p);     /* Parse source address (his) */
                ciaddr1 = htonl(tl);
                if (ciaddr1 != wo->hisaddr && (ciaddr1 == 0 || !wo->accept_remote)) {
                    orc = CONFNAK;
                    if (!reject_if_disagree) {
                        DECPTR(sizeof(u_int32_t), p);
                        tl = ntohl(wo->hisaddr);
                        PUTLONG(tl, p);
                    }
                } else if (ciaddr1 == 0 && wo->hisaddr == 0) {
                    /*
                     * If neither we nor he knows his address, reject the option.
                     */
                    orc = CONFREJ;
                    wo->req_addr = 0;   /* don't NAK with 0.0.0.0 later */
                    break;
                }

                /*
                 * If he doesn't know our address, or if we both have our address
                 * but disagree about it, then NAK it with our idea.
                 */
                GETLONG(tl, p);     /* Parse desination address (ours) */
                ciaddr2 = htonl(tl);
                if (ciaddr2 != wo->ouraddr) {
                    if (ciaddr2 == 0 || !wo->accept_local) {
                        orc = CONFNAK;
                        if (!reject_if_disagree) {
                            DECPTR(sizeof(u_int32_t), p);
                            tl = ntohl(wo->ouraddr);
                            PUTLONG(tl, p);
                        }
                    } else {
                        wo->ouraddr = ciaddr2;  /* accept peer's idea */
                    }
                }

                ho->old_addrs = 1;
                ho->hisaddr = ciaddr1;
                ho->ouraddr = ciaddr2;
                break;

            case CI_ADDR:
                if (!ao->neg_addr || ho->old_addrs || cilen != CILEN_ADDR) {  /* Check CI length */
                    orc = CONFREJ;      /* Reject CI */
                    break;
                }

                /*
                 * If he has no address, or if we both have his address but
                 * disagree about it, then NAK it with our idea.
                 * In particular, if we don't know his address, but he does,
                 * then accept it.
                 */
                GETLONG(tl, p); /* Parse source address (his) */
                ciaddr1 = htonl(tl);
                if (ciaddr1 != wo->hisaddr && (ciaddr1 == 0 || !wo->accept_remote)) {
                    orc = CONFNAK;
                    if (!reject_if_disagree) {
                        DECPTR(sizeof(u_int32_t), p);
                        tl = ntohl(wo->hisaddr);
                        PUTLONG(tl, p);
                    }
                } else if (ciaddr1 == 0 && wo->hisaddr == 0) {
                    /*
                     * Don't ACK an address of 0.0.0.0 - reject it instead.
                     */
                    orc = CONFREJ;
                    wo->req_addr = 0;   /* don't NAK with 0.0.0.0 later */
                    break;
                }
            
                ho->neg_addr = 1;
                ho->hisaddr = ciaddr1;
                break;

            case CI_MS_DNS1:
            case CI_MS_DNS2:
                /* Microsoft primary or secondary DNS request */
                d = citype == CI_MS_DNS2;

                /* If we do not have a DNS address then we cannot send it */
                if (ao->dnsaddr[d] == 0 || cilen != CILEN_ADDR) {  /* Check CI length */
                    orc = CONFREJ;      /* Reject CI */
                    break;
                }
                GETLONG(tl, p);
                if (htonl(tl) != ao->dnsaddr[d]) {
                    DECPTR(sizeof(u_int32_t), p);
                    tl = ntohl(ao->dnsaddr[d]);
                    PUTLONG(tl, p);
                    orc = CONFNAK;
                }
                break;

            case CI_MS_WINS1:
            case CI_MS_WINS2:
                /* Microsoft primary or secondary WINS request */
                d = citype == CI_MS_WINS2;

                /* If we do not have a DNS address then we cannot send it */
                if (ao->winsaddr[d] == 0 || cilen != CILEN_ADDR) {  /* Check CI length */
                    orc = CONFREJ;      /* Reject CI */
                    break;
                }
                GETLONG(tl, p);
                if (htonl(tl) != ao->winsaddr[d]) {
                    DECPTR(sizeof(u_int32_t), p);
                    tl = ntohl(ao->winsaddr[d]);
                    PUTLONG(tl, p);
                    orc = CONFNAK;
                }
                break;
            
            case CI_COMPRESSTYPE:
                if (!ao->neg_vj || (cilen != CILEN_VJ && cilen != CILEN_COMPRESS)) {
                    orc = CONFREJ;
                    break;
                }
                GETSHORT(cishort, p);

                if (!(cishort == IPCP_VJ_COMP || (cishort == IPCP_VJ_COMP_OLD && cilen == CILEN_COMPRESS))) {
                    orc = CONFREJ;
                    break;
                }

                ho->neg_vj = 1;
                ho->vj_protocol = cishort;
                if (cilen == CILEN_VJ) {
                    GETCHAR(maxslotindex, p);
                    if (maxslotindex > ao->maxslotindex) { 
                        orc = CONFNAK;
                        if (!reject_if_disagree) {
                            DECPTR(1, p);
                            PUTCHAR(ao->maxslotindex, p);
                        }
                    }
                    GETCHAR(cflag, p);
                    if (cflag && !ao->cflag) {
                        orc = CONFNAK;
                        if (!reject_if_disagree) {
                            DECPTR(1, p);
                            PUTCHAR(wo->cflag, p);
                        }
                    }
                    ho->maxslotindex = maxslotindex;
                    ho->cflag = cflag;
                } else {
                    ho->old_vj = 1;
                    ho->maxslotindex = MAX_STATES - 1;
                    ho->cflag = 1;
                } // if (cilen == CILEN_VJ) 
                break;

            default:
                orc = CONFREJ;
                break;
        } // switch (citype)
        
endswitch:
    if (orc == CONFACK &&       /* Good CI */
        rc != CONFACK)      /*  but prior CI wasnt? */
        continue;           /* Don't send this one */

    if (orc == CONFNAK) {       /* Nak this CI? */
        if (reject_if_disagree) /* Getting fed up with sending NAKs? */
            orc = CONFREJ;      /* Get tough if so */
        else {
            if (rc == CONFREJ)  /* Rejecting prior CI? */
                continue;       /* Don't send this one */
            if (rc == CONFACK) {    /* Ack'd all prior CIs? */
                rc = CONFNAK;   /* Not anymore... */
                ucp = inp;      /* Backup */
            }
        }
    }

    if (orc == CONFREJ &&       /* Reject this CI */
        rc != CONFREJ) {        /*  but no prior ones? */
        rc = CONFREJ;
        ucp = inp;          /* Backup */
    }

    /* Need to move CI? */
    if (ucp != cip)
        BCOPY(cip, ucp, cilen); /* Move it */

    /* Update output pointer */
    INCPTR(cilen, ucp);
    } // while (l)
    
    /*
     * If we aren't rejecting this packet, and we want to negotiate
     * their address, and they didn't send their address, then we
     * send a NAK with a CI_ADDR option appended.  We assume the
     * input buffer is long enough that we can append the extra
     * option safely.
     */
    if (rc != CONFREJ && !ho->neg_addr && !ho->old_addrs &&
        wo->req_addr && !reject_if_disagree && !m_noremoteip) {
        if (rc == CONFACK) {
            rc = CONFNAK;
            ucp = inp;          /* reset pointer */
            wo->req_addr = 0;   /* don't ask again */
        }
        PUTCHAR(CI_ADDR, ucp);
        PUTCHAR(CILEN_ADDR, ucp);
        tl = ntohl(wo->hisaddr);
        PUTLONG(tl, ucp);
    }

    *len = ucp - inp;           /* Compute output length */
    ACE_DEBUG ((LM_DEBUG,"CPPPIPCP::Reqci, returning Configure-%s", CODENAME(rc)));
    return (rc);            /* Return final code */
}

void CPPPIPCP::Up(CPPPFsm *pfsm)
{
    ipcp_options *ho = &m_ipcp_hisoptions;
    ipcp_options *go = &m_ipcp_gotoptions;
    ipcp_options *wo = &m_ipcp_wantoptions;

    ACE_DEBUG ((LM_DEBUG, "CPPPIPCP::Up\n"));

    /*
     * We must have a non-zero IP address for both ends of the link.
     */
    if (!ho->neg_addr && !ho->old_addrs)
    {
        ho->hisaddr = wo->hisaddr;
    }

    if (!(go->neg_addr || go->old_addrs) 
        && (wo->neg_addr || wo->old_addrs)
        && wo->ouraddr != 0) 
    {
        ACE_DEBUG((LM_DEBUG, "Refused our IP address"));
        Close("Refused our IP address");
        
        return;
    }
    
    if (go->ouraddr == 0) 
    {
        ACE_DEBUG((LM_DEBUG, "Could not determine local IP address"));
        Close("Could not determine local IP address");
        
        return;
    }

    if (m_psink)
    {
        m_psink->OnIPCPUp();
    }
}

void CPPPIPCP::Down(CPPPFsm *pfsm)
{
    ACE_DEBUG((LM_DEBUG, "CPPPIPCP::Down\n"));
    
    /* XXX a bit IPv4-centric here, we only need to get the stats
     * before the interface is marked down. */
    /* XXX more correct: we must get the stats before running the notifiers,
     * at least for the radius plugin */

    if (m_psink)
    {
        m_psink->OnIPCPDown();
    }
}

void CPPPIPCP::Starting(CPPPFsm *pfsm)
{
    LowerUp();
}

void CPPPIPCP::Finished(CPPPFsm *pfsm)
{
    ACE_DEBUG ((LM_DEBUG, "CPPPIPCP::Finished\n"));
    m_ipcp_is_open = false;
}

void CPPPIPCP::ProtReject(int)
{
    ACE_DEBUG ((LM_DEBUG, "CPPPIPCP::ProtReject\n"));    
}

void CPPPIPCP::Retransmit(CPPPFsm *pfsm)
{
    ACE_DEBUG ((LM_DEBUG, "CPPPIPCP::Retransmit\n"));    
}

//Output Packet
void CPPPIPCP::OutputPacket(unsigned char *pkg, size_t size)
{
    ACE_DEBUG ((LM_DEBUG, "CPPPIPCP::OutputPacket, size=%u\n", size));

    if (m_psink)
    {
        m_psink->OnIPCPOutput(pkg,size);
    }
}

int CPPPIPCP::ExtCode(CPPPFsm *pfsm, int, int, u_char *, int)
{
    ACE_DEBUG ((LM_DEBUG, "CPPPIPCP::ExtCode\n"));    
    
    return 0;
}

//check fsm status
int CPPPIPCP::State()
{
    return m_fsm.State();
}

//Send Payload
void CPPPIPCP::SendPayload(unsigned char *pkg, size_t size)
{
    unsigned char buffer[1024*4] = {0};
    unsigned char *outp = NULL;
    int outlen;

    /* Adjust length to be smaller than MTU */
    outp = buffer;
    if (size > PPP_MRU - HEADERLEN)
    {
        size = PPP_MRU - HEADERLEN;
    }
    if (size && pkg != outp + PPP_HDRLEN + HEADERLEN)
    {
        BCOPY(pkg, outp + PPP_HDRLEN + HEADERLEN, size);
    }
    outlen = size + HEADERLEN;
    MAKEHEADER(outp, 0x0021);
    PUTSHORT(outlen, outp);
    
    if (m_psink)
    {
        m_psink->OnIPCPOutput(outp,outlen + PPP_HDRLEN);
    }  
}

//Input Data to packet
void CPPPIPCP::InputData(unsigned char *packet, size_t size)
{
    if (m_psink)
    {
        m_psink->OnIPCPPayload(packet,size);
    }
}

/* Notice for arguments in the following SetXXXX functions:
 * IP/dns (WORD32) -- IP address in network byte order
 */
void CPPPIPCP::SetMyIP(WORD32 myIP)
{
    m_ipcp_wantoptions.ouraddr = myIP;
}

void CPPPIPCP::SetMyIP(std::string &myIP)
{
    std::string ip = myIP;
    ip += ":0";
    
    ACE_INET_Addr addr(ip.c_str());
    SetMyIP( htonl(addr.get_ip_address()) );
}

//Set Subscriber Ip
void CPPPIPCP::SetSubscriberIP(WORD32 hisIP)
{
    m_ipcp_wantoptions.hisaddr = hisIP;
}

void CPPPIPCP::SetSubscriberIP(std::string &hisIP)
{
    std::string ip = hisIP;
    ip += ":0";
    
    ACE_INET_Addr addr2(ip.c_str());
    SetSubscriberIP( htonl(addr2.get_ip_address()) );
}

//Set Primary DNS
void CPPPIPCP::SetPrimaryDNS(WORD32 dns)
{
    m_ipcp_allowoptions.dnsaddr[0] = dns;
}

void CPPPIPCP::SetPrimaryDNS(std::string &dns)
{
    std::string tmp = dns;
    tmp += ":0";
    
    ACE_INET_Addr dnsAddr1(tmp.c_str());
    SetPrimaryDNS( htonl(dnsAddr1.get_ip_address()) );
}

//Set Secondary DNS
void CPPPIPCP::SetSecondaryDNS(WORD32 dns)
{
    m_ipcp_allowoptions.dnsaddr[1] = dns;
}

void CPPPIPCP::SetSecondaryDNS(std::string &dns)
{
    std::string tmp = dns;
    tmp += ":0";
    
    ACE_INET_Addr dnsAddr2(tmp.c_str());
    SetSecondaryDNS( htonl(dnsAddr2.get_ip_address()) );
}

//Get Subscriber IP
WORD32 CPPPIPCP::GetSubscriberIP()
{
    return m_ipcp_wantoptions.hisaddr;
}

//Get Primary DNS
WORD32 CPPPIPCP::GetPrimaryDNS()
{
    return m_ipcp_allowoptions.dnsaddr[0];
}

//Get Secondary DNS
WORD32 CPPPIPCP::GetSecondaryDNS()
{
    return m_ipcp_allowoptions.dnsaddr[1];
}


