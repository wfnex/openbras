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

