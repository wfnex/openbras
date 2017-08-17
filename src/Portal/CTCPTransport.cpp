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

#include "CTCPTransport.h"

CTCPTransport::CTCPTransport(ACE_Reactor *reactor)
    :SVC_HANDLER(0,0,reactor)
    ,m_sndbuf(0)
    ,m_rcvbuf(0)
    ,m_dwRcvBufMaxLen(1024*32)
{
    ACE_DEBUG ((LM_DEBUG,"CTCPTransport::CTCPTransport\n"));
    if (reactor == NULL)
    {
        ACE_ASSERT(0);
    }
}

CTCPTransport::~CTCPTransport()
{
    if (m_sndbuf)
    {
        m_sndbuf->release();
        m_sndbuf = NULL;
    }

    if (m_rcvbuf)
    {
        m_rcvbuf->release();
        m_rcvbuf = NULL;
    }

    if (reactor ())
    {
        reactor ()->remove_handler(this,
            ACE_Event_Handler::READ_MASK | ACE_Event_Handler::WRITE_MASK|
            ACE_Event_Handler::DONT_CALL);
    }
    ACE_DEBUG ((LM_DEBUG,"CTCPTransport::~CTCPTransport\n"));

}



int CTCPTransport::open(void *acceptor_or_connector)
{

    ACE_TCHAR buf[BUFSIZ];
    ACE_DEBUG ((LM_DEBUG,"CTCPTransport::Open\n"));


    int bufsiz = 65535;
    if (this->peer_.set_option (SOL_SOCKET, SO_RCVBUF,&bufsiz, sizeof bufsiz))
    {
        ACE_ERROR ((LM_ERROR, "%p\n", "set_option buffer"));
    }

    if (this->peer_.set_option (SOL_SOCKET, SO_SNDBUF,&bufsiz, sizeof bufsiz))
    {
        ACE_ERROR ((LM_ERROR, "%p\n", "set_option buffer"));
    }
    
    int nodelay = 1;
    if (this->peer_.set_option (ACE_IPPROTO_TCP,
                                  TCP_NODELAY,
                                  &nodelay,
                                  sizeof (nodelay)))
    {
        ACE_ERROR ((LM_ERROR, "%p\n", "set_option"));
    }
    
    //stream.get_local_addr (m_local_address);
    //stream.get_remote_addr (m_remote_address);
    
    if (this->peer_.get_remote_addr (m_remote_address) == -1)
    {
        ACE_ERROR_RETURN ((LM_ERROR,
                         ACE_TEXT ("%p\n"),
                         ACE_TEXT ("get_remote_addr")),
                        -1);
    }
    
    if (m_remote_address.addr_to_string (buf, sizeof buf) == -1)
    {
        ACE_ERROR_RETURN ((LM_ERROR,
                         ACE_TEXT ("%p\n"),
                         ACE_TEXT ("can't obtain peer's address")),
                        -1);
    }
    
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("connected to %s on fd %d\n"),
                buf,
                this->peer_.get_handle ()));

    
    if (this->reactor () && this->reactor ()->register_handler(this,
        ACE_Event_Handler::READ_MASK | ACE_Event_Handler::WRITE_MASK) == -1)
    {
        ACE_ERROR_RETURN ((LM_ERROR,
                           ACE_TEXT ("%p\n"),
                           ACE_TEXT ("unable to register CTCPTransport handler")),
                          -1);
    }

    OnConnected();

    return 0;
}


//static CMessageBlock szBuf(16*1024);
//Input Handle
int CTCPTransport::handle_input (ACE_HANDLE handle)
{
    for(;;)
    {
        ACE_Message_Block szBuf(16*1024);
        int bytes_read = ACE_OS::recv(get_handle(),szBuf.rd_ptr (), szBuf.space (), 0);
        if (bytes_read == -1)
        {
            if (errno == EWOULDBLOCK)
            {
                //ACE_DEBUG ((LM_DEBUG,"COpenFlowReactorSwitch::handle_input woudblock\n"));
                break;
            }
            else
            {
                ACE_ERROR_RETURN ((LM_ERROR,
                ACE_TEXT ("(%t) %p\n"),
                ACE_TEXT ("CTCPTransport::handle_input")),
                -1);
            }
        }
        else if (bytes_read == 0)
        {
            ACE_ERROR_RETURN ((LM_ERROR,
            ACE_TEXT ("(%t) %p\n"),
            ACE_TEXT ("CTCPTransport::handle_input, byte_read=0")),
            -1);
        }
        ACE_DEBUG ((LM_DEBUG,"CTCPTransport::handle_input, bytes_read=%d\n",bytes_read));

        //CMessageBlock *mblk = new CMessageBlock(szBuf, bytes_read);
        szBuf.wr_ptr (bytes_read);
        
        if (this->OnReceive_I(szBuf) == -1)
        {
            ACE_DEBUG ((LM_DEBUG,"CTCPTransport::handle_input, OnReceive_I error"));
            return -1;
        }

    }

    return 0;
}

//Append Message
int CTCPTransport::AppendMessage(ACE_Message_Block *pMessage, ACE_Message_Block *toAdd)
{
    ACE_Message_Block *lastBuffer = pMessage;
    //find last one, add add msg 
    while (lastBuffer->cont ())
    {
        lastBuffer = lastBuffer->cont () ;
    }
    lastBuffer->cont (toAdd);

    return 0;
}

//Receive Data
int CTCPTransport::OnReceive_I(const ACE_Message_Block &aData)
{
    ACE_DEBUG ((LM_DEBUG,"CTCPTransport::OnReceive_I,length=%d\n",aData.length()));

    if (m_rcvbuf == NULL)
    {
        m_rcvbuf = aData.duplicate();
    }
    else
    {
        AppendMessage(m_rcvbuf, aData.duplicate());
    }

    if (m_rcvbuf->total_length() > m_dwRcvBufMaxLen)
    {
        ACE_DEBUG ((LM_DEBUG,"CTCPTransport::OnReceive_I,buffer over flow,size=%d\n",m_dwRcvBufMaxLen));
        OnRcvBufferOverFlow(m_dwRcvBufMaxLen);
        return -1;
    }

    
    ACE_DEBUG ((LM_DEBUG,"CTCPTransport::OnReceive_I, total_length=%d\n",m_rcvbuf->total_length()));

    while(m_rcvbuf && m_rcvbuf->total_length() >= 0)
    {
        ACE_DEBUG ((LM_DEBUG,"CTCPTransport::OnReceive_I looper,totalsize=%d\n", 
        m_rcvbuf->total_length()));

        //CMessageBlock *pTmp = m_rcvbuf->DuplicateChained();
        size_t totalsize = OnHandleMessage(*m_rcvbuf);

        ACE_DEBUG ((LM_DEBUG,"CTCPTransport::OnReceive_I wLength=%d, m_rcvbuf totalsize=%d, length()=%d\n", 
        totalsize, m_rcvbuf->total_length(), m_rcvbuf->length ()));


        if (totalsize == 0)
        {
            break;
        }

        while (m_rcvbuf &&totalsize>= m_rcvbuf->length ())
        {
            ACE_Message_Block *buffer = m_rcvbuf;
            totalsize -= m_rcvbuf->length ();
            m_rcvbuf= m_rcvbuf->cont ();
            buffer->cont (0);
            buffer->release ();
        }
        
        if (m_rcvbuf)
        {
            m_rcvbuf->rd_ptr (totalsize);
        }
        
    }

    return 0;
}

//Output Handle
int CTCPTransport::handle_output (ACE_HANDLE handle)
{
    SendBuffedMsg();

    return 0;
}

//Close Handle
int CTCPTransport::handle_close (ACE_HANDLE handle,ACE_Reactor_Mask mask)
{
    //OnPeerDisconnect();
    SVC_HANDLER::handle_close (handle, mask);
    return 0;
}

//Send Message
int CTCPTransport::SendMessage(char *buffer, size_t size)
{
    ACE_Message_Block *newblk = new ACE_Message_Block(size);
    ::memcpy(newblk->rd_ptr(), buffer, size);
    newblk->wr_ptr(size);

    return SendMessage(newblk);
}


int CTCPTransport::SendMessage (const ACE_Message_Block &Msg)
{
    ACE_Message_Block *pdup = NULL;
    if (ACE_BIT_DISABLED (Msg.flags(),
                          ACE_Message_Block::DONT_DELETE))
    {
        pdup = Msg.duplicate();
    }
    else
    {
        pdup = Msg.clone();
    }
    return SendMessage(pdup);
}

int CTCPTransport::SendMessage (ACE_Message_Block *pMsg)
{
    if (m_sndbuf == NULL)
    {
        m_sndbuf = pMsg;
    }
    else
    {
        AppendMessage(m_sndbuf,pMsg);
    }
    return SendBuffedMsg(); 
}

//Fill Iov
int CTCPTransport::FillIov(const ACE_Message_Block *pmsg, iovec aIov[], int aMax) const
{
    int iovcnt = 0;

    const ACE_Message_Block *current_message_block = pmsg;

    while (current_message_block != 0)
    {
        aIov[iovcnt].iov_base = current_message_block->rd_ptr ();
        aIov[iovcnt].iov_len  = current_message_block->length ();
        ++iovcnt;

        if (iovcnt == aMax)
        {
            break;
        }

        // Select the next message block in the chain.
        current_message_block = current_message_block->cont ();
    }

    return iovcnt;

}

//Send Buffer Message
int CTCPTransport::SendBuffedMsg()
{
    if (m_sndbuf == NULL)
    {
        return 0;
    }
    size_t totalsize = 0;

    iovec iov[ACE_IOV_MAX];
    int iovcnt = FillIov(m_sndbuf, iov,ACE_IOV_MAX);

    ACE_DEBUG 
    ((LM_DEBUG,"CTCPTransport::SendBuffedMsg,m_sndbuf=%d, iovcnt=%d\n",
    m_sndbuf->total_length(),
    iovcnt));

    totalsize = ACE_OS::writev(this->get_handle(),
                                        iov,
                                        iovcnt);

    if (totalsize == -1)
    {
        if (errno == EWOULDBLOCK)
        {
            return 0;
        }
        else
        {
          ACE_ERROR_RETURN ((LM_ERROR,
                             ACE_TEXT ("(%t) %p\n"),
                             ACE_TEXT ("CTCPTransport::Send_i ss")),
                            -1);
        }
    }
    else if (totalsize == 0)
    {
            ACE_ERROR_RETURN ((LM_ERROR,
                             ACE_TEXT ("(%t) %p\n"),
                             ACE_TEXT ("CTCPTransport::Send_i")),
                            -1);
    }
    ACE_DEBUG ((LM_DEBUG,"CTCPTransport::SendBuffedMsg,totalsize=%d\n",totalsize));

    while (m_sndbuf &&totalsize>= m_sndbuf->length ())
    {
        ACE_Message_Block *buffer = m_sndbuf;
        totalsize -= m_sndbuf->length ();
        m_sndbuf= m_sndbuf->cont ();
        buffer->cont (0);
        buffer->release ();
    }

    if (m_sndbuf)
    {
        m_sndbuf->rd_ptr (totalsize);
    }


    //ACE_DEBUG ((LM_DEBUG,"CTCPTransport::SendBuffedMsg, end(),m_sndbuf=%d\n",m_sndbuf->total_length()));

    return 0;
}

//Get Remote Address
ACE_INET_Addr &CTCPTransport::GetRemoteAddress()
{
    return m_remote_address;
}

//Get Local Address
ACE_INET_Addr &CTCPTransport::GetLocalAddress()
{
    return m_local_address;
}



