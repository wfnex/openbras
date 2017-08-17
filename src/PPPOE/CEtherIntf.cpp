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

//provide PPPOE interface
CPPPOE &CEtherIntf::GetPppoe()
{
    return m_pppoe;
}

//Get Intf Name
void CEtherIntf::GetIntfName(CHAR acIntfName[ETHER_INTF_NAME_SIZE+1])
{
    if (acIntfName != NULL)
    {
        ::strncpy(acIntfName, m_name, sizeof m_name);
        acIntfName[ETHER_INTF_NAME_SIZE] = 0;
    }
}

//Set Intf Name
void CEtherIntf::SetIntfName(const CHAR acIntfName[ETHER_INTF_NAME_SIZE+1])
{
    if (acIntfName != NULL)
    {
        ::strncpy(m_name, acIntfName, sizeof m_name);
        m_name[ETHER_INTF_NAME_SIZE] = 0;
    }
}

//Get Intf Mac
void CEtherIntf::GetIntfMac(BYTE acIntfMac[ETH_ALEN])
{
    if (acIntfMac != NULL)
    {
        ::memcpy(acIntfMac, m_mac, sizeof m_mac);
    }
}

//Set Intf Ip
void CEtherIntf::SetIntfIp(std::string &ip)
{
    std::string tmp = ip;
    tmp += ":0";
    
    ACE_INET_Addr addr(tmp.c_str());
    SetIntfIp( htonl(addr.get_ip_address()) );
}

//Set Intf Mac By Name
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


