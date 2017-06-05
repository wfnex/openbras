/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
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
