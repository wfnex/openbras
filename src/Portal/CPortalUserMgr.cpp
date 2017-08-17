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

#include "CPortalUserMgr.h"
#include "CPortalManager.h"

CPortalUser::CPortalUser(uint64_t userid)
    :m_userid(userid)
    ,m_isAuthOk(false)
    ,m_packet(NULL)
    ,m_packetsize(0)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalUser::CPortalUser\n")); 
}

CPortalUser::~CPortalUser()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalUser::~CPortalUser\n")); 
    if (m_packet)
    {
        free(m_packet);
        m_packet = NULL;
        m_packetsize = 0;
    }
}

//Get User Id
uint64_t CPortalUser::GetUserID()
{
    return m_userid;
}

//Get Portal Server Ip
ACE_INET_Addr CPortalUser::GetPortalServerIP()
{
    return m_PortalServerAddr;
}

//Get Portal Local Ip
ACE_INET_Addr CPortalUser::GetPortalLocalIP()
{
    return m_PortalLocalAddr;
}

//Set Portal Server Ip
void CPortalUser::SetPortalServerIP(const ACE_INET_Addr &ipaddr)
{
    m_PortalServerAddr = ipaddr;
}

//Set Portal Local Ip
void CPortalUser::SetPortalLocalIP(const ACE_INET_Addr &ipaddr)
{
    m_PortalLocalAddr = ipaddr;
}

//check Auth Status
bool CPortalUser::IsAuthOK()
{
    return m_isAuthOk;
}

//Set Auth Result
void CPortalUser::SetAuthResult(bool isAuthOk)
{
    m_isAuthOk = isAuthOk;
}

//Set Auth Info
void CPortalUser::SetAuthInfo(const char *packet, size_t packetsize)
{
    ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalUser::SetAuthInfo SetAuthInfo packet=%d,packetsize=%d\n",packet,packetsize));

    if ((packet == NULL) || (packetsize==0))
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalUser::SetAuthInfo error\n"));
        return;
    }
    if (m_packet)
    {
        free(m_packet);
        m_packet = NULL;
        m_packetsize = 0;
    }
    m_packet = (char *)malloc(packetsize);
    if (m_packet)
    {
        ::memcpy(m_packet, packet, packetsize);
    }
    m_packetsize = packetsize;
}

//Get Auth Info
void CPortalUser::GetAuthInfo(char *&packet, size_t &packetsize)
{
    packet = m_packet;
    packetsize = m_packetsize;
}

CPortalUserMgr::CPortalUserMgr(CPortalManager &mgr)
    :m_mgr(mgr)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalUserMgr::CPortalUserMgr\n")); 
}

CPortalUserMgr::~CPortalUserMgr()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalUserMgr::~CPortalUserMgr\n")); 
}

//Get UserId
uint64_t CPortalUserMgr::GetUserID(uint32_t ipaddr, uint32_t vrf)
{
    uint64_t id = 0;
    ::memcpy((char*)&id,(char*)&ipaddr, sizeof(uint32_t));
    ::memcpy((char*)&id+sizeof(uint32_t),(char*)&vrf,sizeof(uint32_t));
    return id;
}

//Add User
int CPortalUserMgr::AddUser(CCmAutoPtr<CPortalUser> &user)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, -1);
    uint64_t userid = user->GetUserID();
    USERType::iterator it = m_users.find(userid);
    if(it != m_users.end())
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalUserMgr::AddUser already have one\n"));
        return -1;
    }
    m_users[userid]=user;
    return 0;
}

//Remove User
int CPortalUserMgr::RemoveUser(uint64_t userid)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, -1);
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalUserMgr::RemoveUser size=%d\n",m_users.size())); 

    m_users.erase(userid);
    return 0; 
}

int CPortalUserMgr::RemoveUser(uint32_t ipaddr, uint32_t vpnid)
{
    uint64_t userid = GetUserID(ipaddr,vpnid);
    return RemoveUser(userid);
}

//Find User
int CPortalUserMgr::FindUser(uint64_t userid, CPortalUser *&puser)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, -1);
    USERType::iterator it = m_users.find(userid);
    if(it == m_users.end())
    {
        return -1;
    }
    CCmAutoPtr<CPortalUser> &user = it->second;
    puser = user.Get();
    return 0;

}

int CPortalUserMgr::FindUser(uint32_t ipaddr, uint32_t vpnid, CPortalUser *&puser)
{
    uint64_t userid = GetUserID(ipaddr,vpnid);
    return FindUser(userid,puser);
}

//Create User
int CPortalUserMgr::CreateUser(uint32_t ipaddr, uint32_t vpnid, CPortalUser *&puser)
{
    uint64_t userid = GetUserID(ipaddr,vpnid);
    CCmAutoPtr<CPortalUser> user(new CPortalUser(userid));
    if (AddUser(user) == 0)
    {
        puser = user.Get();
        return 0;
    }
    ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalUserMgr::CreateUser already have one\n"));
    return -1;
}



