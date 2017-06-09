/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved. 
**********************************************************************/
#ifndef CPORTALCONFIG_H
#define CPORTALCONFIG_H
#include <stdlib.h>
#include <stdint.h>
#include "iniparser.h"
#include "openportal.h"
#include <string>
#include <list>
#include "aceinclude.h"
#define SERVER_MAX 10

class CPortalManager;
class CPortalConfig
{
public:
    CPortalConfig(CPortalManager &mgr);
    ~CPortalConfig();
    int Init();
    ACE_INET_Addr GetPortalListenAddr();
    ACE_INET_Addr GetHttpListenAdd();
    PortalServerCfg *GetServerCfg(uint32_t serverip);
    inline int GetDetectTimeout() const
    {
        return m_detecttimeout;
    }
    PortalServerCfg *GetRedirectCfg(uint8_t serverid);
private:
    std::string m_portalistenip;
    uint16_t m_portallistenport;
    std::string m_httplistenip;
    uint16_t m_httplistenport;
    int m_servercount;
    int m_detecttimeout;
    std::list<PortalServerCfg *> m_serverlist;
    CPortalManager &m_mgr;
};

#endif//CPORTALCONFIG_H

