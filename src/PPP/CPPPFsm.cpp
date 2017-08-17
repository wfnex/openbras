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


#include "CPPPFsm.h"

CPPPFsm::CPPPFsm(IFsmSink *psink, int protocol, const std::string &protname)
    :m_protocol(protocol)
    ,m_state(INITIAL)
    ,m_flags(0)
    ,m_id(0)
    ,m_reqid(0)
    ,m_seen_ack(0)
    ,m_timeouttime(DEFTIMEOUT)
    ,m_maxconfreqtransmits(DEFMAXCONFREQS)
    ,m_retransmits(0)
    ,m_maxtermtransmits(DEFMAXTERMREQS)
    ,m_nakloops(0)
    ,m_rnakloops(0)
    ,m_maxnakloops(DEFMAXNAKLOOPS)
    ,m_callbacks(psink)
    ,m_term_reason_len(0)
    ,m_strProtoName(protname)
{
    ::memset(m_outpacket_buf, 0, sizeof(m_outpacket_buf));
}

CPPPFsm::~CPPPFsm()
{
    ACE_DEBUG ((LM_DEBUG, "CPPPFsm::~CPPPFsm\n"));
    CancelTimer();
}

void
CPPPFsm::Init()
{
    m_state = INITIAL;
    m_flags = 0;
    m_id = 0;				/* XXX Start with random id? */
    m_timeouttime = DEFTIMEOUT;
    m_maxconfreqtransmits = DEFMAXCONFREQS;
    m_maxtermtransmits = DEFMAXTERMREQS;
    m_maxnakloops = DEFMAXNAKLOOPS;
    m_term_reason_len = 0;
}


/*
 * fsm_lowerup - The lower layer is up.
 */
void
CPPPFsm::LowerUp()
{
    switch (m_state)
    {
        case INITIAL:
        {
            this->m_state = CLOSED;
            break;
        }
        case STARTING:
        {
            if ( this->m_flags & OPT_SILENT )
            {
                this->m_state = STOPPED;
            }
            else 
            {
                /* Send an initial configure-request */
                SendConfReq(0);
                this->m_state = REQSENT;
            }
            break;
        }
    }
}


/*
 * fsm_lowerdown - The lower layer is down.
 *
 * Cancel all timeouts and inform upper layers.
 */
void
CPPPFsm::LowerDown()
{
    switch ( this->m_state )
    {
        case CLOSED:
        {
            this->m_state = INITIAL;
            break;
        }
        case STOPPED:
        {
            this->m_state = STARTING;
            m_callbacks->Starting(this);
            break;
        }
        case CLOSING:
        {
            this->m_state = INITIAL;
            CancelTimer();
            break;
        }
        case STOPPING:
        case REQSENT:
        case ACKRCVD:
        case ACKSENT:
        {
            this->m_state = STARTING;
            CancelTimer();
            break;
        }
        case OPENED:
        {
            m_callbacks->Down(this);

            this->m_state = STARTING;
            break;
        }
    }
}

//Cancel Timer
void CPPPFsm::CancelTimer()
{
    ACE_DEBUG ((LM_DEBUG, "CPPPFsm::CancelTimer\n"));
    ACE_Reactor::instance()->cancel_timer(this);
}

//Start Timer
void CPPPFsm::StartTime(int seconds)
{
    ACE_DEBUG ((LM_DEBUG, "CPPPFsm::StartTime, seconds = %d\n", seconds));
    
    CancelTimer();
    
    ACE_Time_Value delay(seconds);
    ACE_Reactor::instance()->schedule_timer(this, 0, delay, delay); 
}

/*
 * fsm_open - Link is allowed to come up.
 */
void
CPPPFsm::Open()
{
    switch( this->m_state ){
        case INITIAL:
        this->m_state = STARTING;
        m_callbacks->Starting(this);

        break;

        case CLOSED:
        if( this->m_flags & OPT_SILENT )
            this->m_state = STOPPED;
        else {
            /* Send an initial configure-request */
            SendConfReq(0);
            this->m_state = REQSENT;
        }
        break;

        case CLOSING:
        this->m_state = STOPPING;
        /* fall through */
        case STOPPED:
        case OPENED:
        if( this->m_flags & OPT_RESTART ){
            LowerDown();
            LowerUp();
        }
        break;
    }
}

/*
 * terminate_layer - Start process of shutting down the FSM
 *
 * Cancel any timeout running, notify upper layers we're done, and
 * send a terminate-request message as configured.
 */
void
CPPPFsm::TerminateLayer(int nextstate)
{
    ACE_DEBUG((LM_DEBUG, "CPPPFsm::TerminateLayer, nextstate=%d\n", nextstate));
    
    if ( this->m_state != OPENED )
    {
        CancelTimer();
    }
    
    m_callbacks->Down(this);/* Inform upper layers we're down */

    /* Init restart counter and send Terminate-Request */
    this->m_retransmits = this->m_maxtermtransmits;

    ACE_DEBUG((LM_DEBUG, 
               "send terminate request, protocol=%#x, reqId=%d, reason=%.*s\n", 
               m_protocol, m_id + 1, m_term_reason_len, m_term_reason.c_str()));
    
    SendData(TERMREQ, 
             this->m_reqid = ++this->m_id,
	         (u_char *) this->m_term_reason.c_str(), 
	         this->m_term_reason.size());

    if (this->m_retransmits == 0) 
    {
    	/*
    	 * User asked for no terminate requests at all; just close it.
    	 * We've already fired off one Terminate-Request just to be nice
    	 * to the peer, but we're not going to wait for a reply.
    	 */
    	this->m_state = nextstate == CLOSING ? CLOSED : STOPPED;
	    m_callbacks->Finished(this);
        
    	return;
    }

    StartTime(this->m_timeouttime);
    --this->m_retransmits;

    this->m_state = nextstate;
}

/*
 * fsm_close - Start closing connection.
 *
 * Cancel timeouts and either initiate close or possibly go directly to
 * the CLOSED state.
 */
void
CPPPFsm::Close(char *reason)
{
    this->m_term_reason = std::string(reason, strlen(reason));

    this->m_term_reason_len = (reason == NULL ? 0 : strlen(reason));
    
    switch ( this->m_state )
    {
        case STARTING:
        {
            this->m_state = INITIAL;
            break;
        }
        case STOPPED:
        {
            this->m_state = CLOSED;
            break;
        }
        case STOPPING:
        {
            this->m_state = CLOSING;
            break;
        }
        case REQSENT:
        case ACKRCVD:
        case ACKSENT:
        case OPENED:
        {
            TerminateLayer(CLOSING);
            break;
        }
    }
}

//Timeout Handle
int CPPPFsm::handle_timeout (const ACE_Time_Value &current_time,
                            const void *act)
{
    Timeout(NULL);
}

/*
 * fsm_timeout - Timeout expired.
 */
void
CPPPFsm::Timeout(void *arg)
{
    ACE_DEBUG((LM_DEBUG, "CPPPFsm::Timeout(), state=%d\n", this->m_state));
    
    switch (this->m_state) {
    case CLOSING:
    case STOPPING:
	if( this->m_retransmits <= 0 ){
	    /*
	     * We've waited for an ack long enough.  Peer probably heard us.
	     */
	    this->m_state = (this->m_state == CLOSING)? CLOSED: STOPPED;
		m_callbacks->Finished(this);
	} else {
	    /* Send Terminate-Request */
	    SendData(TERMREQ, this->m_reqid = ++this->m_id,
		      (u_char *) this->m_term_reason.c_str(), this->m_term_reason_len);
	    StartTime(this->m_timeouttime);
	    --this->m_retransmits;
	}
	break;

    case REQSENT:
    case ACKRCVD:
    case ACKSENT:
	if (this->m_retransmits <= 0) {
	    this->m_state = STOPPED;
	    if( (this->m_flags & OPT_PASSIVE) == 0)
		m_callbacks->Finished(this);
	} else {
	    /* Retransmit the configure-request */
		m_callbacks->Retransmit(this);
	    SendConfReq(1);		/* Re-send Configure-Request */
	    if( this->m_state == ACKRCVD )
		this->m_state = REQSENT;
	}
	break;
    }
}

/*
 * fsm_input - Input packet.
 */
void
CPPPFsm::Input(u_char *inpacket, int l)
{
    u_char *inp;
    u_char code, id;
    int len;

    /*
     * Parse header (code, id and length).
     * If packet too short, drop it.
     */
    inp = inpacket;
    
    if (l < HEADERLEN) 
    {
        ACE_DEBUG((LM_ERROR, "CPPPFsm::Input, argument l too short(%d)\n", l));
        return;
    }
    
    GETCHAR(code, inp);
    GETCHAR(id, inp);
    GETSHORT(len, inp);
    
    if (len < HEADERLEN) 
    {
        ACE_DEBUG((LM_ERROR, "CPPPFsm::Input, length field in the pkt too short(%d)\n", len));
        return;
    }
    
    if (len > l) 
    {
        ACE_DEBUG((LM_ERROR, "CPPPFsm::Input, invalid length, len=%d, l=%d\n", len, l));
        return;
    }
    
    len -= HEADERLEN;		/* subtract header length */

    if( this->m_state == INITIAL || this->m_state == STARTING )
    {
        ACE_DEBUG((LM_INFO, "CPPPFsm::Input, improper state(%d), just discard the pkt\n", this->m_state));
        return;
    }

    /*
     * Action depends on code.
     */
    switch (code) 
    {
        case CONFREQ:
        {
            RcvConfReq(id, inp, len);
            break;
        }
        case CONFACK:
        {
            RcvConfAck(id, inp, len);
            break;
        }
        case CONFNAK:
        case CONFREJ:
        {
            RcvConfNakRej(code, id, inp, len);
            break;
        }
        case TERMREQ:
        {
            RcvTermReq(id, inp, len);
            break;
        }
        case TERMACK:
        {
            RcvTermack();
            break;
        }
        case CODEREJ:
        {
            RcvCodeReject(inp, len);
            break;
        }
        default:
        {
            if (!m_callbacks->ExtCode(this, code, id, inp, len) )
            {
                SendData(CODEREJ, ++this->m_id, inpacket, len + HEADERLEN);
            }
            break;
        }
    }
}


/*
 * fsm_rconfreq - Receive Configure-Request.
 */
void
CPPPFsm::RcvConfReq(u_char id, u_char *inp, int len)
{
    int code, reject_if_disagree;

    /* 对此switch语句的说明: 不能为了编程规范的要求而简单的加上default分支。
     * 原因: default包含以下两类状态: (Initial   Starting)、(Req-Sent  Ack-Rcvd Ack-Sent)，第一类在PPPXCP.Input中直接返回
     * 了，不会走到这里。但不能仅仅依赖外部函数。第二类需要执行switch下面的分支。
     * 如果要改，可以这样改:
     * case INITIAL:
     * case STARTING:
     *      return;
     * case REQSENT:
     * case ACKRCVD:
     * case ACKSENT:
     * default:
     *      break;
     */
    switch ( this->m_state )
    {
        case CLOSED:
        {
            /* Go away, we're closed */
            SendData(TERMACK, id, NULL, 0);
        	return;
        }
        case STOPPED:
        {
            /* Negotiation started by our peer */
            SendConfReq(0);		/* Send initial Configure-Request */
            this->m_state = REQSENT;
            break;
        }
        case CLOSING:
        case STOPPING:
        {
        	return;
        }
        case OPENED:
        {
            m_callbacks->Down(this);/* Inform upper layers */

            SendConfReq(0);		/* Send initial Configure-Request */
            this->m_state = REQSENT;
            break;
        }
    }

    /*
     * Pass the requested configuration options
     * to protocol-specific code for checking.
     */
    reject_if_disagree = (this->m_nakloops >= this->m_maxnakloops);
    code = m_callbacks->Reqci(this, inp, &len, reject_if_disagree);
    if (code == -1)
    {
        if (len)
            code = CONFREJ; /* Reject all CI */
        else
            code = CONFACK;
    }

    /* send the Ack, Nak or Rej to the peer */
    SendData(code, id, inp, len);

    if (code == CONFACK) 
    {
    	if (this->m_state == ACKRCVD) 
        {
    	    CancelTimer();	/* Cancel timeout */
    	    this->m_state = OPENED;
            m_callbacks->Up(this);/* Inform upper layers */
    	} 
        else
    	{
    	    this->m_state = ACKSENT;
    	}
        
    	this->m_nakloops = 0;
    } 
    else 
    {
    	/* we sent CONFACK or CONFREJ */
    	if (this->m_state != ACKRCVD)
    	{
    	    this->m_state = REQSENT;
    	}
    	if ( code == CONFNAK )
    	{
    	    ++this->m_nakloops;
    	}
    }
}


/*
 * fsm_rconfack - Receive Configure-Ack.
 */
void
CPPPFsm::RcvConfAck(int id, u_char *inp, int len)
{
    if (id != this->m_reqid || this->m_seen_ack)/* Expected id? */
    {
        ACE_DEBUG((LM_ERROR, 
                   "CPPPFsm::RcvConfAck, id dismatch or already received valid Ack/Nak/Rej to Req\n"));
        ACE_DEBUG((LM_ERROR, 
                   "id=%d, this->m_reqid=%u, this->m_seen_ack=%u\n",
                   id, this->m_reqid, this->m_seen_ack));
                   
        return;     /* Nope, toss... */
    }
    
    if(!m_callbacks->Ackci(this, inp, len))
    {
        /* Ack is bad - ignore it */
        ACE_DEBUG((LM_ERROR, "CPPPFsm::RcvConfAck, Ack is bad\n"));
        
        return;
    }
    
    this->m_seen_ack = 1;
    this->m_rnakloops = 0;

    switch (this->m_state) 
    {
        case CLOSED:
        case STOPPED:
        {
            SendData(TERMACK, id, NULL, 0);
            break;
        }
        case REQSENT:
        {
            this->m_state = ACKRCVD;
            this->m_retransmits = this->m_maxconfreqtransmits;
            
            break;
        }
        case ACKRCVD:
        {
            /* Huh? an extra valid Ack? oh well... */
            CancelTimer();	/* Cancel timeout */
            SendConfReq(0);
            this->m_state = REQSENT;
            
            break;
        }
        case ACKSENT:
        {
            CancelTimer();	/* Cancel timeout */
            this->m_state = OPENED;
            this->m_retransmits = this->m_maxconfreqtransmits;
            m_callbacks->Up(this);  /* Inform upper layers */

            break;
        }
        case OPENED:
        {
            /* Go down and restart negotiation */
            m_callbacks->Down(this);    /* Inform upper layers */

            SendConfReq(0);		/* Send initial Configure-Request */
            this->m_state = REQSENT;
            
            break;
        }
    }
}

/*
 * fsm_rconfnakrej - Receive Configure-Nak or Configure-Reject.
 */
void
CPPPFsm::RcvConfNakRej(int code, int id, u_char *inp, int len)
{
    int ret;
    int treat_as_reject;

    if (id != this->m_reqid || this->m_seen_ack)    /* Expected id? */
    {
        ACE_DEBUG((LM_ERROR, "CPPPFsm::RcvConfNakRej, id dismatch or already received valid Ack/Nak/Rej to Req\n"));
        ACE_DEBUG((LM_ERROR, "id=%d, this->m_reqid=%u, this->m_seen_ack=%u\n", id, m_reqid, m_seen_ack));
        
        return; /* Nope, toss... */
    }

    if (code == CONFNAK) 
    {
        ++this->m_rnakloops;
        treat_as_reject = (this->m_rnakloops >= this->m_maxnakloops);
        
        if (!(ret = m_callbacks->Nakci(this, inp, len, treat_as_reject))) 
        {
            ACE_DEBUG((LM_ERROR, "CPPPFsm::RcvConfNakRej, received bad nak.\n"));           
            return;
        }
    } 
    else 
    {
        this->m_rnakloops = 0;
        if (!(ret = m_callbacks->Rejci(this, inp, len)))
        {
            ACE_DEBUG((LM_ERROR, "CPPPFsm::RcvConfNakRej, received bad reject.\n"));           
            return;
        }
    }

    this->m_seen_ack = 1;

    switch (this->m_state) 
    {
        case CLOSED:
        case STOPPED:
            SendData(TERMACK, id, NULL, 0);
            break;

        case REQSENT:
        case ACKSENT:
            /* They didn't agree to what we wanted - try another request */
            CancelTimer();
            if (ret < 0)
                this->m_state = STOPPED;		/* kludge for stopping CCP */
            else
                SendConfReq(0);		/* Send Configure-Request */
            break;

        case ACKRCVD:
            /* Got a Nak/reject when we had already had an Ack?? oh well... */
            CancelTimer();	/* Cancel timeout */
            SendConfReq(0);
            this->m_state = REQSENT;
            break;

        case OPENED:
            /* Go down and restart negotiation */
            m_callbacks->Down(this);	/* Inform upper layers */
            SendConfReq(0);		/* Send initial Configure-Request */
            this->m_state = REQSENT;
            break;
    }
}


/*
 * fsm_rtermreq - Receive Terminate-Req.
 */
void
CPPPFsm::RcvTermReq(int id,
                    u_char *p,
                    int len)
{
    switch (this->m_state) 
    {
        case ACKRCVD:
        case ACKSENT:
        {
            this->m_state = REQSENT;    /* Start over but keep trying */
            break;
        }
        case OPENED:
        {
            if (len > 0) 
            {
                ACE_DEBUG ((LM_DEBUG,"%s terminated by peer (%.*s)\n", m_strProtoName.c_str(), len, p));
            } 
            else
            {
                ACE_DEBUG ((LM_DEBUG,"%s terminated by peer\n", m_strProtoName.c_str()));
            }
            
            this->m_retransmits = 0;
            this->m_state = STOPPING;
            
            SendData(TERMACK, id, NULL, 0); // Revised by mazhh as m_callbacks->Down will free the session memory, where
                                            // CPPPLCP and CPPPFsm lie.
            m_callbacks->Down(this);        /* Inform upper layers */
            StartTime(this->m_timeouttime);
            
            return;
        }
    }

    SendData(TERMACK, id, NULL, 0);
}


/*
 * fsm_rtermack - Receive Terminate-Ack.
 */
void
CPPPFsm::RcvTermack()
{
    switch (this->m_state) 
    {
        case CLOSING:
            CancelTimer();
            this->m_state = CLOSED;
            if (m_callbacks)
            {
                m_callbacks->Finished(this);
            }
            break;
        case STOPPING:
            CancelTimer();
            this->m_state = STOPPED;
            if (m_callbacks)
            {
                m_callbacks->Finished(this);
            }
            break;

        case ACKRCVD:
            this->m_state = REQSENT;
            break;

        case OPENED:
            if (m_callbacks)
            {
                m_callbacks->Down(this);/* Inform upper layers */
            }
            SendConfReq(0);
            this->m_state = REQSENT;
            break;
    }
}


/*
 * fsm_rcoderej - Receive an Code-Reject.
 */
void
CPPPFsm::RcvCodeReject(u_char *inp, int len)
{
    u_char code, id;

    if (len < HEADERLEN) 
    {
        ACE_DEBUG ((LM_ERROR, "CPPPFsm::RcvCodeReject: Rcvd short Code-Reject packet! len=%d\n", len));
        
        return;
    }
    
    GETCHAR(code, inp);
    GETCHAR(id, inp);
    
    ACE_DEBUG ((LM_DEBUG,"%s: Rcvd Code-Reject for code %d, id %d\n", m_strProtoName.c_str(), m_state, id));
    
    if ( this->m_state == ACKRCVD )
    {
        this->m_state = REQSENT;
    }
}


/*
 * fsm_protreject - Peer doesn't speak this protocol.
 *
 * Treat this as a catastrophic error (RXJ-).
 */
void
CPPPFsm::ProtReject()
{
    switch( this->m_state )
    {
        case CLOSING:
            CancelTimer();
        /* fall through */
        case CLOSED:
            this->m_state = CLOSED;
            m_callbacks->Finished(this);

            break;

        case STOPPING:
        case REQSENT:
        case ACKRCVD:
        case ACKSENT:
            CancelTimer();
        /* fall through */
        case STOPPED:
            this->m_state = STOPPED;
            m_callbacks->Finished(this);

            break;

        case OPENED:
            TerminateLayer(STOPPING);
            break;
        default:
            ACE_DEBUG ((LM_DEBUG,"CPPPFsm::ProtReject %s: Protocol-reject event in state %d!",
                  m_strProtoName.c_str(), m_state));

    }
}


/*
 * fsm_sconfreq - Send a Configure-Request.
 */
void
CPPPFsm::SendConfReq(int retransmit)
{
    u_char *outp;
    int cilen;

    if( this->m_state != REQSENT && this->m_state != ACKRCVD && this->m_state != ACKSENT ){
        /* Not currently negotiating - reset options */
        m_callbacks->Resetci(this);

        this->m_nakloops = 0;
        this->m_rnakloops = 0;
    }

    if( !retransmit ){
        /* New request - reset retransmission counter, use new ID */
        this->m_retransmits = this->m_maxconfreqtransmits;
        this->m_reqid = ++this->m_id;
    }

    this->m_seen_ack = 0;

    /*
     * Make up the request packet
     */
    outp = m_outpacket_buf + PPP_HDRLEN + HEADERLEN;
    cilen = m_callbacks->Cilen(this);
    if (cilen >PPP_MRU - HEADERLEN)
    {
        cilen = PPP_MRU - HEADERLEN;
    }

    m_callbacks->Addci(this,outp,&cilen);

    /* send the request to our peer */
    SendData(CONFREQ, this->m_reqid, outp, cilen);

    /* start the retransmit timer */
    --this->m_retransmits;
    StartTime(this->m_timeouttime);
}


/*
 * fsm_sdata - Send some data.
 *
 * Used for all packets sent to our peer by this module.
 */
void
CPPPFsm::SendData(u_char code, 
                  u_char id,
                  u_char *data,
                  int datalen)
{
    unsigned char *outp = NULL;
    int outlen = 0;

    /* Adjust length to be smaller than MTU */
    outp = m_outpacket_buf;
    
    if (datalen > PPP_MRU - HEADERLEN)
    {
        datalen = PPP_MRU - HEADERLEN;
    }
    
    if (datalen && data != outp + PPP_HDRLEN + HEADERLEN)
    {
        BCOPY(data, outp + PPP_HDRLEN + HEADERLEN, datalen);
    }
    
    outlen = datalen + HEADERLEN;
    MAKEHEADER(outp, this->m_protocol);
    PUTCHAR(code, outp);
    PUTCHAR(id, outp);
    PUTSHORT(outlen, outp);
    Output(m_outpacket_buf, outlen + PPP_HDRLEN);
}

//Output Packet
void CPPPFsm::Output(u_char *data, int datalen)
{
    if (m_callbacks)
    {
        m_callbacks->OutputPacket(data, datalen);
    }
    return;
}
//Fsm status
int CPPPFsm::State()
{
    return m_state;
}

//Enable Flag
void CPPPFsm::EnableFlag(int flag)
{
    m_flags |=flag;
}

//Disable Flag
void CPPPFsm::DisableFlag(int flag)
{
    m_flags &=~flag;
}

//Get Id
int CPPPFsm::GetId()
{
	m_id++;
    return m_id;
}



