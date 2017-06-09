/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
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

    response.vrf=0;
    response.user_ip=request->subIp;

    m_psink->OnAuthResponse(&response);
    return 0;
}



