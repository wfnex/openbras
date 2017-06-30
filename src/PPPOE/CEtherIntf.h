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


#ifndef CETHER_INTF_H
#define CETHER_INTF_H

#include "BaseDefines.h"
#include "pppoe.h"
#include <string>

#define ETHER_INTF_NAME_SIZE 16

class CPPPOE;

/* An Ethernet interface */
class CEtherIntf
{
public:
    CEtherIntf(CPPPOE &pppoe);
    virtual ~CEtherIntf();

    CPPPOE &GetPppoe();
    void GetIntfName(CHAR acIntfName[ETHER_INTF_NAME_SIZE+1]);
    void SetIntfName(const CHAR acIntfName[ETHER_INTF_NAME_SIZE+1]);
    void GetIntfMac(BYTE acIntfMac[ETH_ALEN]);
    WORD16 GetIntfMtu() {return m_mtu;}
    void SetIntfMtu(WORD16 mtu) {m_mtu = mtu;}
    WORD32 GetIntfIp() {return m_ip;}
    void SetIntfIp(WORD32 ip) { m_ip = ip;}
    void SetIntfIp(std::string &ip);
    
    // SetIntfXXXByName() should be called after calling SetIntfName("xxxxx"). 
    int SetIntfMacByName();
    
private:    
    CPPPOE &m_pppoe;
    CHAR m_name[ETHER_INTF_NAME_SIZE+1];	/* Interface name */
    BYTE m_mac[ETH_ALEN];       /* MAC address */
    WORD16 m_mtu;               /* MTU of interface. MTU给转发用，由全局配置来设定 */
    WORD32 m_ip;                /* Ip address. In network byte order. */
};

#endif // CETHER_INTF_H

