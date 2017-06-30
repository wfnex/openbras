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


#ifndef CPPPIPCP_H
#define CPPPIPCP_H

#include "pppdef.h"
#include "CPPPFsm.h"
#include "CPPPProtocol.h"
#include <string>
#include "BaseDefines.h"

class VPN_PUBLIC IPPPIPCPSink 
{
public:
    virtual ~IPPPIPCPSink(){}
    
    virtual void OnIPCPUp() = 0;
    virtual void OnIPCPDown() = 0;
    virtual void OnIPCPPayload(unsigned char *packet, size_t size) = 0;
    virtual void OnIPCPOutput(unsigned char *packet, size_t size) = 0;
};

class VPN_PUBLIC CPPPIPCP : public CPPPProtocol, public IFsmSink
{
public:
    CPPPIPCP(IPPPIPCPSink *psink);
    virtual ~CPPPIPCP();

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

    void InputData(unsigned char *packet, size_t size);
    int State();

    void SendPayload(unsigned char *pkg, size_t size);

    /* Notice for arguments in the following SetXXXX functions:
     * IP/dns (WORD32) -- IP address in network byte order
     */
    void SetMyIP(WORD32 myIP);
    void SetMyIP(std::string &myIP);
    void SetSubscriberIP(WORD32 hisIP);
    void SetSubscriberIP(std::string &hisIP);
    void SetPrimaryDNS(WORD32 dns);
    void SetPrimaryDNS(std::string &dns);
    void SetSecondaryDNS(WORD32 dns);
    void SetSecondaryDNS(std::string &dns);
    
    WORD32 GetSubscriberIP();
    WORD32 GetPrimaryDNS();
    WORD32 GetSecondaryDNS();

private:
    IPPPIPCPSink *m_psink;
    CPPPFsm m_fsm;
    ipcp_options m_ipcp_wantoptions;  /* Options that we want to request */
    ipcp_options m_ipcp_gotoptions;   /* Options that peer ack'd */
    ipcp_options m_ipcp_allowoptions; /* Options we allow peer to request */
    ipcp_options m_ipcp_hisoptions;   /* Options that we ack'd */
    bool m_busepeerdns;       /* Ask peer for DNS addrs */
    bool m_ask_for_local;     /* request our address from peer */
                              // 开源代码ppp-2.4.7中该注释好像有误，我理解为本端是否向对端发送IP选项，或者peer request 
                              // our address
    bool m_noremoteip;        /* Let him have no IP address */
    bool m_ipcp_is_open;
};


#endif//CPPPIPCP_H

