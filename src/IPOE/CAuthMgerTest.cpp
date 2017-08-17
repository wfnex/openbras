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

#include "CAuthMgerTest.h"

IAuthManager &IAuthManager::instance()
{
    static CAuthManager authmgr;
    return authmgr;
}


CAuthManager::CAuthManager()
        :m_psink(0)
{
}

CAuthManager::~CAuthManager()
{
}


int CAuthManager::OpenWithSink(IAuthManagerSink *psink)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CAuthManager::openWithSink\n"));

    m_psink = psink;
    return 0;
}

int CAuthManager::Close()
{
    m_psink = NULL;
    return 0;
}

int CAuthManager::AuthRequest(const Auth_Request *request)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CAuthManager::AuthRequest\n")); 
    Auth_Response response;
    response.authResult = 0;
    memcpy(response.mac,request->mac,6);

    ACE_INET_Addr subip(100,"100.1.1.2");
    ACE_INET_Addr dns1(100,"200.1.1.1");
    ACE_INET_Addr dns2(100,"200.1.1.100");
    ACE_INET_Addr gateway(100,"100.1.1.1");
    ACE_INET_Addr netmask(100,"255.255.255.0");


    response.subscriberIP = subip.get_ip_address();
    response.primaryDNS = dns1.get_ip_address();
    response.secondaryDNS = dns2.get_ip_address();
    response.subscriberIPMask = netmask.get_ip_address();
    response.gatewayaddr = gateway.get_ip_address();
    response.leaseTime = 10;

    m_psink->OnAuthResponse(&response);
    return 0;
}

std::string CAuthManager::AllocIp()
{

}


