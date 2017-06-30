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


#ifndef CPPP_H
#define CPPP_H

#include "aceinclude.h"
#include "pppdef.h"
#include "CPPPLCP.h"
#include "CPPPIPCP.h"
#include "CPapProtocol.h"
#include <string.h>
#include "ISessionManager.h"
#include "CChapProtocol.h"
#include <string>
#include "CAuthen.h"

enum PPPPhase {
    PHASE_DEAD = 0,
    PHASE_INITIALIZE,
    PHASE_ESTABLISH,
    PHASE_AUTHENTICATE,
    PHASE_NETWORK,
    PHASE_TERMINATE
};

#define BRAS_DEFAULT_AUTHTYPE (PPP_PAP)
#define BRAS_DEFAULT_HOSTNAME ("WFNEX")

class IPPPSink 
{
public:
    virtual ~IPPPSink(){}

    // Protocol related
    virtual void OnLCPDown(const std::string reason) = 0;

    // Packet related
    virtual void OnPPPOutput(unsigned char *packet, size_t size) = 0;  // 输出PPP报文
    virtual void OnPPPPayload(unsigned char *packet, size_t size) = 0; // 接收到PPP数据报文（不归我们处理，因为我们只处理
                                                                       // PPP协议报文），log后直接返回

    // Authentication Manager related
    virtual void OnAuthRequest(Auth_Request &authReq) = 0;
    
    // Subscriber Manager related
    virtual SWORD32 OnAddSubscriber(Session_User_Ex &sInfo) = 0;
    virtual SWORD32 OnDelSubscriber() = 0;
};

class CPPP : public IPPPLCPSink, 
                        public IPPPIPCPSink,
                        public IAuthenSvrSink
{
public:
    CPPP(IPPPSink *psink);
    virtual ~CPPP();
    
    void StartFms();

    void Init();

    void OnAuthResponse(WORD32 result, std::string &reason);
    
    virtual void Input(unsigned char *packet, size_t size);
    virtual void InputPayload(unsigned char *packet, size_t size);

    CPPPLCP &GetLCP() {return m_lcp;}
    CPPPIPCP &GetIPCP() {return m_ipcp;}
    void SetAuthType(uint16_t authType) {m_lcp.SetAuthType(authType);}
    uint16_t GetAuthType() {return m_lcp.GetAuthType();}
    void SetHostName(std::string &hostName) {m_hostName = hostName;}
    
protected:
    void NewPhase(PPPPhase phase);

    //IPPPLCPSink
    virtual void OnLCPUp();
    virtual void OnLCPDown();
    virtual void OnLCPTerminate();
    virtual void OnLCPOutput(unsigned char *packet, size_t size);

    //IPPPIPCPSink
    virtual void OnIPCPUp();
    virtual void OnIPCPDown();
    virtual void OnIPCPPayload(unsigned char *packet, size_t size);
    virtual void OnIPCPOutput(unsigned char *packet, size_t size);

    // For interface IAuthenSvrSink
    virtual void SendAuthRequest2AM(Auth_Request &authReq);
    virtual void OnAuthenOutput(unsigned char *packet, size_t size);
    virtual void OnAuthenResult(int result, int protocol);

private:
    CPPPLCP m_lcp;
    CPapProtocolSvr m_pap;
    CChapProtocolSvr m_chap;
    CPPPIPCP m_ipcp;
    IPPPSink *m_pSink;
    bool m_bLinkOk;
    PPPPhase m_phase;
    std::string m_hostName;
};

#endif//CPPP

