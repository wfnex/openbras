/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
**********************************************************************/
#include "CRadiuConfig.h"
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
    char        * slocal  = NULL;
    char        * sasip = NULL;
    char        * scsip = NULL;
    char        * sshared = NULL;

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
    char        *username  = iniparser_getstring(ini, "Test:User_Name", NULL);
    m_UserName=std::string(username, strlen(username));

    char        *password  = iniparser_getstring(ini, "Test:User_Password", NULL);
    m_Userpassword=std::string(password, strlen(password));

    char        *called_station_id  = iniparser_getstring(ini, "Test:Called_Station_Id", NULL);
    m_called_station_id=std::string(called_station_id, strlen(called_station_id));

    char        *calling_station_id  = iniparser_getstring(ini, "Test:Calling_Station_Id", NULL);
    m_calling_station_id=std::string(calling_station_id, strlen(calling_station_id));

    m_acct_session_timeout=iniparser_getint(ini,"Test:Acct_Session_Time",0);
    m_acct_terminate_cause=iniparser_getint(ini,"Test:Acct_Terminate_Cause",0);

#endif

    
    iniparser_freedict(ini); 

    //CPortalServerManager::Instance()->Dump();

    return 0;
}

ACE_INET_Addr CRadiusConfig::GetLocalAddr()
{
    ACE_INET_Addr myaddr(m_localport,m_localip.c_str());
    return myaddr;
}

ACE_INET_Addr CRadiusConfig::GetAuthServerAddrP()
{
    ACE_INET_Addr myaddr(m_authserverport,m_authserverip.c_str());
    return myaddr; 
}

ACE_INET_Addr CRadiusConfig::GetAcctServerAddrP()
{
    ACE_INET_Addr myaddr(m_accserverport,m_accserverip.c_str());
    return myaddr;
}

ACE_INET_Addr CRadiusConfig::GetAuthServerAddrS()
{
    ACE_INET_Addr myaddr(m_authserverports,m_authserverips.c_str());
    return myaddr; 
}

ACE_INET_Addr CRadiusConfig::GetAcctServerAddrS()
{
    ACE_INET_Addr myaddr(m_accserverports,m_accserverips.c_str());
    return myaddr;
}


