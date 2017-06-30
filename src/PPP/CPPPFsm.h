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


#ifndef CPPPFSM_H
#define CPPPFSM_H

#include "pppdef.h"

class CPPPFsm;
class VPN_PUBLIC IFsmSink
{
public:
    virtual ~IFsmSink(){};
    
    virtual void Resetci(CPPPFsm *pfsm) = 0;  /* Reset our Configuration Information */
    virtual int Cilen(CPPPFsm *pfsm) = 0;     /* Length of our Configuration Information */
    virtual void Addci(CPPPFsm *pfsm, u_char *, int *) = 0;      /* Add our Configuration Information */
    virtual int Ackci(CPPPFsm *pfsm, u_char *, int) =0;          /* ACK our Configuration Information */
    virtual int  Nakci(CPPPFsm *pfsm, u_char *, int, int) = 0;   /* NAK our Configuration Information */
    virtual int  Rejci(CPPPFsm *pfsm, u_char *, int) = 0;        /* Reject our Configuration Information */    
    virtual int  Reqci(CPPPFsm *pfsm, u_char *, int *, int) = 0; /* Request peer's Configuration Information */
    virtual void Up(CPPPFsm *pfsm) = 0;                          /* Called when fsm reaches OPENED state */
    virtual void Down(CPPPFsm *pfsm) = 0;                        /* Called when fsm leaves OPENED state */
    virtual void Starting(CPPPFsm *pfsm) = 0;                    /* Called when we want the lower layer */    
    virtual void Finished(CPPPFsm *pfsm) = 0;                    /* Called when we don't want the lower layer */
    virtual void ProtReject(int) = 0;                            /* Called when Protocol-Reject received */
    virtual void Retransmit(CPPPFsm *pfsm)= 0;                   /* Retransmission is necessary */
    virtual int ExtCode(CPPPFsm *pfsm, int, int, u_char *, int)= 0; /* Called when unknown code received */
    virtual void OutputPacket(unsigned char *pkg, size_t size) = 0;
} ;


/*
 * Each FSM is described by an fsm structure and fsm callbacks.
 */
class VPN_PUBLIC CPPPFsm : public ACE_Event_Handler
{
public:
    CPPPFsm(IFsmSink *psink, int protocol, const std::string &protname);
    virtual ~CPPPFsm();
    
    void Init();
    void LowerUp();
    void LowerDown();
    void Open();
    void Close(char *);
    void Input(u_char *, int);
    void Protreject();
    void SendConfReq(int retransmit);
    void SendData(u_char code, 
                    u_char id,
                    u_char *data,
                    int datalen);

    // For class ACE_Event_Handler
    virtual int handle_timeout (const ACE_Time_Value &current_time,
                                const void *act = 0);
    
    void ProtReject();
    void CancelTimer();
    void StartTime(int seconds);
    void Output(u_char *data, int datalen);
    int State();
    void EnableFlag(int flag);
    void DisableFlag(int flag);
	int GetId();
    
protected:
    void TerminateLayer(int nextstate);
    void RcvConfReq(u_char id, u_char *inp, int len);
    void RcvConfAck(int id, u_char *inp, int len);
    void RcvConfNakRej(int code, int id, u_char *inp, int len);
    void RcvTermReq(int id,u_char *p,int len);
    void RcvTermack();
    void RcvCodeReject(u_char *inp,int len);
    void Timeout(void *arg);
    
public:
    int m_protocol;       /* Data Link Layer Protocol field value */
    int m_state;          /* State */
    int m_flags;          /* Contains option bits */
    u_char m_id;          /* Current id */
    u_char m_reqid;           /* Current request id */
    u_char m_seen_ack;        /* Have received valid Ack/Nak/Rej to Req */
    int m_timeouttime;        /* Timeout time in milliseconds */
    int m_maxconfreqtransmits;	/* Maximum Configure-Request transmissions */
    int m_retransmits;		/* Number of retransmissions left */
    int m_maxtermtransmits;	/* Maximum Terminate-Request transmissions */
    int m_nakloops;		/* Number of nak loops since last ack */
    int m_rnakloops;		/* Number of naks received */
    int m_maxnakloops;		/* Maximum number of nak loops tolerated */
    IFsmSink *m_callbacks;	/* Callback routines */
    std::string m_term_reason;		/* Reason for closing protocol */
    int m_term_reason_len;	/* Length of term_reason */
    std::string m_strProtoName;
    unsigned char m_outpacket_buf[PPP_MRU+PPP_HDRLEN];
};


#endif//CPPPFSM_H

