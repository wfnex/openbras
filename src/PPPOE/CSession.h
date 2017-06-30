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
#ifndef CSESSION_H
#define CSESSION_H

#include "aceinclude.h"
#include "CReferenceControl.h"
#include "STDInclude.h"
#include "pppoe.h"
#include "BaseDefines.h"
#include "CPPP.h"

class CPPPOE;

class CSession : public CReferenceControl, public IPPPSink, public ACE_Event_Handler
{
public:
    CSession(CPPPOE &pppoe, WORD16 session);
    virtual ~CSession();

    int Init();
    int ProcessSessData(const PPPoEPacket *packet, ssize_t size);

    // For Interface IPPPSink
    virtual void OnLCPDown(const std::string reason);
    virtual void OnPPPOutput(unsigned char *packet, size_t size);
    virtual void OnPPPPayload(unsigned char *packet, size_t size);
    virtual void OnAuthRequest(Auth_Request &authReq);
    virtual SWORD32 OnAddSubscriber(Session_User_Ex &sInfo);
    virtual SWORD32 OnDelSubscriber();
    
    void OnPPPOEAuthResponse(WORD32 result, std::string reason); // 处理认证响应

    // For interface ACE_Event_Handler
    virtual int handle_timeout (const ACE_Time_Value &current_time,
                                const void *act);

    void CancelTimer();
    void StartOneOffTimer();
    
    // Gets and Sets
    inline CPPPOE &GetPppoe() const;
    inline CPPP &GetPPP();
    inline WORD16 GetSessionId() const;
    inline WORD16 GetReqMtu() const;
    inline void SetReqMtu(WORD16 reqMtu);
    inline void GetClientEth(BYTE clientEth[ETH_ALEN]) const;
    inline void SetClientEth(BYTE clientEth[ETH_ALEN]);
    inline bool GetDeleteFlag() const;
    inline void SetDeleteFlag();

private:
    CPPPOE &m_pppoe;
    CPPP m_ppp;
    WORD16 m_sessionid;
    WORD16 m_requestedMtu;     /* Requested PPP_MAX_PAYLOAD  per RFC 4638 */
                               // PPPOE协商得到的最大MRU，作为本端PPP协商时MRU的初始值。
                               // 目前实现: PPP协商MRU的初始值为1492。
    BYTE m_clientEth[ETH_ALEN]; /* Peer's Ethernet address */
    bool m_sessionToBeDeleted;  // Is this session to be deleted? After we send PADT, this field will be set true. 
                                // And we don't response to the received PADT, neither send or receive PPP pakcets.
                                // Implemented according to RFC 2516.
    SWORD32 m_time2waitClientPADT;  // After we send PADT, we'll wait so long after which we'll delete this session.                                
};

inline CPPPOE & CSession::GetPppoe() const
{
    return m_pppoe;
}

inline CPPP & CSession::GetPPP()
{
    return m_ppp;
}

inline WORD16 CSession::GetSessionId() const
{
    return m_sessionid;
}

inline WORD16 CSession::GetReqMtu() const 
{
    return m_requestedMtu;
}

inline void CSession::SetReqMtu(WORD16 reqMtu)
{
    m_requestedMtu = reqMtu;
}

inline void CSession::GetClientEth(BYTE clientEth[ETH_ALEN]) const
{
    if (clientEth != NULL)
    {
        memcpy(clientEth, m_clientEth, ETH_ALEN);
    }
}

inline void CSession::SetClientEth(BYTE clientEth[ETH_ALEN])
{
    memcpy(m_clientEth, clientEth, ETH_ALEN);
}

inline bool CSession::GetDeleteFlag() const
{
    return m_sessionToBeDeleted;
}

inline void CSession::SetDeleteFlag()
{
    m_sessionToBeDeleted = true;
}

#endif //CSESSION_H

