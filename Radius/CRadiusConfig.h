/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved. 
**********************************************************************/
#ifndef CRADIUSCONFIG_H
#define CRADIUSCONFIG_H
#include <stdlib.h>
#include <stdint.h>
#include "iniparser.h"
#include "openportal.h"
#include <string>
#include <list>
#include "aceinclude.h"
#define SERVER_MAX 10

#define RADIUS_TEST 1

class CRadiusManager;
class CRadiusConfig
{
public:
    CRadiusConfig(CRadiusManager &mgr);
    ~CRadiusConfig();
    int Init();
    ACE_INET_Addr GetLocalAddr();
    ACE_INET_Addr GetAuthServerAddrP();
    ACE_INET_Addr GetAcctServerAddrP();
    ACE_INET_Addr GetAuthServerAddrS();
    ACE_INET_Addr GetAcctServerAddrS();

    std::string GetSharedKey() const {return m_sharedkey;}
    int GetResponseTimeout() const {return m_responsetimeout;}
    int GetRetrans() const {return m_retransmit;}
#ifdef RADIUS_TEST
    int   test_access()            const { return m_bauth; }
    int   test_accounting()        const { return m_bacc; }
    std::string User_Name()              const { return m_UserName; }
    std::string User_Password()          const { return m_Userpassword; }
    std::string Called_Station_Id()      const { return m_called_station_id; }
    std::string Calling_Station_Id()     const { return m_calling_station_id; }
    int Acct_Session_Time()      const { return m_acct_session_timeout; }
    int Acct_Terminate_Cause()   const { return m_acct_terminate_cause; }
#endif
private:
    std::string m_localip;
    uint16_t m_localport;
    std::string m_authserverip;
    uint16_t m_authserverport;
    std::string m_accserverip;
    uint16_t m_accserverport;

    std::string m_authserverips;
    uint16_t m_authserverports;
    std::string m_accserverips;
    uint16_t m_accserverports;


    std::string m_sharedkey;
    int m_responsetimeout;
    int m_retransmit;
#define RADIUS_TEST
    int m_bauth;
    int m_bacc;
    std::string m_UserName;
    std::string m_Userpassword;
    std::string m_called_station_id;
    std::string m_calling_station_id;
    int m_acct_session_timeout;
    int m_acct_terminate_cause;

    CRadiusManager &m_mgr;

};

#endif//CPORTALCONFIG_H

