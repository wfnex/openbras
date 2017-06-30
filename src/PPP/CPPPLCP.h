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


#ifndef CPPPLCP_H
#define CPPPLCP_H

#include "pppdef.h"
#include "CPPPFsm.h"
#include "CPPPProtocol.h"
#include "BaseDefines.h"

class VPN_PUBLIC IPPPLCPSink 
{
public:
    virtual ~IPPPLCPSink(){}
    
    virtual void OnLCPUp() = 0;
    virtual void OnLCPDown() = 0;
    virtual void OnLCPTerminate() = 0;
    virtual void OnLCPOutput(unsigned char *packet, size_t size) = 0;
};

class VPN_PUBLIC CPPPLCP : public CPPPProtocol, 
                           public IFsmSink,
                           public ACE_Event_Handler
{
public:
    CPPPLCP(IPPPLCPSink *psink);
    virtual ~CPPPLCP();

    // For interface CPPPProtocol
    virtual void Init();
    virtual void Input(unsigned char *packet ,size_t size);
    virtual void Protrej();
    virtual void LowerUp();
    virtual void LowerDown();
    virtual void Open();
    virtual void Close(char *reason);

    // For interface IFsmSink
    virtual void Resetci(CPPPFsm *pfsm);
    virtual int Cilen(CPPPFsm *pfsm);
    virtual void Addci(CPPPFsm *pfsm, u_char *, int *);
    virtual int Ackci(CPPPFsm *pfsm, u_char *, int);
    virtual int  Nakci(CPPPFsm *pfsm, u_char *, int, int);
    virtual int  Rejci(CPPPFsm *pfsm, u_char *, int);
    virtual int  Reqci(CPPPFsm *pfsm, u_char *, int *, int);
    virtual void Up(CPPPFsm *pfsm);
    virtual void Down(CPPPFsm *pfsm);
    virtual void Starting(CPPPFsm *pfsm);
    virtual void Finished(CPPPFsm *pfsm);
    virtual void ProtReject(int);
    virtual void Retransmit(CPPPFsm *pfsm);
    virtual int ExtCode(CPPPFsm *pfsm, int, int, u_char *, int);
    virtual void OutputPacket(unsigned char *pkg, size_t size);

    // For interface ACE_Event_Handler
    virtual int handle_timeout (const ACE_Time_Value &current_time,
                                const void *act = 0);

    int GetId();
    int State();
    void SendProtRej(u_char *p, int len);
    lcp_options &GetWantOpt() {return m_lcp_wantoptions;}
    lcp_options &GetGotOpt() {return m_lcp_gotoptions;}
    lcp_options &GetAllowOpt() {return m_lcp_allowoptions;}
    lcp_options &GetHisOpt() {return m_lcp_hisoptions;}
    void SetAuthType(uint16_t authType) {m_authType = authType;}
    uint16_t GetAuthType() {return m_authType;}
    
protected:
    void EchoTimeout();
    void RcvEchoReply(int id, u_char *inp, int len);
    void LcpLinkFailure ();
    void SendEchoRequest();
    void EchoLowerUp ();
    void EchoLowerDown ();
    void CancelEchoTimer();
    void LcpEchoCheck ();
    void StartEchoTimer(int interval);
    void LCPRcvProtRej(u_char *inp, int len);
    
private:
    IPPPLCPSink *m_psink;
    CPPPFsm m_fsm;
    int m_lcp_echos_pending;       /* Number of outstanding echo msgs */
    int m_lcp_echo_number;         /* ID number of next echo frame */
    int m_lcp_echo_timer_running;  /* set if a timer is running */
    uint32_t m_magicnumber;
    int m_lcp_echo_fails;          /* Tolerance to unanswered echo-requests */
    int m_lcp_echo_interval;       /* Interval between LCP echo-requests */
    lcp_options m_lcp_wantoptions;  /* Options that we want to request */
    lcp_options m_lcp_gotoptions;   /* Options that peer ack'd */
    lcp_options m_lcp_allowoptions; /* Options we allow peer to request */
    lcp_options m_lcp_hisoptions;   /* Options that we ack'd */
    int m_peer_mru;
    uint16_t m_authType;  // The value of authType is the enumeraton of AIM_AUTH_TYPE in aim_ex.h    
};


#endif//CPPPLCP_H

