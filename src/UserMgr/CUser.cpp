#include "CUser.h"
#include <string.h>

//Get and Set User info
CUser::CUser()
    :m_session(0),m_usertype(0),m_accesstype(0),
    m_authtype(0),m_authstate(0),m_vrf(0),
    m_userip(0),m_primarydns(0),m_secondarydns(0),
    m_poolid(0),m_internvlan(0),m_externvlan(0)
{
    ACE_DEBUG ((LM_DEBUG, "CUser::CUser\n"));
    ::memset(m_mac, 0, sizeof m_mac);
    ::memset(m_username,0,sizeof m_username);
    ::memset(m_domain,0,sizeof m_domain);
}
CUser::~CUser()
{
    ACE_DEBUG ((LM_DEBUG, "CUser::~CUser\n"));    
}

void CUser::DateFrom(const Session_User_Ex* sInfo)
{
    m_session = sInfo->session;
    ::memcpy(m_mac, sInfo->mac, sizeof m_mac);
    m_usertype = sInfo->user_type;             
    m_accesstype = sInfo->access_type;           
    m_authtype = sInfo->auth_type;           
    m_authstate = sInfo->auth_state;          
    ::strncpy(m_username,sInfo->user_name,sizeof m_username); 
    m_username[SESSION_MGR_MAX_USERNAME_SIZE - 1] = 0;
    ::strncpy(m_domain,sInfo->domain,sizeof m_domain); 
    m_domain[SESSION_MGR_MAX_DOMAIN_SIZE - 1] = 0; 
    m_vrf = sInfo->vrf;
    m_userip = sInfo->user_ip;           
    m_primarydns = sInfo->primary_dns;       
    m_secondarydns = sInfo->secondary_dns;        
    m_poolid = sInfo->pool_id;            
    m_internvlan = sInfo->intern_vlan;        
    m_externvlan = sInfo->extern_vlan;     
}


WORD16 CUser::GetSession()
{
    return m_session;
}
void CUser::SetSession(WORD16 session)
{
    m_session = session;
}
BYTE* CUser::GetMac()
{
    return m_mac;
}
void CUser::SetMac(BYTE mac[SESSION_MGR_MAC_SIZE])
{
    if (mac != NULL)
    {
        ::memcpy(m_mac, mac, sizeof m_mac);
    }
}
BYTE CUser::GetUsertype()
{
    return m_usertype;
}
void CUser::SetUsertype(BYTE usertype)
{
    m_usertype = usertype;
}
BYTE CUser::GetAccesstype()
{
    return m_accesstype;
}
void CUser::SetAccesstype(BYTE acctype)
{
    m_accesstype = acctype;
}
BYTE CUser::GetAuthtype()
{
    return m_authtype;
}
void CUser::SetAuthtype(BYTE authtype)
{
    m_authtype = authtype;
}
BYTE CUser::GetAuthstate()
{
    return m_authstate;
}
void CUser::SetAuthstate(BYTE authstate)
{
    m_authstate = authstate;
}
CHAR* CUser::GetUsername()
{
    return m_username;
}
void CUser::SetUsername(CHAR username[SESSION_MGR_MAX_USERNAME_SIZE])
{
    if(username != NULL)
    {
        ::strncpy(m_username,username,sizeof m_username); 
        m_username[SESSION_MGR_MAX_USERNAME_SIZE - 1] = 0;
    }
}
CHAR* CUser::GetDomian()
{
    return m_domain;
}
void CUser::SetDomian(CHAR domain[SESSION_MGR_MAX_DOMAIN_SIZE])
{
    if(domain != NULL)
    {
        ::strncpy(m_domain,domain,sizeof m_domain);
        m_domain[SESSION_MGR_MAX_DOMAIN_SIZE - 1] = 0;
    }
}

WORD32 CUser::GetVrf()
{
    return m_vrf;
}
void CUser::SetVrf(WORD32 vrf)
{
    m_vrf = vrf;
}
WORD32 CUser::GetUserIp()
{
    return m_userip;
}
void CUser::SetUserIp(WORD32 userip)
{
    m_userip = userip;
}
WORD32 CUser::GetPrimarydns()
{
    return m_primarydns;
}
void CUser::SetPrimarydns(WORD32 primarydns)
{
    m_primarydns = primarydns;
}
WORD32 CUser::GetSecondarydns()
{
    return m_secondarydns;
}
void CUser::SetSecondarydns(WORD32 secondarydns)
{
    m_secondarydns = secondarydns;
}
WORD32 CUser::GetPoolid()
{
    return m_poolid;
}
void CUser::SetPoolid(WORD32 poolid)
{
    m_poolid = poolid;
}
WORD32 CUser::GetInternvlan()
{
    return m_internvlan;
}
void CUser::SetInternvlan(WORD32 internvlan)
{
    m_internvlan = internvlan;
}
WORD32 CUser::GetExternvlan()
{
    return m_externvlan;
}
void CUser::SetExternvlan(WORD32 externvlan)
{
    m_externvlan = externvlan;
}




