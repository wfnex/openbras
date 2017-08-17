#ifndef CUSER_H
#define CUSER_H

#include "BaseDefines.h"
#include "aceinclude.h"
#include "aim_ex.h"

class CUser {
public:
    CUser();
    ~CUser();

    void DateFrom(const Session_User_Ex* sInfo);
    
    WORD16 GetSession();
    void SetSession(WORD16 session);
    BYTE* GetMac();
    void SetMac(BYTE mac[SESSION_MGR_MAC_SIZE]);
    BYTE GetUsertype();
    void SetUsertype(BYTE usertype);
    BYTE GetAccesstype();
    void SetAccesstype(BYTE acctype);
    BYTE GetAuthtype();
    void SetAuthtype(BYTE authtype);
    BYTE GetAuthstate();
    void SetAuthstate(BYTE authstate);
    CHAR* GetUsername();
    void SetUsername(CHAR username[SESSION_MGR_MAX_USERNAME_SIZE]);
    CHAR* GetDomian();
    void SetDomian(CHAR domain[SESSION_MGR_MAX_DOMAIN_SIZE]);
    
    WORD32 GetVrf();
    void SetVrf(WORD32 vrf);
    WORD32 GetUserIp();
    void SetUserIp(WORD32 userip);
    WORD32 GetPrimarydns();
    void SetPrimarydns(WORD32 primarydns);
    WORD32 GetSecondarydns();
    void SetSecondarydns(WORD32 secondarydns);
    WORD32 GetPoolid();
    void SetPoolid(WORD32 poolid);
    WORD32 GetInternvlan();
    void SetInternvlan(WORD32 internvlan);
    WORD32 GetExternvlan();
    void SetExternvlan(WORD32 externvlan);
private:
    WORD16 m_session;
    BYTE  m_mac[SESSION_MGR_MAC_SIZE];
    BYTE  m_usertype;             // 用户类型,pppoe ipoe�?
    BYTE  m_accesstype;           // 用户接入类型 ipv4 ipv6或dual stack
    BYTE  m_authtype;             // 认证类型如radius认证
    BYTE  m_authstate;            // 用户认证状�?
    CHAR  m_username[SESSION_MGR_MAX_USERNAME_SIZE];
    CHAR  m_domain[SESSION_MGR_MAX_DOMAIN_SIZE];  
    WORD32   m_vrf;
    WORD32   m_userip;            // radius分配给subsriber的IP，主机序
    WORD32   m_primarydns;        // radius分配给subsriber的Primary DNS IP，主机序
    WORD32   m_secondarydns;      // radius分配给subsriber的Secondary DNS IP，主机序   
    WORD32   m_poolid;            // ip pool id�?
    WORD32   m_internvlan;        // 接入内层vlan�?
    WORD32   m_externvlan;        // 接入外层vlan�
};


#endif //CUSER_H












