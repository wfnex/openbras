/*
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
 */

#ifndef CPAPPROTOCOL_H
#define CPAPPROTOCOL_H

#include "aceinclude.h"
#include "CPPPProtocol.h"
#include "BaseDefines.h"
#include "CAuthen.h"

/*
 * Packet header = Code, id, length.
 */
#define UPAP_HEADERLEN	4


/*
 * UPAP codes.
 */
#define UPAP_AUTHREQ	1	/* Authenticate-Request */
#define UPAP_AUTHACK	2	/* Authenticate-Ack */
#define UPAP_AUTHNAK	3	/* Authenticate-Nak */


/*
 * Timeouts.
 */
 #define UPAP_DEFREQTIME	30	/* Time to wait for auth-req from peer */

class CPapProtocolSvr : public CPPPProtocol, public ACE_Event_Handler
{
public:
    CPapProtocolSvr(IAuthenSvrSink *psink);
    virtual ~CPapProtocolSvr();

    // for class CPPPProtocol
    virtual void Init();
    virtual void Input(unsigned char *packet ,size_t size);
    virtual void Protrej();
    virtual void LowerUp();
    virtual void LowerDown();
    virtual void Open();
    virtual void Close(char *reason);

    // for class ACE_Event_Handler
    virtual int handle_timeout (const ACE_Time_Value &current_time,
                                const void *act = 0);    

    void ResponseAuthenResult(int result, std::string &reason);

    std::string &GetUsername() {return m_username;}
    
protected:
    void CancelTimer();
    void StartTimer(int seconds);

    void upap_rauthreq(BYTE *inp, int id, int len);    
    void upap_sresp(BYTE code, BYTE id, CHAR *msg, int msglen);
    
private:
    IAuthenSvrSink *m_psink;
    int m_reqtimeout;       /* Time to wait for auth-req from peer */
    std::string m_username; // Username subscriber input
    int m_usrReqId;         // User request id
};

#endif//CPAPPROTOCOL_H


