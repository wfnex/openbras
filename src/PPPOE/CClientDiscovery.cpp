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

WORD64 CClientDiscovery::GetClientDiscoveryId()
{
    WORD64 clientMac = 0;
    ::memcpy(&clientMac, m_clientMac, ETH_ALEN);
    return clientMac;
}

void CClientDiscovery::SetClientMac(BYTE clientMac[ETH_ALEN])
{
    if (clientMac != NULL)
    {
        ::memcpy(m_clientMac, clientMac, ETH_ALEN);
    }
}

void CClientDiscovery::GetSvcName(CHAR svcName[PPPOE_MAX_SERVICE_NAME_LENGTH])
{
    ::strncpy(svcName, m_serviceName, sizeof m_serviceName);
    svcName[PPPOE_MAX_SERVICE_NAME_LENGTH - 1] = 0;
}

void CClientDiscovery::SetSvcName(CHAR svcName[PPPOE_MAX_SERVICE_NAME_LENGTH])
{
    ::strncpy(m_serviceName, svcName, sizeof m_serviceName);
    m_serviceName[PPPOE_MAX_SERVICE_NAME_LENGTH - 1] = 0;
}


