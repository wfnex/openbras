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


#ifndef CSESSIONMANAGER_H
#define CSESSIONMANAGER_H
#include "ISessionManager.h"
#include "aceinclude.h"
#include "CUser.h"
#include <unordered_map>

typedef std::unordered_map<uint32_t, CUser *> USERMAP;

class CSessionManager : public ISessionManager 
{
public:
    CSessionManager();
    virtual ~CSessionManager();
    
    virtual int openWithSink(ISessionManagerSink *pSink);
    virtual int Close();
    
    virtual int addUserRequest(const Session_User_Ex* sInfo);
    virtual int deleteUserRequest(const Session_Offline* sInfo);
    virtual int modifyUserRequest(const Session_User_Ex* sInfo);

    void AddUser(CUser *user);
    int DeleteUser(WORD32 ip);
    WORD32 FindUserIp(WORD16 sessionid);
private:
    ISessionManagerSink *m_psink;
    USERMAP m_usermgr;
};


#endif//CSESSIONMANAGER_H

