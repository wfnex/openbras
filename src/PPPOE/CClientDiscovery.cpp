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
#include "CClientDiscovery.h"

CClientDiscovery::CClientDiscovery(BYTE clientMac[ETH_ALEN])
    : m_sess(0)
    , m_requestedMtu(0)
{
    if (clientMac != NULL)
    {
        ::memcpy(m_clientMac, clientMac, ETH_ALEN);
    }
    ::memset(m_serviceName, 0, sizeof m_serviceName);
    ::memset(&m_relayId, 0, sizeof m_relayId);
}

CClientDiscovery::~CClientDiscovery()
{
}

// Get Client Mac
WORD64 CClientDiscovery::GetClientDiscoveryId()
{
    WORD64 clientMac = 0;
    ::memcpy(&clientMac, m_clientMac, ETH_ALEN);
    return clientMac;
}

//Set Client Mac
void CClientDiscovery::SetClientMac(BYTE clientMac[ETH_ALEN])
{
    if (clientMac != NULL)
    {
        ::memcpy(m_clientMac, clientMac, ETH_ALEN);
    }
}

//Get Service Name
void CClientDiscovery::GetSvcName(CHAR svcName[PPPOE_MAX_SERVICE_NAME_LENGTH])
{
    ::strncpy(svcName, m_serviceName, sizeof m_serviceName);
    svcName[PPPOE_MAX_SERVICE_NAME_LENGTH - 1] = 0;
}

//Set service Name
void CClientDiscovery::SetSvcName(CHAR svcName[PPPOE_MAX_SERVICE_NAME_LENGTH])
{
    ::strncpy(m_serviceName, svcName, sizeof m_serviceName);
    m_serviceName[PPPOE_MAX_SERVICE_NAME_LENGTH - 1] = 0;
}


