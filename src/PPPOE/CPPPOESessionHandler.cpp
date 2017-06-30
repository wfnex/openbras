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
#include "CPPPOESessionHandler.h"
#include "CPPPOE.h"
#include "CSession.h"

CPPPOESessionHandler::CPPPOESessionHandler(CPPPOE &pppoe)
    :m_pppoe(pppoe)
{
}

CPPPOESessionHandler::~CPPPOESessionHandler()
{
}

int CPPPOESessionHandler::ProcessPacket(const char *packet, ssize_t size)
{
    if (NULL == packet)
    {
        ACE_DEBUG ((LM_ERROR,
                   "CPPPOESessionHandler::ProcessPacket(), packet NULL\n"));
    	return -1;
    }

    #ifdef DEBUG_PKT
    static CHAR buffer[4*1024];
    memset(buffer, 0, sizeof buffer);

    const BYTE *printPkt = (const BYTE *)packet;

    if (3 * size < 4 * 1024)
    {
        ACE_DEBUG((LM_INFO, "CPPPOESessionHandler::ProcessPacket(), dump packet:\n"));
        ssize_t i;
        for (i = 0; i < size; ++i)
        {
            snprintf(buffer + strlen(buffer), 4, "%02x ", printPkt[i]);
        }
        ACE_DEBUG((LM_INFO, "%s\n", buffer));
    }
    #endif
    
    if (size < HDR_SIZE)
    {
    	/* Impossible - ignore */
        ACE_DEBUG ((LM_ERROR,
                   "CPPPOESessionHandler::ProcessPacket(), packet size too short(%d), less than HDR_SIZE(%u)\n",
                   size, 
                   HDR_SIZE));
    	return -1;
    }

    PPPoEPacket *pkt = (PPPoEPacket *)packet;
    
    /* Sanity check on packet */
    if (pkt->ver != 1 || pkt->type != 1)
    {
        ACE_DEBUG ((LM_ERROR,
                   "CPPPOESessionHandler::ProcessPacket(), packet version(%u) or type(%u) wrong.\n",
                   pkt->ver, 
                   pkt->type));
    	return -1;
    }

    if (pkt->code != CODE_SESS)
    {
        ACE_DEBUG ((LM_ERROR,
                   "CPPPOESessionHandler::ProcessPacket(), Bogus PPPoE code field (%#x).\n",
                   pkt->code));

    	return -1;
    } 

    /* Check length */
    if (ntohs(pkt->length) + HDR_SIZE > size) 
    {
        ACE_DEBUG ((LM_ERROR,
                   "CPPPOESessionHandler::ProcessPacket(), Bogus PPPoE length field (%u). size=%d\n",
                   (unsigned int) ntohs(pkt->length),
                   size));

    	return -1;
    }

    /* Dispatch the packet to the associated PPP entity. */
    CSession * session = GetPppoe().FindSession(ntohs(pkt->session));
    if (NULL == session)
    {
        ACE_DEBUG ((LM_ERROR,
                   "CPPPOESessionHandler::ProcessPacket(), Bogus PPPoE sessionId field (%#x)\n",
                   (unsigned int) ntohs(pkt->session)));

    	return -1;
    }

    return session->ProcessSessData(pkt, size);
}

int CPPPOESessionHandler::Init()
{
    int ret = CPPPOEPacketHandler::OpenX(ETH_PPPOE_SESSION);
    if (-1 == ret)
    {
        ACE_DEBUG((LM_ERROR, 
                   "CPPPOESessionHandler::Init(), CPPPOEPacketHandler::OpenX(ETH_PPPOE_SESSION) failed. ret=%d\n", 
                   ret));
        return -1;
    }

    CHAR intfName[ETHER_INTF_NAME_SIZE+1] = {0};
    GetPppoe().GetEtherIntf().GetIntfName(intfName);

    ret = CPPPOEPacketHandler::BindInterface(intfName, ETH_PPPOE_SESSION);
    if (-1 == ret)
    {
        ACE_DEBUG((LM_ERROR, 
                   "CPPPOESessionHandler::Init(), CPPPOEPacketHandler::BindInterface(ETH_PPPOE_SESSION) failed. "
                   "intfName=%s ret=%d\n", 
                   intfName,
                   ret));
        return -1;
    }
    
    return 0;
}

CPPPOE &CPPPOESessionHandler::GetPppoe()
{
    return m_pppoe;
}


