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

#include "CRadiusConfig.h"
#include "CPortalManager.h"

CRadiusConfig::CRadiusConfig(CRadiusManager &mgr)
    :m_localport(0)
    ,m_authserverport(0)
    ,m_accserverport(0)
    ,m_responsetimeout(0)
    ,m_retransmit(0)
    ,m_mgr(mgr)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CRadiusConfig::CRadiusConfig\n")); 
}

CRadiusConfig::~CRadiusConfig()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CRadiusConfig::~CRadiusConfig\n")); 
}

int CRadiusConfig::Init()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CRadiusConfig::Init\n")); 

    dictionary * ini = NULL;
    const char * slocal  = NULL;
    const char * sasip = NULL;
    const char * scsip = NULL;
    const char * sshared = NULL;

    int         listcount = 0;

    ini = iniparser_load("radius.cfg");
    if (ini == NULL) {
        ACE_DEBUG ((LM_INFO, "CRadiusConfig, failed to iniparser_load ipoe.cfg.\n"));
        return -1;
    }
    
    iniparser_dump(ini, stderr);

    slocal = iniparser_getstring(ini, "Global:RadiusLocalIP", NULL);
    m_localip = std::string(slocal, strlen(slocal));
    m_localport = iniparser_getint(ini,"Global:RadiusLocalPort",21812);
    sasip = iniparser_getstring(ini, "Global:RadiusAuthServerIP", NULL);
    m_authserverip = std::string(sasip, strlen(sasip));
    m_authserverport = iniparser_getint(ini,"Global:RadiusAuthServerPort",1812);

    scsip = iniparser_getstring(ini, "Global:RadiusAccServerIP", NULL);
    m_accserverip = std::string(scsip, strlen(scsip));
    m_accserverport = iniparser_getint(ini,"Global:RadiusAccServerPort",1812);

    sshared = iniparser_getstring(ini, "Global:SharedKey", NULL);
    m_sharedkey = std::string(sshared, strlen(sshared));

    m_responsetimeout = iniparser_getint(ini,"Global:ResponseTimeOut",0);

    m_retransmit=iniparser_getint(ini,"Global:Retransmit",0);

#define RADIUS_TEST
    m_bauth = iniparser_getboolean(ini,"Test:test_access",1);
    m_bacc = iniparser_getboolean(ini,"Test:test_accounting",1);
    const char *username  = iniparser_getstring(ini, "Test:User_Name", NULL);
    m_UserName=std::string(username, strlen(username));

    const char *password  = iniparser_getstring(ini, "Test:User_Password", NULL);
    m_Userpassword=std::string(password, strlen(password));

    const char  *called_station_id  = iniparser_getstring(ini, "Test:Called_Station_Id", NULL);
    m_called_station_id=std::string(called_station_id, strlen(called_station_id));

    const char  *calling_station_id  = iniparser_getstring(ini, "Test:Calling_Station_Id", NULL);
    m_calling_station_id=std::string(calling_station_id, strlen(calling_station_id));

    m_acct_session_timeout=iniparser_getint(ini,"Test:Acct_Session_Time",0);
    m_acct_terminate_cause=iniparser_getint(ini,"Test:Acct_Terminate_Cause",0);

    
    iniparser_freedict(ini); 

    //CPortalServerManager::Instance()->Dump();

    return 0;
}

//Get Local Addr
ACE_INET_Addr CRadiusConfig::GetLocalAddr()
{
    ACE_INET_Addr myaddr(m_localport,m_localip.c_str());
    return myaddr;
}

//Get Primer Auth Server Addr
ACE_INET_Addr CRadiusConfig::GetAuthServerAddrP()
{
    ACE_INET_Addr myaddr(m_authserverport,m_authserverip.c_str());
    return myaddr; 
}

//Get Primer Acct Server Addr
ACE_INET_Addr CRadiusConfig::GetAcctServerAddrP()
{
    ACE_INET_Addr myaddr(m_accserverport,m_accserverip.c_str());
    return myaddr;
}

//Get Second Auth Server Addr
ACE_INET_Addr CRadiusConfig::GetAuthServerAddrS()
{
    ACE_INET_Addr myaddr(m_authserverports,m_authserverips.c_str());
    return myaddr; 
}

//Get Second Acct Server Addr
ACE_INET_Addr CRadiusConfig::GetAcctServerAddrS()
{
    ACE_INET_Addr myaddr(m_accserverports,m_accserverips.c_str());
    return myaddr;
}


