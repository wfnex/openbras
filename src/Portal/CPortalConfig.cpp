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

#include "CPortalConfig.h"
#include "CPortalServerChannel.h"
#include "CPortalServerManager.h"
#include "CPortalManager.h"

CPortalConfig::CPortalConfig(CPortalManager &mgr)
    :m_portallistenport(0)
    ,m_httplistenport(0)
    ,m_servercount(0)
    ,m_mgr(mgr)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalConfig::CPortalConfig\n")); 
}

CPortalConfig::~CPortalConfig()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalConfig::~CPortalConfig\n")); 
}

//Init config
int CPortalConfig::Init()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalConfig::Init\n")); 

    dictionary * ini = NULL;
    char        * s  = NULL;
    char        * https = NULL;
    int         listcount = 0;

    ini = iniparser_load("portal.cfg");
    if (ini == NULL) {
        ACE_DEBUG ((LM_INFO, "CPortalConfig, failed to iniparser_load ipoe.cfg.\n"));
        return -1;
    }
    
    iniparser_dump(ini, stderr);

    s = const_cast<char *>(iniparser_getstring(ini, "Global:PortalClientListenIP", NULL));
    m_portalistenip = std::string(s, strlen(s));
    m_portallistenport = iniparser_getint(ini,"Global:PortalClientListenPortal",2000);
    https = const_cast<char *>(iniparser_getstring(ini, "Global:RedirectServerListenIP", NULL));
    m_httplistenip = std::string(https, strlen(https));
    m_httplistenport = iniparser_getint(ini,"Global:RedirectServerListenPort",4444);

    m_servercount = iniparser_getint(ini,"PortalServerList:count",0);

    m_detecttimeout=iniparser_getint(ini,"PortalServerList:detecttimeout",0);
     ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalConfig::Init,m_servercount=%d\n",m_servercount)); 
    if (m_servercount>SERVER_MAX)
    {
        m_servercount = SERVER_MAX;
    }
    for(int i=0;i<m_servercount;i++)
    {
        
        char buffer[1024]={0};
        sprintf(buffer,"PortalServerList:PortalServer%d",i);
        char *psip=const_cast<char *>(iniparser_getstring(ini,buffer, NULL));


        ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalConfig:Init,buffer=%s\n",buffer)); 

        ::memset(buffer, 0, sizeof(buffer));
        sprintf(buffer,"PortalServerList:PortalServerPort%d",i);

        
        int pport=iniparser_getint(ini,buffer, 50100);

        ::memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "PortalServerList:URL%d", i);
        char *urlstr=const_cast<char *>(iniparser_getstring(ini,buffer, NULL));


        ::memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "PortalServerList:KEY%d", i);
        char *keystr=const_cast<char *>(iniparser_getstring(ini,buffer, NULL));

        ::memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "PortalServerList:version%d", i);
        int version=iniparser_getint(ini,buffer, 1);

        
        ACE_INET_Addr subip(pport,psip);
        ACE_INET_Addr myaddr(m_portallistenport,m_portalistenip.c_str());
        //CCmAutoPtr<CPortalServerChannel> channel(new CPortalServerChannel(myaddr, subip));
        PortalServerCfg *pcfg=(PortalServerCfg*)malloc(sizeof(PortalServerCfg));
        PortalServerCfg &cfg = *pcfg;
        cfg.version=version;
        cfg.ip_address=subip.get_ip_address();
        cfg.port=subip.get_port_number();
        cfg.server_id = i;
        if (keystr)
        {
            if (PORTAL_WEB_SERVER_MAX_KEY_LEN + 1<strlen(keystr))
            {
                ::memcpy(cfg.key,keystr,PORTAL_WEB_SERVER_MAX_KEY_LEN);
            }
            else
            {
                ::memcpy(cfg.key,keystr,strlen(keystr));
            }
        }

        if (urlstr)
        {
            if (PORTAL_WEB_SERVER_MAX_URL_LEN + 1<strlen(urlstr))
            {
                ::memcpy(cfg.url,urlstr,PORTAL_WEB_SERVER_MAX_URL_LEN);
            }
            else
            {
                ::memcpy(cfg.url,urlstr,strlen(urlstr));
            }
        }

        m_serverlist.push_back(pcfg);

        //channel->StartDetect(m_detecttimeout);
        //CPortalServerManager::Instance()->AddChannel(subip,myaddr,channel);
        m_mgr.GetPortalClient().CreateConnector(*pcfg);
    }
    
    iniparser_freedict(ini); 

    //CPortalServerManager::Instance()->Dump();

    return 0;
}

//Gets the address of the portal listener
ACE_INET_Addr CPortalConfig::GetPortalListenAddr()
{
    ACE_INET_Addr myaddr(m_portallistenport,m_portalistenip.c_str());
    return myaddr;
}

//Get Http Listen Addr
ACE_INET_Addr CPortalConfig::GetHttpListenAdd()
{
    ACE_INET_Addr myaddr(m_httplistenport,m_httplistenip.c_str());
    return myaddr;
}

//Get Server config
PortalServerCfg *CPortalConfig::GetServerCfg(uint32_t serverip)
{
    std::list<PortalServerCfg *>::iterator it = m_serverlist.begin();
    while(it != m_serverlist.end())
    {
        PortalServerCfg *cfg = *it;
        if (cfg)
        {
            if (cfg->ip_address == serverip)
            {
                return cfg;
            }
        }
        it++;
    }

    return NULL;
}

//Get Redirect Config
PortalServerCfg *CPortalConfig::GetRedirectCfg(uint8_t serverid)
{
    std::list<PortalServerCfg *>::iterator it = m_serverlist.begin();
    while(it != m_serverlist.end())
    {
        PortalServerCfg *cfg = *it;
        if (cfg)
        {
            if (cfg->server_id== serverid)
            {
                return cfg;
            }
        }
        it++;
    }

    return NULL;   
}



