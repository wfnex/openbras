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

#ifndef CPORTALFAKESERVER_H
#define CPORTALFAKESERVER_H
#include "aceinclude.h"
#include <stdint.h>
#include <unordered_map>
#include "CTCPTransport.h"
#include "CReferenceControl.h"
#include "openportal.h"

class CPortalManager;
class CPortalFakeServer;
class CPortalHTTPChannel : public CTCPTransport, public CReferenceControl
{
public:
    CPortalHTTPChannel();
    CPortalHTTPChannel(uint32_t id, CPortalFakeServer *pserver);
    virtual ~CPortalHTTPChannel();
    void SetPortalServer(CPortalFakeServer *pserver);
    virtual size_t OnHandleMessage(const ACE_Message_Block &aData);
    virtual void OnRcvBufferOverFlow(size_t maxbuffersize);
    virtual void OnConnected();
    inline uint32_t GetID() const
    {
        return m_id;
    }
private:
    uint32_t m_id;
    CPortalFakeServer *m_pserver;
};

typedef ACE_Acceptor<CPortalHTTPChannel, ACE_SOCK_ACCEPTOR> ACCEPTOR;

class CPortalConfig;
class CPortalFakeServer : public ACCEPTOR
{
public:
    CPortalFakeServer(CPortalManager &mgr);
    virtual ~CPortalFakeServer();
    virtual int StartListen(const ACE_INET_Addr &httplistenip);
    virtual void StopListen();
    virtual int accept_svc_handler (CPortalHTTPChannel *handler);
    virtual int make_svc_handler (CPortalHTTPChannel *&sh);
    virtual int AddChannel(CPortalHTTPChannel *pvs);
    virtual int RemoveChannel(CPortalHTTPChannel *pvs);
    virtual CPortalHTTPChannel *FindChannel(int handler);
    CPortalConfig &GetPortalConfig();
private:
    uint32_t m_nHttIDBase;
    typedef std::unordered_map<uint32_t, CCmAutoPtr<CPortalHTTPChannel>> UNBINDVSType;
    UNBINDVSType    m_UnBindVSs;
    CPortalManager &m_mgr;
};



#endif//CPORTALFAKESERVER_H
