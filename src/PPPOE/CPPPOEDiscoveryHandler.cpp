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


#include "CPPPOEDiscoveryHandler.h"
#include "CPPPOE.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "openssl/md5.h"
#include "CSession.h"

PPPoETag CPPPOEDiscoveryHandler::hostUniq;
PPPoETag CPPPOEDiscoveryHandler::relayId;
PPPoETag CPPPOEDiscoveryHandler::receivedCookie;
PPPoETag CPPPOEDiscoveryHandler::requestedService;
/* Requested max_ppp_payload */
WORD16 CPPPOEDiscoveryHandler::max_ppp_payload = 0;

CPPPOEDiscoveryHandler::CPPPOEDiscoveryHandler(CPPPOE &pppoe)
    : m_pppoe(pppoe)
{
    ::memset(m_cookieSeed, 0, sizeof m_cookieSeed);
}

CPPPOEDiscoveryHandler::~CPPPOEDiscoveryHandler()
{
}

//provide PPPOE interface
CPPPOE &CPPPOEDiscoveryHandler::GetPppoe() 
{
    return m_pppoe;
}

SWORD32 CPPPOEDiscoveryHandler::Init()
{
    ACE_DEBUG ((LM_DEBUG, "CPPPOEDiscoveryHandler::Init\n"));
    
    SWORD32 ret = CPPPOEPacketHandler::OpenX(ETH_PPPOE_DISCOVERY);
    
    if (-1 == ret)
    {
        ACE_DEBUG((LM_ERROR, 
                   "CPPPOEDiscoveryHandler::Init(), CPPPOEPacketHandler::OpenX(ETH_PPPOE_DISCOVERY) failed. ret=%d\n", 
                   ret));
        return -1;
    }

    CHAR intfName[ETHER_INTF_NAME_SIZE+1] = {0};
    GetPppoe().GetEtherIntf().GetIntfName(intfName);

    ret = CPPPOEPacketHandler::BindInterface(intfName, ETH_PPPOE_DISCOVERY);
    if (-1 == ret)
    {
        ACE_DEBUG((LM_ERROR, 
                   "CPPPOEDiscoveryHandler::Init(), CPPPOEPacketHandler::BindInterface(ETH_PPPOE_DISCOVERY) failed. "
                   "intfName=%s ret=%d\n", 
                   intfName,
                   ret));
        return -1;
    }

    BNGPResult result = InitCookieSeed();
    if (BNGPLAT_FAILED(result))
    {
        ACE_DEBUG((LM_ERROR, "CPPPOEDiscoveryHandler::Init(), InitCookieSeed() failed. result=%l\n", result));
        return -1;
    }
    
    return 0;
}

//Init Cookies
BNGPResult CPPPOEDiscoveryHandler::InitCookieSeed()
{
    FILE *fp = NULL;

    /* Initialize our random cookie.  Try /dev/urandom; if that fails,
       use PID and rand() */
    fp = fopen("/dev/urandom", "r");
    if (fp) 
    {
    	WORD32 x;
    	fread(&x, 1, sizeof(x), fp);
    	srand(x);
    	fread(&m_cookieSeed, 1, SEED_LEN, fp);
    	fclose(fp);
    } 
    else 
    {
    	srand((WORD32) getpid() * (WORD32) time(NULL));
    	m_cookieSeed[0] = getpid() & 0xFF;
    	m_cookieSeed[1] = (getpid() >> 8) & 0xFF;
    	for (BYTE i=2; i<SEED_LEN; i++) 
        {
    	    m_cookieSeed[i] = (rand() >> (i % 9)) & 0xFF;
    	}
    }

    return BNGPLAT_OK;
}

SWORD32 CPPPOEDiscoveryHandler::ProcessPacket(const CHAR *packet, ssize_t size)
{
    if (NULL == packet)
    {
        ACE_DEBUG ((LM_ERROR,
                   "CPPPOEDiscoveryHandler::ProcessPacket(), packet NULL\n"));
    	return -1;
    }

    ACE_DEBUG((LM_INFO, "CPPPOEDiscoveryHandler::ProcessPacket(), size=%d\n", size));
    
    if (size < HDR_SIZE)
    {
    	/* Impossible - ignore */
        ACE_DEBUG ((LM_ERROR,
                   "CPPPOEDiscoveryHandler::ProcessPacket(), packet size too short(%d), less than HDR_SIZE(%u)\n",
                   size, 
                   HDR_SIZE));
    	return -1;
    }

    PPPoEPacket *pkt = (PPPoEPacket *)packet;
    
    /* Sanity check on packet */
    if (pkt->ver != 1 || pkt->type != 1)
    {
        ACE_DEBUG ((LM_ERROR,
                   "CPPPOEDiscoveryHandler::ProcessPacket(), packet version(%d) or type(%d) wrong.\n",
                   pkt->ver, 
                   pkt->type));
    	return -1;
    }

    /* Check length */
    if (ntohs(pkt->length) + HDR_SIZE > size) 
    {
        ACE_DEBUG ((LM_ERROR,
                   "CPPPOEDiscoveryHandler::ProcessPacket(), Bogus PPPoE length field (%u). size=%d\n",
                   (unsigned int) ntohs(pkt->length),
                   size));

    	return -1;
    }

    switch(pkt->code)
    {
        case CODE_PADI:
        {
        	processPADI(pkt, size);
        	break;
        }
        case CODE_PADR:
        {
        	processPADR(pkt, size);
        	break;
        }
        case CODE_PADT:
        {
        	processPADT(pkt, size);
        	break;
        }
        case CODE_SESS:
        {
        	/* Ignore SESS -- children will handle them */
        	break;
        }
        case CODE_PADO:
        case CODE_PADS:
        {
        	/* Ignore PADO and PADS totally */
        	break;
        }
        default:
        {
            ACE_DEBUG ((LM_ERROR,
                       "CPPPOEDiscoveryHandler::ProcessPacket(), unknown PPPoE type(%d)\n",
                       pkt->code));
        	break;
        }
    }

    return 0;
}

/**********************************************************************
*%FUNCTION: processPADI
*%ARGUMENTS:
* packet -- PPPoE PADI packet
* len -- length of received packet
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Sends a PADO packet back to client
***********************************************************************/
void CPPPOEDiscoveryHandler::processPADI(PPPoEPacket *packet, SWORD32 len)
{
    PPPoEPacket pado;
    
    PPPoETag acname;
    PPPoETag servname;
    PPPoETag cookie;
    size_t acname_len = 0;
    BYTE *cursor = pado.payload;
    WORD16 plen = 0;
    SWORD32 ok = 0;

    BYTE myAddr[ETH_ALEN] = {0};   
    GetPppoe().GetEtherIntf().GetIntfMac(myAddr);

    ACE_DEBUG((LM_DEBUG, "CPPPOEDiscoveryHandler::processPADI(), len=%d\n", len));

    /* Ignore PADI's which don't come from a unicast address */
    if (NOT_UNICAST(packet->ethHdr.h_source)) 
    {
        ACE_DEBUG ((LM_ERROR,
                   "CPPPOEDiscoveryHandler::processPADI(), PADI packet from non-unicast source address. "
                   "packet source: %02x:%02x:%02x:%02x:%02x:%02x\n",
                   packet->ethHdr.h_source[0], packet->ethHdr.h_source[1], packet->ethHdr.h_source[2],
                   packet->ethHdr.h_source[3], packet->ethHdr.h_source[4], packet->ethHdr.h_source[5]));
    	return;
    }

    acname.type = htons(TAG_AC_NAME);
    acname_len = strlen(GetPppoe().GetACName());
    acname.length = htons(acname_len);
    memcpy(acname.payload, GetPppoe().GetACName(), acname_len);

    relayId.type = 0;
    hostUniq.type = 0;
    requestedService.type = 0;
    max_ppp_payload = 0;

    parsePacket(packet, parsePADITags, NULL);

    /* If PADI specified non-default service name, and we do not offer
       that service, DO NOT send PADO */
    if (requestedService.type)
    {
    	WORD16 slen = ntohs(requestedService.length);
    	if (slen > 0)
        {
    		if ((slen == ::strlen(GetPppoe().GetSvcName()))
                && (0 == ::memcmp(GetPppoe().GetSvcName(), requestedService.payload, slen))) 
		    {
    		    ok = 1;
    		}
    	} 
        else 
        {
    	    ok = 1;		/* Default service requested */
    	}
    }
    else
    {
    	ok = 1;			/* No Service-Name tag in PADI */
    }

    if (!ok)
    {
    	/* PADI asked for unsupported service */
        ACE_DEBUG((LM_DEBUG, 
                  "CPPPOEDiscoveryHandler::processPADI(), PADI asked for unsupported service(%s)./n",
                  requestedService.payload));
    	return;
    }

    /* Generate a cookie */
    cookie.type = htons(TAG_AC_COOKIE);
    cookie.length = htons(COOKIE_LEN);
    genCookie(packet->ethHdr.h_source, myAddr, m_cookieSeed, cookie.payload);

    /* Construct a PADO packet */
    memcpy(pado.ethHdr.h_dest, packet->ethHdr.h_source, ETH_ALEN);
    memcpy(pado.ethHdr.h_source, myAddr, ETH_ALEN);
    pado.ethHdr.h_proto = htons(ETH_PPPOE_DISCOVERY);
    pado.ver = 1;
    pado.type = 1;
    pado.code = CODE_PADO;
    pado.session = 0;
    plen = TAG_HDR_SIZE + acname_len;

    CHECK_ROOM(cursor, pado.payload, acname_len+TAG_HDR_SIZE);
    memcpy(cursor, &acname, acname_len + TAG_HDR_SIZE);
    cursor += acname_len + TAG_HDR_SIZE;

    /* If we asked for an MTU, handle it */
    WORD16 intfMtu = GetPppoe().GetEtherIntf().GetIntfMtu();
    if (max_ppp_payload > ETH_PPPOE_MTU && intfMtu > 0) 
    {
    	/* Shrink payload to fit */
    	if (max_ppp_payload > intfMtu - TOTAL_OVERHEAD) 
        {
    	    max_ppp_payload = intfMtu - TOTAL_OVERHEAD;
    	}
    	if (max_ppp_payload > ETH_JUMBO_LEN - TOTAL_OVERHEAD)
        {
    	    max_ppp_payload = ETH_JUMBO_LEN - TOTAL_OVERHEAD;
    	}
    	if (max_ppp_payload > ETH_PPPOE_MTU) 
        {
    	    PPPoETag maxPayload;
    	    WORD16 mru = htons(max_ppp_payload);
    	    maxPayload.type = htons(TAG_PPP_MAX_PAYLOAD);
    	    maxPayload.length = htons(sizeof(mru));
    	    memcpy(maxPayload.payload, &mru, sizeof(mru));
    	    CHECK_ROOM(cursor, pado.payload, sizeof(mru) + TAG_HDR_SIZE);
    	    memcpy(cursor, &maxPayload, sizeof(mru) + TAG_HDR_SIZE);
    	    cursor += sizeof(mru) + TAG_HDR_SIZE;
    	    plen += sizeof(mru) + TAG_HDR_SIZE;
    	}
    }

    /* If no service-names specified on command-line, just send default
       zero-length name.  Otherwise, add all service-name tags */
    servname.type = htons(TAG_SERVICE_NAME);
    if (GetPppoe().GetSvcName() == NULL)
    {
    	servname.length = 0;
    	CHECK_ROOM(cursor, pado.payload, TAG_HDR_SIZE);
    	memcpy(cursor, &servname, TAG_HDR_SIZE);
    	cursor += TAG_HDR_SIZE;
    	plen += TAG_HDR_SIZE;
    }
    else 
    {
	    int slen = strlen(GetPppoe().GetSvcName());
	    servname.length = htons(slen);
	    CHECK_ROOM(cursor, pado.payload, TAG_HDR_SIZE+slen);
	    memcpy(cursor, &servname, TAG_HDR_SIZE);
	    memcpy(cursor+TAG_HDR_SIZE, GetPppoe().GetSvcName(), slen);
	    cursor += TAG_HDR_SIZE+slen;
	    plen += TAG_HDR_SIZE+slen;
    }

    CHECK_ROOM(cursor, pado.payload, TAG_HDR_SIZE + COOKIE_LEN);
    memcpy(cursor, &cookie, TAG_HDR_SIZE + COOKIE_LEN);
    cursor += TAG_HDR_SIZE + COOKIE_LEN;
    plen += TAG_HDR_SIZE + COOKIE_LEN;

    if (relayId.type)
    {
    	CHECK_ROOM(cursor, pado.payload, ntohs(relayId.length) + TAG_HDR_SIZE);
    	memcpy(cursor, &relayId, ntohs(relayId.length) + TAG_HDR_SIZE);
    	cursor += ntohs(relayId.length) + TAG_HDR_SIZE;
    	plen += ntohs(relayId.length) + TAG_HDR_SIZE;
    }
    
    if (hostUniq.type) 
    {
    	CHECK_ROOM(cursor, pado.payload, ntohs(hostUniq.length)+TAG_HDR_SIZE);
    	memcpy(cursor, &hostUniq, ntohs(hostUniq.length) + TAG_HDR_SIZE);
    	cursor += ntohs(hostUniq.length) + TAG_HDR_SIZE;
    	plen += ntohs(hostUniq.length) + TAG_HDR_SIZE;
    }
    
    pado.length = htons(plen);

    ACE_DEBUG((LM_DEBUG, "CPPPOEDiscoveryHandler::processPADI(), send PADO, length=%d\n", (SWORD32) (plen + HDR_SIZE)));
    SendPacket((const CHAR *)&pado, (SWORD32) (plen + HDR_SIZE));
}

/**********************************************************************
*%FUNCTION: processPADR
*%ARGUMENTS:
* ethif -- Ethernet interface
* packet -- PPPoE PADR packet
* len -- length of received packet
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Sends a PADS packet back to client and starts a PPP session if PADR
* packet is OK.
***********************************************************************/
void CPPPOEDiscoveryHandler::processPADR(PPPoEPacket *packet, SWORD32 len)
{
    BYTE cookieBuffer[COOKIE_LEN] = {0};
    PPPoEPacket pads;
    
    BYTE *cursor = pads.payload;
    WORD16 plen = 0;
    WORD16 slen = 0;
    CHAR const *serviceName = NULL;

    ACE_DEBUG((LM_DEBUG, "CPPPOEDiscoveryHandler::processPADR(), len=%d\n", len));

    BYTE myAddr[ETH_ALEN];
    ::memset(myAddr, 0, sizeof myAddr);
    GetPppoe().GetEtherIntf().GetIntfMac(myAddr);

    /* Initialize some static members. */
    relayId.type = 0;
    hostUniq.type = 0;
    receivedCookie.type = 0;
    requestedService.type = 0;

    /* Ignore PADR's not directed at us */
    if (memcmp(packet->ethHdr.h_dest, myAddr, ETH_ALEN)) 
    {
        ACE_DEBUG((LM_DEBUG, 
                   "CPPPOEDiscoveryHandler::processPADR(), Ignore PADR's not directed at us. "
                   "packet dest: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x; "
                   "myAddr: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
                   packet->ethHdr.h_dest[0], packet->ethHdr.h_dest[1], packet->ethHdr.h_dest[2],
                   packet->ethHdr.h_dest[3], packet->ethHdr.h_dest[4], packet->ethHdr.h_dest[5],
                   myAddr[0], myAddr[1], myAddr[2], myAddr[3], myAddr[4], myAddr[5]));

        return;
    }

    /* Ignore PADR's from non-unicast addresses */
    if (NOT_UNICAST(packet->ethHdr.h_source))
    {
        ACE_DEBUG((LM_ERROR, 
                   "CPPPOEDiscoveryHandler::processPADR(), PADR packet from non-unicast source address.\n"
                   "packet source: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
                   packet->ethHdr.h_source[0], packet->ethHdr.h_source[1], packet->ethHdr.h_source[2],
                   packet->ethHdr.h_source[3], packet->ethHdr.h_source[4], packet->ethHdr.h_source[5]));
        
    	return;
    }

    max_ppp_payload = 0;
    parsePacket(packet, parsePADRTags, NULL);

    /* Check that everything's cool */
    if (!receivedCookie.type) 
    {
    	/* Drop it -- do not send error PADS */
        ACE_DEBUG((LM_ERROR, "CPPPOEDiscoveryHandler::processPADR(), no TAG_AC_COOKIE in PADR packet, discard pkt.\n"));

    	return;
    }

    /* Is cookie kosher? */
    if (receivedCookie.length != htons(COOKIE_LEN)) 
    {
    	/* Drop it -- do not send error PADS */
        ACE_DEBUG((LM_ERROR, 
                   "CPPPOEDiscoveryHandler::processPADR(), invalid length(%u) of TAG_AC_COOKIE in PADR packet, discard pkt.\n",
                   receivedCookie.length));

    	return;
    }

    genCookie(packet->ethHdr.h_source, myAddr, m_cookieSeed, cookieBuffer);
    if (memcmp(receivedCookie.payload, cookieBuffer, COOKIE_LEN)) 
    {
    	/* Drop it -- do not send error PADS */
        ACE_DEBUG((LM_ERROR, "CPPPOEDiscoveryHandler::processPADR(), invalid payload of TAG_AC_COOKIE in PADR packet, discard pkt.\n"));

    	return;
    }

    /* Check service name */
    if (!requestedService.type) 
    {
        ACE_DEBUG((LM_ERROR, "CPPPOEDiscoveryHandler::processPADR(), Received PADR packet with no SERVICE_NAME tag\n"));
    	sendErrorPADS(packet->ethHdr.h_source, TAG_SERVICE_NAME_ERROR, "WFNEX-PPPoE: Server: No service name tag");
        
    	return;
    }

    slen = ntohs(requestedService.length);
    if (slen) 
    {
    	/* Check supported services */
	    if (slen == strlen(GetPppoe().GetSvcName()) 
            && !memcmp(GetPppoe().GetSvcName(), &requestedService.payload, slen)) 
        {
    		serviceName = GetPppoe().GetSvcName();
	    }

    	if (!serviceName) 
        {
    	    ACE_DEBUG((LM_ERROR, 
                       "CPPPOEDiscoveryHandler::processPADR(), Received PADR packet asking for unsupported service %.*s\n", 
                       (SWORD32) ntohs(requestedService.length), 
                       requestedService.payload));
            
    	    sendErrorPADS(packet->ethHdr.h_source, 
            			  TAG_SERVICE_NAME_ERROR, 
            			  "WFNEX-PPPoE: Server: Invalid service name tag");
    	    return;
    	}
    } 
    else 
    {
    	serviceName = "";
    }

    WORD16 sessionId = 0;

    CCmAutoPtr<CSession> session;
    GetPppoe().CreateSession(session);
    session->SetClientEth(packet->ethHdr.h_source);
    
    /* Send PADS and Start ppp negotiation. */
    memcpy(pads.ethHdr.h_dest, packet->ethHdr.h_source, ETH_ALEN);
    memcpy(pads.ethHdr.h_source, myAddr, ETH_ALEN);
    pads.ethHdr.h_proto = htons(ETH_PPPOE_DISCOVERY);
    pads.ver = 1;
    pads.type = 1;
    pads.code = CODE_PADS;
    
    pads.session = htons(session->GetSessionId());
    sessionId=pads.session;
    /* Copy requested service name tag back in.  If requested-service name
       length is zero, and we have non-zero services, use first service-name
       as default */
    if (!slen && (strlen(GetPppoe().GetSvcName()) != 0))
    {
    	slen = strlen(GetPppoe().GetSvcName());
    	memcpy(&requestedService.payload, GetPppoe().GetSvcName(), slen);
    	requestedService.length = htons(slen);
    }
    memcpy(cursor, &requestedService, TAG_HDR_SIZE+slen);
    cursor += TAG_HDR_SIZE+slen;
    plen += TAG_HDR_SIZE+slen;

    /* If we asked for an MTU, handle it */
    WORD16 intfMtu = GetPppoe().GetEtherIntf().GetIntfMtu();
    
    if (max_ppp_payload > ETH_PPPOE_MTU && intfMtu > 0) 
    {
    	/* Shrink payload to fit */
    	if (max_ppp_payload > intfMtu - TOTAL_OVERHEAD) 
        {
    	    max_ppp_payload = intfMtu - TOTAL_OVERHEAD;
    	}
    	if (max_ppp_payload > ETH_JUMBO_LEN - TOTAL_OVERHEAD) 
        {
    	    max_ppp_payload = ETH_JUMBO_LEN - TOTAL_OVERHEAD;
    	}
    	if (max_ppp_payload > ETH_PPPOE_MTU) 
        {
    	    PPPoETag maxPayload;
    	    WORD16 mru = htons(max_ppp_payload);
    	    maxPayload.type = htons(TAG_PPP_MAX_PAYLOAD);
    	    maxPayload.length = htons(sizeof(mru));
    	    memcpy(maxPayload.payload, &mru, sizeof(mru));
    	    CHECK_ROOM(cursor, pads.payload, sizeof(mru) + TAG_HDR_SIZE);
    	    memcpy(cursor, &maxPayload, sizeof(mru) + TAG_HDR_SIZE);
    	    cursor += sizeof(mru) + TAG_HDR_SIZE;
    	    plen += sizeof(mru) + TAG_HDR_SIZE;
    	    session->SetReqMtu(max_ppp_payload);
    	}
    }

    if (relayId.type) 
    {
    	memcpy(cursor, &relayId, ntohs(relayId.length) + TAG_HDR_SIZE);
    	cursor += ntohs(relayId.length) + TAG_HDR_SIZE;
    	plen += ntohs(relayId.length) + TAG_HDR_SIZE;
        // TBD!!!! 保存此session对应的relayId，以便后面在发送终结此session对应的PADT中加上此选项。
        // rp-pppoe中server端的PADT不带此TAG。
        // session.SetRelayId(xxxxx);
    }
    
    if (hostUniq.type) 
    {
    	memcpy(cursor, &hostUniq, ntohs(hostUniq.length) + TAG_HDR_SIZE);
    	cursor += ntohs(hostUniq.length) + TAG_HDR_SIZE;
    	plen += ntohs(hostUniq.length) + TAG_HDR_SIZE;
    }
    
    pads.length = htons(plen);

    ACE_DEBUG((LM_INFO, 
               "CPPPOEDiscoveryHandler::processPADR(), send PADS with session id(%#x), length=%d\n", 
               sessionId,
               (SWORD32) (plen + HDR_SIZE)));
    
    SendPacket((const CHAR *)&pads, (SWORD32) (plen + HDR_SIZE));

    // Start PPP negotiation.
    session->Init();
}

/**********************************************************************
*%FUNCTION: processPADT
*%ARGUMENTS:
* packet -- PPPoE PADT packet
* len -- length of received packet
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Kills session whose session-ID is in PADT packet.
***********************************************************************/
void CPPPOEDiscoveryHandler::processPADT(PPPoEPacket *packet, SWORD32 len)
{
    // packet is NOT NULL.

    ACE_DEBUG((LM_INFO, "CPPPOEDiscoveryHandler::processPADT(), len=%d\n", len));
    
    BYTE myAddr[ETH_ALEN];
    ::memset(myAddr, 0, sizeof myAddr);
    GetPppoe().GetEtherIntf().GetIntfMac(myAddr);
    
    /* Ignore PADT's not directed at us */
    if (memcmp(packet->ethHdr.h_dest, myAddr, ETH_ALEN))
    {
        ACE_DEBUG((LM_INFO, 
                   "CPPPOEDiscoveryHandler::processPADT(), Ignore PADT's not directed at us. "
                   "packet dest: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x; "
                   "myAddr: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
                   packet->ethHdr.h_dest[0], packet->ethHdr.h_dest[1], packet->ethHdr.h_dest[2],
                   packet->ethHdr.h_dest[3], packet->ethHdr.h_dest[4], packet->ethHdr.h_dest[5],
                   myAddr[0], myAddr[1], myAddr[2], myAddr[3], myAddr[4], myAddr[5]));
        
        return;
    }

    CSession * session = GetPppoe().FindSession(ntohs(packet->session));
    if (NULL == session)
    {
        ACE_DEBUG((LM_ERROR, 
                           "CPPPOEDiscoveryHandler::processPADT(), we don't have session with id %#x\n", 
                           ntohs(packet->session)));
        return;
    }

    /* If source MAC does not match, do not kill session */
    BYTE clientEth[ETH_ALEN];
    memset(clientEth, 0, ETH_ALEN);
    session->GetClientEth(clientEth);
    
    if (memcmp(packet->ethHdr.h_source, clientEth, ETH_ALEN)) 
    {
    	ACE_DEBUG((LM_ERROR, 
                   "CPPPOEDiscoveryHandler::processPADT(), PADT for session %#X received from "
        	       "%02X:%02X:%02X:%02X:%02X:%02X; should be from %02X:%02X:%02X:%02X:%02X:%02X",
        	       (unsigned int) ntohs(packet->session),
        	       packet->ethHdr.h_source[0],
        	       packet->ethHdr.h_source[1],
        	       packet->ethHdr.h_source[2],
        	       packet->ethHdr.h_source[3],
        	       packet->ethHdr.h_source[4],
        	       packet->ethHdr.h_source[5],
        	       clientEth[0],
        	       clientEth[1],
        	       clientEth[2],
        	       clientEth[3],
        	       clientEth[4],
        	       clientEth[5]));
    	return;
    }

    if (session->GetDeleteFlag())
    {
        ACE_DEBUG((LM_INFO, 
                   "CPPPOEDiscoveryHandler::processPADT(), we sent PADT and the user replies a PADT.  So remove this session(%#x).\n",
                   ntohs(packet->session)));
        GetPppoe().RemoveSession(ntohs(packet->session));
        return;        
    }
    
    parsePacket(packet, parseLogErrs, NULL);

    ACE_DEBUG((LM_INFO, "CPPPOEDiscoveryHandler::processPADT(), send PADT for session %#x\n", ntohs(packet->session)));
    
    sendPADT(session, "Received PADT");
}

/**********************************************************************
*%FUNCTION: genCookie
*%ARGUMENTS:
* peerEthAddr -- peer Ethernet address (6 bytes)
* myEthAddr -- my Ethernet address (6 bytes)
* seed -- random cookie seed to make things tasty (16 bytes)
* cookie -- buffer which is filled with server PID and
*           md5 sum of previous items
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Forms the md5 sum of peer MAC address, our MAC address and seed, useful
* in a PPPoE Cookie tag.
***********************************************************************/
void
CPPPOEDiscoveryHandler::genCookie(BYTE const *peerEthAddr, BYTE const *myEthAddr, BYTE const *seed, BYTE *cookie)
{
    MD5_CTX ctx;
    pid_t pid = getpid();

    MD5_Init(&ctx);
    MD5_Update(&ctx, peerEthAddr, ETH_ALEN);
    MD5_Update(&ctx, myEthAddr, ETH_ALEN);
    MD5_Update(&ctx, seed, SEED_LEN);
    MD5_Final(cookie, &ctx);
    memcpy(cookie+MD5_LEN, &pid, sizeof(pid));
}

/**********************************************************************
*%FUNCTION: sendErrorPADS
*%ARGUMENTS:
* sock -- socket to write to
* source -- source Ethernet address
* dest -- destination Ethernet address
* errorTag -- error tag
* errorMsg -- error message
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Sends a PADS packet with an error message
***********************************************************************/
void CPPPOEDiscoveryHandler::sendErrorPADS(BYTE *dest, SWORD32 errorTag, CHAR *errorMsg)
{
    PPPoEPacket pads;
    
    BYTE *cursor = pads.payload;
    WORD16 plen = 0;
    PPPoETag err;
    size_t elen = strlen(errorMsg);

    BYTE source[ETH_ALEN];
    ::memset(source, 0, sizeof source);
    GetPppoe().GetEtherIntf().GetIntfMac(source);

    memcpy(pads.ethHdr.h_dest, dest, ETH_ALEN);
    memcpy(pads.ethHdr.h_source, source, ETH_ALEN);
    pads.ethHdr.h_proto = htons(ETH_PPPOE_DISCOVERY);
    pads.ver = 1;
    pads.type = 1;
    pads.code = CODE_PADS;

    pads.session = htons(0);

    err.type = htons(errorTag);
    err.length = htons(elen);

    memcpy(err.payload, errorMsg, elen);
    memcpy(cursor, &err, TAG_HDR_SIZE+elen);
    cursor += TAG_HDR_SIZE + elen;
    plen += TAG_HDR_SIZE + elen;

    if (relayId.type) 
    {
    	memcpy(cursor, &relayId, ntohs(relayId.length) + TAG_HDR_SIZE);
    	cursor += ntohs(relayId.length) + TAG_HDR_SIZE;
    	plen += ntohs(relayId.length) + TAG_HDR_SIZE;
    }
    
    if (hostUniq.type) 
    {
    	memcpy(cursor, &hostUniq, ntohs(hostUniq.length) + TAG_HDR_SIZE);
    	cursor += ntohs(hostUniq.length) + TAG_HDR_SIZE;
    	plen += ntohs(hostUniq.length) + TAG_HDR_SIZE;
    }
    
    pads.length = htons(plen);
    SendPacket((const CHAR *)&pads, (SWORD32) (plen + HDR_SIZE));
}

/***********************************************************************
*%FUNCTION: sendPADT
*%ARGUMENTS:
* session -- PPPoE session
* msg -- if non-NULL, extra error message to include in PADT packet.
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Sends a PADT packet
***********************************************************************/
void
CPPPOEDiscoveryHandler::sendPADT(CSession *session, CHAR const *msg)
{
    ACE_DEBUG ((LM_DEBUG, "CPPPOEDiscoveryHandler::sendPADT, msg=%s\n", msg));
    
    if (NULL == session)
    {
        ACE_DEBUG ((LM_ERROR, "CPPPOEDiscoveryHandler::sendPADT, NULL session\n"));
        return;
    }

    if (session->GetDeleteFlag())
    {
        ACE_DEBUG ((LM_INFO, "We've sent PADT before, and the session is to be deleted, so we don't send PADT any more.\n"));
        return;
    }

    PPPoEPacket packet;
    
    BYTE *cursor = packet.payload;
    WORD16 plen = 0;
    
    BYTE clientEth[ETH_ALEN] = {0};
    session->GetClientEth(clientEth);

    BYTE myMac[ETH_ALEN] = {0};
    GetPppoe().GetEtherIntf().GetIntfMac(myMac);

    memcpy(packet.ethHdr.h_dest, clientEth, ETH_ALEN);
    memcpy(packet.ethHdr.h_source, myMac, ETH_ALEN);

    packet.ethHdr.h_proto = htons(ETH_PPPOE_DISCOVERY);
    packet.ver = 1;
    packet.type = 1;
    packet.code = CODE_PADT;
    packet.session = htons(session->GetSessionId());

    /* Copy error message */
    if (msg) 
    {
    	PPPoETag err;
    	size_t elen = strlen(msg);
    	err.type = htons(TAG_GENERIC_ERROR);
    	err.length = htons(elen);
    	strcpy((CHAR *) err.payload, msg);
    	memcpy(cursor, &err, elen + TAG_HDR_SIZE);
    	cursor += elen + TAG_HDR_SIZE;
    	plen += elen + TAG_HDR_SIZE;
    }

    /* Copy cookie and relay-ID if needed */
    
    /* Generate a cookie */
    BYTE myAddr[ETH_ALEN] = {0};
    GetPppoe().GetEtherIntf().GetIntfMac(myAddr);
    
    PPPoETag cookie;
    cookie.type = htons(TAG_AC_COOKIE);
    cookie.length = htons(COOKIE_LEN);
    genCookie(packet.ethHdr.h_source, myAddr, m_cookieSeed, cookie.payload);
    
	CHECK_ROOM(cursor, packet.payload, COOKIE_LEN + TAG_HDR_SIZE);
	memcpy(cursor, &cookie, COOKIE_LEN + TAG_HDR_SIZE);
	cursor += COOKIE_LEN + TAG_HDR_SIZE;
	plen += COOKIE_LEN + TAG_HDR_SIZE;

    /*当前server不支持保存中转client报文的relayId。详细参见CClientDiscovery.m_relayId.
    if (conn->relayId.type) {
	CHECK_ROOM(cursor, packet.payload,
		   ntohs(conn->relayId.length) + TAG_HDR_SIZE);
	memcpy(cursor, &conn->relayId, ntohs(conn->relayId.length) + TAG_HDR_SIZE);
	cursor += ntohs(conn->relayId.length) + TAG_HDR_SIZE;
	plen += ntohs(conn->relayId.length) + TAG_HDR_SIZE;
    }
    */

    packet.length = htons(plen);
    SendPacket((const CHAR *)&packet, (SWORD32) (plen + HDR_SIZE));

	dumpPacket(&packet, "SENT");

    ACE_DEBUG((LM_DEBUG, "Sent PADT\n"));

    // DO NOT Remove the session immediately to finish the PPP negotiation.
    // The session will be deleted after receiving user's PADT or 5-second timer expires.
    session->SetDeleteFlag();
    session->StartOneOffTimer();

    return;
}

/**********************************************************************
*%FUNCTION: parsePacket
*%ARGUMENTS:
* packet -- the PPPoE discovery packet to parse
* func -- function called for each tag in the packet
* extra -- an opaque data pointer supplied to parsing function
*%RETURNS:
* 0 if everything went well; -1 if there was an error
*%DESCRIPTION:
* Parses a PPPoE discovery packet, calling "func" for each tag in the packet.
* "func" is passed the additional argument "extra".
***********************************************************************/
SWORD32
CPPPOEDiscoveryHandler::parsePacket(PPPoEPacket *packet, ParseFunc *func, void *extra)
{
    if ((NULL == packet) || (NULL == func))
    {
        ACE_DEBUG ((LM_ERROR, 
                    "CPPPOEDiscoveryHandler::parsePacket(), Invalid arg. packet=%#x, func=%#x\n",
                    packet,
                    func));

    	return -1;
    }
    
    WORD16 len = ntohs(packet->length);
    BYTE  *curTag = NULL;
    WORD16 tagType = 0;
    WORD16 tagLen = 0;

    if (packet->ver != 1) 
    {
        ACE_DEBUG ((LM_ERROR, 
                    "CPPPOEDiscoveryHandler::parsePacket(), Invalid PPPoE version (%d)\n",
                    (SWORD32) packet->ver));

    	return -1;
    }
    
    if (packet->type != 1)
    {
        ACE_DEBUG ((LM_ERROR, 
                    "CPPPOEDiscoveryHandler::parsePacket(), Invalid PPPoE type (%d)\n",
                    (SWORD32) packet->type));

    	return -1;
    }

    /* Do some sanity checks on packet */
    /* 6-byte overhead for PPPoE header */
    if (len > ETH_JUMBO_LEN - PPPOE_OVERHEAD)
    { 
        ACE_DEBUG ((LM_ERROR, 
                    "CPPPOEDiscoveryHandler::parsePacket(), Invalid PPPoE packet length (%u)\n",
                    len));

    	return -1;
    }

    #if 0
    暂时不使用m_clientDiscoveryMgr
    CCmAutoPtr<CClientDiscovery> &clientDiscovery;
    int result = GetPppoe().CreateClientDiscovery(packet->ethHdr.h_source, clientDiscovery);
    if (-1 == result)
    {
        ACE_DEBUG((LM_ERROR, 
                   "CPPPOEDiscoveryHandler::parsePacket(), CreateClientDiscovery error",
                   "client Mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
                   packet->ethHdr.h_source[0], packet->ethHdr.h_source[1],
                   packet->ethHdr.h_source[2], packet->ethHdr.h_source[3],
                   packet->ethHdr.h_source[4], packet->ethHdr.h_source[5]));
        
        return -1;
    }
    #endif

    /* Step through the tags */
    curTag = packet->payload;
    while (curTag - packet->payload < len)
    {
    	/* Alignment is not guaranteed, so do this by hand... */
    	tagType = (((WORD16) curTag[0]) << 8) + (WORD16) curTag[1];
    	tagLen = (((WORD16) curTag[2]) << 8) + (WORD16) curTag[3];
        
    	if (TAG_END_OF_LIST == tagType)
        {
    	    return 0;
    	}
        
    	if ((curTag - packet->payload) + tagLen + TAG_HDR_SIZE > len)
        {
            ACE_DEBUG ((LM_ERROR, 
                        "CPPPOEDiscoveryHandler::parsePacket(), Invalid PPPoE tag length (%u)\n",
                        tagLen));

    	    return -1;
    	}
        
    	func(tagType, tagLen, curTag+TAG_HDR_SIZE, extra);
    	curTag = curTag + TAG_HDR_SIZE + tagLen;
    }
    
    return 0;
}

/**********************************************************************
*%FUNCTION: parsePADITags
*%ARGUMENTS:
* type -- tag type
* len -- tag length
* data -- tag data
* extra -- extra user data.
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Picks interesting tags out of a PADI packet
***********************************************************************/
void
CPPPOEDiscoveryHandler::parsePADITags(WORD16 type, WORD16 len, BYTE *data, void *extra)
{
    switch (type)
    {
        case TAG_PPP_MAX_PAYLOAD:
        {
        	if (len == sizeof(max_ppp_payload)) 
            {
        	    memcpy(&max_ppp_payload, data, sizeof(max_ppp_payload));
        	    max_ppp_payload = ntohs(max_ppp_payload);
        	    if (max_ppp_payload <= ETH_PPPOE_MTU)
                {
            		max_ppp_payload = 0;
        	    }
        	}
        	break;
        }
        case TAG_SERVICE_NAME:
        {
        	/* Copy requested service name */
        	requestedService.type = htons(type);
        	requestedService.length = htons(len);
        	memcpy(requestedService.payload, data, len);
        	break;
        }
        case TAG_RELAY_SESSION_ID:
        {
        	relayId.type = htons(type);
        	relayId.length = htons(len);
        	memcpy(relayId.payload, data, len);
        	break;
        }
        case TAG_HOST_UNIQ:
        {
        	hostUniq.type = htons(type);
        	hostUniq.length = htons(len);
        	memcpy(hostUniq.payload, data, len);
        	break;
        }
        default:
        {
            ACE_DEBUG((LM_ERROR, "CPPPOEDiscoveryHandler::parsePADITags(), Uknown tag type(%u).\n", type));
            break;
        }
    }

    return;
}

/**********************************************************************
*%FUNCTION: parsePADRTags
*%ARGUMENTS:
* type -- tag type
* len -- tag length
* data -- tag data
* extra -- extra user data.
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Picks interesting tags out of a PADR packet
***********************************************************************/
void CPPPOEDiscoveryHandler::parsePADRTags(WORD16 type, WORD16 len, BYTE *data, void *extra)
{
    switch (type) 
    {
        case TAG_PPP_MAX_PAYLOAD:
        {
        	if (len == sizeof(max_ppp_payload)) 
            {
        	    memcpy(&max_ppp_payload, data, sizeof(max_ppp_payload));
        	    max_ppp_payload = ntohs(max_ppp_payload);
        	    if (max_ppp_payload <= ETH_PPPOE_MTU) 
                {
            		max_ppp_payload = 0;
        	    }
        	}
        	break;
        }
        case TAG_RELAY_SESSION_ID:
        {
        	relayId.type = htons(type);
        	relayId.length = htons(len);
        	memcpy(relayId.payload, data, len);
        	break;
        }
        case TAG_HOST_UNIQ:
        {
        	hostUniq.type = htons(type);
        	hostUniq.length = htons(len);
        	memcpy(hostUniq.payload, data, len);
        	break;
        }
        case TAG_AC_COOKIE:
        {
        	receivedCookie.type = htons(type);
        	receivedCookie.length = htons(len);
        	memcpy(receivedCookie.payload, data, len);
        	break;
        }
        case TAG_SERVICE_NAME:
        {
        	requestedService.type = htons(type);
        	requestedService.length = htons(len);
        	memcpy(requestedService.payload, data, len);
        	break;
        }
        default:
        {
            ACE_DEBUG((LM_ERROR, "CPPPOEDiscoveryHandler::parsePADRTags(), uknown type(%#x)\n", type));
            break;
        }
    }

    return;
}

/**********************************************************************
*%FUNCTION: parseLogErrs
*%ARGUMENTS:
* type -- tag type
* len -- tag length
* data -- tag data
* extra -- extra user data
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Picks error tags out of a packet and logs them.
***********************************************************************/
void
CPPPOEDiscoveryHandler::parseLogErrs(WORD16 type, WORD16 len, BYTE *data, void *extra)
{
    pktLogErrs("PADT", type, len, data, extra);
}

/**********************************************************************
*%FUNCTION: pktLogErrs
*%ARGUMENTS:
* pkt -- packet type (a string)
* type -- tag type
* len -- tag length
* data -- tag data
* extra -- extra user data
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Logs error tags
***********************************************************************/
void
CPPPOEDiscoveryHandler::pktLogErrs(CHAR const *pkt, WORD16 type, WORD16 len, BYTE *data, void *extra)
{
    CHAR const *str;
    CHAR const *fmt = "%s: %s: %.*s\n";
    
    switch (type) 
    {
        case TAG_SERVICE_NAME_ERROR:
        {
        	str = "Service-Name-Error";
        	break;
        }
        case TAG_AC_SYSTEM_ERROR:
        {
        	str = "System-Error";
        	break;
        }
        default:
        {
        	str = "Generic-Error";
            break;
        }
    }

    ACE_DEBUG((LM_DEBUG, fmt, pkt, str, (int) len, data));
}

/**********************************************************************
*%FUNCTION: dumpPacket
*%ARGUMENTS:
* packet -- a PPPoE packet
* dir -- either SENT or RCVD
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Dumps the PPPoE packet to fp in an easy-to-read format
***********************************************************************/
void
CPPPOEDiscoveryHandler::dumpPacket(PPPoEPacket *packet, CHAR const *dir)
{    
    if (NULL == packet)
    {
        return;
    }
    
    static CHAR buffer[1024];
    ::memset(buffer, 0, sizeof buffer);
    static CHAR tmp[100];
    
    SWORD32 len = ntohs(packet->length);

    /* Sheesh... printing times is a pain... */
    struct timeval tv;
    time_t now;
    SWORD32 millisec;
    struct tm *lt;
    CHAR timebuf[256];

    WORD16 type = (WORD16) ntohs(packet->ethHdr.h_proto);

    gettimeofday(&tv, NULL);
    now = (time_t) tv.tv_sec;
    millisec = tv.tv_usec / 1000;
    lt = localtime(&now);
    strftime(timebuf, 256, "%H:%M:%S", lt);

    ::memset(tmp, 0, sizeof tmp);
    snprintf(tmp, sizeof tmp, "%s.%03d %s PPPoE ", timebuf, millisec, dir);
    strncpy(buffer, tmp, strlen(tmp));

    ::memset(tmp, 0, sizeof tmp);
    if (ETH_PPPOE_DISCOVERY == type) 
    {
    	snprintf(tmp, sizeof tmp, "Discovery (%x) ", (unsigned) type);
    } 
    else if (ETH_PPPOE_SESSION == type) 
    {
    	snprintf(tmp, sizeof tmp, "Session (%x) ", (unsigned) type);
    } 
    else 
    {
    	snprintf(tmp, sizeof tmp, "Unknown (%x) ", (unsigned) type);
    }
    strncpy(buffer + strlen(buffer), tmp, strlen(tmp));

    ::memset(tmp, 0, sizeof tmp);
    switch(packet->code) 
    {
        case CODE_PADI: strncpy(tmp, "PADI ", strlen("PADI ")); break;
        case CODE_PADO: strncpy(tmp, "PADO ", strlen("PADO ")); break;
        case CODE_PADR: strncpy(tmp, "PADR ", strlen("PADR ")); break;
        case CODE_PADS: strncpy(tmp, "PADS ", strlen("PADS ")); break;
        case CODE_PADT: strncpy(tmp, "PADT ", strlen("PADT ")); break;
        case CODE_PADM: strncpy(tmp, "PADM ", strlen("PADM ")); break;
        case CODE_PADN: strncpy(tmp, "PADN ", strlen("PADN ")); break;
        case CODE_SESS: strncpy(tmp, "SESS ", strlen("SESS ")); break;
        default:        snprintf(tmp, sizeof tmp, "Unkown code (%#x) ", packet->code); break;
    }
    strncpy(buffer + strlen(buffer), tmp, strlen(tmp));

    ::memset(tmp, 0, sizeof tmp);
    snprintf(tmp, sizeof tmp, "sess-id %d length %d\n", (int) ntohs(packet->session), len);
    strncpy(buffer + strlen(buffer), tmp, strlen(tmp));

    /* Ugly... I apologize... */
    ::memset(tmp, 0, sizeof tmp);
    snprintf(tmp, 
             sizeof tmp, 
      	     "SourceAddr %02x:%02x:%02x:%02x:%02x:%02x "
      	     "DestAddr %02x:%02x:%02x:%02x:%02x:%02x\n",
      	    (unsigned) packet->ethHdr.h_source[0],
      	    (unsigned) packet->ethHdr.h_source[1],
      	    (unsigned) packet->ethHdr.h_source[2],
      	    (unsigned) packet->ethHdr.h_source[3],
      	    (unsigned) packet->ethHdr.h_source[4],
      	    (unsigned) packet->ethHdr.h_source[5],
      	    (unsigned) packet->ethHdr.h_dest[0],
      	    (unsigned) packet->ethHdr.h_dest[1],
      	    (unsigned) packet->ethHdr.h_dest[2],
      	    (unsigned) packet->ethHdr.h_dest[3],
      	    (unsigned) packet->ethHdr.h_dest[4],
      	    (unsigned) packet->ethHdr.h_dest[5]);
    strncpy(buffer + strlen(buffer), tmp, strlen(tmp));

    ACE_DEBUG ((LM_DEBUG, "%s", buffer));
    
    dumpHex(packet->payload, ntohs(packet->length));
}

/**********************************************************************
*%FUNCTION: dumpHex
*%ARGUMENTS:
* fp -- file to dump to
* buf -- buffer to dump
* len -- length of data
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Dumps buffer to fp in an easy-to-read format
***********************************************************************/
void
CPPPOEDiscoveryHandler::dumpHex(BYTE const *buf, SWORD32 len)
{
    SWORD32 i;
    SWORD32 base;

    static CHAR buffer[4*1024];
    ::memset(buffer, 0, sizeof buffer);
    buffer[0] = '\n';
    SWORD32 usedLength = 0;
    
    /* do NOT dump PAP packets */
    if (len >= 2 && buf[0] == 0xC0 && buf[1] == 0x23)
    {
    	ACE_DEBUG((LM_DEBUG, "(PAP Authentication Frame -- Contents not dumped)\n"));
    	return;
    }

    for (base=0; base<len && usedLength<sizeof(buffer); base += 16) 
    {
    	for (i=base; i<base+16 && usedLength + 3 < sizeof(buffer); i++) 
        {
    	    if (i < len) 
            {
        		snprintf(buffer + strlen(buffer), 4, "%02x ", (unsigned) buf[i]);
                usedLength += 3;
    	    } 
            else 
            {
        		snprintf(buffer + strlen(buffer), 4, "   ");
                usedLength += 3;
    	    }
    	}

        if (usedLength + 2 >= sizeof(buffer))
        {
            break;
        }
        
    	snprintf(buffer + strlen(buffer), 3, "  ");
        usedLength += 2;
        
    	for (i=base; i<base+16; i++) 
        {
    	    if (i < len  && usedLength + 1 < sizeof(buffer)) 
            {
        		if (isprint(buf[i])) 
                {
        		    snprintf(buffer + strlen(buffer), 2, "%c", buf[i]);
                    usedLength += 1;
        		} 
                else 
                {
        		    snprintf(buffer + strlen(buffer), 2, ".");
                    usedLength += 1;
        		}
    	    } 
            else 
            {
        		break;
    	    }
    	}

        if (usedLength + 1 >= sizeof(buffer))
        {
            break;
        }
        
    	snprintf(buffer + strlen(buffer), 2, "\n");
        usedLength += 1;
    }

    ACE_DEBUG((LM_DEBUG, "%s", buffer));
}


