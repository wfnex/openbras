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

#ifndef CTCP_TRANSPORT_H
#define CTCP_TRANSPORT_H
#include "aceinclude.h"

typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> SVC_HANDLER;

class CTCPTransport : public SVC_HANDLER
{
public:
    CTCPTransport(ACE_Reactor *reactor = NULL);
    virtual ~CTCPTransport();
    virtual int open(void *acceptor_or_connector);
    virtual int handle_input (ACE_HANDLE handle);
    virtual int handle_output (ACE_HANDLE handle);
    virtual int handle_close (ACE_HANDLE handle,ACE_Reactor_Mask mask);

    int SendMessage(char *buffer, size_t size);
    int SendMessage (const ACE_Message_Block &Msg);
    int SendMessage (ACE_Message_Block *pMsg);

    virtual size_t OnHandleMessage(const ACE_Message_Block &aData) = 0;
    virtual void OnRcvBufferOverFlow(size_t maxbuffersize) = 0;
    virtual void OnConnected() = 0;
    //virtual void OnPeerDisconnect() = 0;
    //void OnSendBufferOverFlow(size_t maxbuffersize) = 0;

    ACE_INET_Addr &GetRemoteAddress();
    ACE_INET_Addr &GetLocalAddress();
protected:
    int OnReceive_I(const ACE_Message_Block &aData);
    int AppendMessage(ACE_Message_Block *pMessage, ACE_Message_Block *toAdd);
    int FillIov(const ACE_Message_Block *pmsg, iovec aIov[], int aMax) const;
    int SendBuffedMsg();
private:
    ACE_Message_Block *m_sndbuf;
    ACE_Message_Block *m_rcvbuf;
    size_t m_dwRcvBufMaxLen;
    ACE_INET_Addr m_remote_address;
    ACE_INET_Addr m_local_address;
};




#endif//CTCP_TRANSPORT_H
