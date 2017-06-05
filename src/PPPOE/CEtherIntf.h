/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
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

