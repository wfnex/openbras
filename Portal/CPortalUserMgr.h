/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
**********************************************************************/
#ifndef CPORTALUSERMGR_H
#define CPORTALUSERMGR_H

#include "aceinclude.h"
#include <stdint.h>
#include <unordered_map>
#include "CReferenceControl.h"

class CPortalManager;
class CPortalUser : public CReferenceControl
{
public:
    CPortalUser(uint64_t userid);
    virtual ~CPortalUser();
    uint64_t GetUserID();
    ACE_INET_Addr GetPortalServerIP();
    ACE_INET_Addr GetPortalLocalIP();
    void SetPortalServerIP(const ACE_INET_Addr &ipaddr);
    void SetPortalLocalIP(const ACE_INET_Addr &ipaddr);
    bool IsAuthOK();
    void SetAuthResult(bool isAuthOk);
    void SetAuthInfo(const char *packet, size_t packetsize);
    void GetAuthInfo(char *&packet, size_t &packetsize);
private:
    uint64_t m_userid;
    ACE_INET_Addr m_PortalServerAddr;
    ACE_INET_Addr m_PortalLocalAddr;
    bool m_isAuthOk;
    char *m_packet;
    size_t m_packetsize;
};

class CPortalUserMgr 
{
public:
    CPortalUserMgr(CPortalManager &mgr);
    ~CPortalUserMgr();
    static CPortalUserMgr *Instance();
    int CreateUser(uint32_t ipaddr, uint32_t vpnid, CPortalUser *&puser);
    int FindUser(uint32_t ipaddr, uint32_t vpnid, CPortalUser *&puser);
    uint64_t GetUserID(uint32_t ipaddr, uint32_t vpnid);
    int AddUser(CCmAutoPtr<CPortalUser> &user);
    int RemoveUser(uint64_t userid);
    int RemoveUser(uint32_t ipaddr, uint32_t vpnid);
    int FindUser(uint64_t userid, CPortalUser *&puser);
private:
    ACE_Thread_Mutex m_mutex;
    typedef std::unordered_map<uint64_t, CCmAutoPtr<CPortalUser>> USERType;
    USERType m_users;
    CPortalManager &m_mgr;
};


#endif//CPORTALUSERMGR_H

