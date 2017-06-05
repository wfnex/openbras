/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
**********************************************************************/

#include "CEtherIntf.h"
#include <string.h>
#include "CPPPOE.h"

CEtherIntf::CEtherIntf(CPPPOE &pppoe)
    : m_pppoe(pppoe)
    , m_mtu(ETH_DATA_LEN)
    , m_ip(0)
{
    ::memset(m_name, 0, sizeof m_name);
    ::memset(m_mac, 0, sizeof m_mac); 
}
        
CEtherIntf::~CEtherIntf()
{
}

CPPPOE &CEtherIntf::GetPppoe()
{
    return m_pppoe;
}

void CEtherIntf::GetIntfName(CHAR acIntfName[ETHER_INTF_NAME_SIZE+1])
{
    if (acIntfName != NULL)
    {
        ::strncpy(acIntfName, m_name, sizeof m_name);
        acIntfName[ETHER_INTF_NAME_SIZE] = 0;
    }
}

void CEtherIntf::SetIntfName(const CHAR acIntfName[ETHER_INTF_NAME_SIZE+1])
{
    if (acIntfName != NULL)
    {
        ::strncpy(m_name, acIntfName, sizeof m_name);
        m_name[ETHER_INTF_NAME_SIZE] = 0;
    }
}

void CEtherIntf::GetIntfMac(BYTE acIntfMac[ETH_ALEN])
{
    if (acIntfMac != NULL)
    {
        ::memcpy(acIntfMac, m_mac, sizeof m_mac);
    }
}

void CEtherIntf::SetIntfIp(std::string &ip)
{
    std::string tmp = ip;
    tmp += ":0";
    
    ACE_INET_Addr addr(tmp.c_str());
    SetIntfIp( htonl(addr.get_ip_address()) );
}

int CEtherIntf::SetIntfMacByName()
{
    CHAR acEmptyName[ETHER_INTF_NAME_SIZE+1];
    ::memset(acEmptyName, 0, sizeof acEmptyName);

    if (::memcmp(m_name, acEmptyName, sizeof acEmptyName) == 0)
    {
        ACE_DEBUG ((LM_ERROR,
                   "CEtherIntf::SetIntfMacByName(), empty interface name\n"));
        return -1;
	}
    
    struct ifreq ifr;
    int fd = GetPppoe().GetPppoeDiscoveryHndl().get_handle();
    
	::strncpy(ifr.ifr_name, m_name, sizeof(ifr.ifr_name));
	if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
        ACE_DEBUG ((LM_ERROR,
                   "CEtherIntf::SetIntfMacByName(), ioctl(SIOCGIFHWADDR)\n"));
        return -1;
	}
	memcpy(m_mac, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

	if (NOT_UNICAST(m_mac))
    {
        ACE_DEBUG ((LM_ERROR,
                   "CEtherIntf::SetIntfMacByName(), Interface %.16s has broadcast/multicast MAC address??\n",
                   m_name));
        return -1;
	}

    return 0;
}


