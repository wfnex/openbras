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
#include "CDHCPServerBCR.h"
#include "CDHCPServer.h"

CDHCPServerBCR::CDHCPServerBCR(CDHCPServer &server)
    :m_server(server)
    ,m_handler(ACE_INVALID_HANDLE)
    ,m_reactor(NULL)
{
}

CDHCPServerBCR::~CDHCPServerBCR()
{
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CDHCPServerBCR::~CDHCPServerBCR \n")));

    if (m_reactor)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CDHCPServerBCR::~CDHCPServerBCR, remove_handler \n")));
        m_reactor->remove_handler (this,
                                    ACE_Event_Handler::ALL_EVENTS_MASK |
                                    ACE_Event_Handler::DONT_CALL);
    }
    
    if (m_handler != ACE_INVALID_HANDLE)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CDHCPServerBCR::~CDHCPServerBCR, closesocket \n"))); 
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
    }

}

int CDHCPServerBCR::Open(ACE_Reactor *reactor)
{
    m_reactor = reactor;

    ACE_DEBUG ((LM_DEBUG, "(%P|%t) CDHCPServerBCR::Open\n"));

    m_handler = ACE_OS::socket (AF_INET, SOCK_DGRAM, 0);
    if (m_handler == ACE_INVALID_HANDLE)
    {
        ACE_DEBUG ((LM_ERROR, "(%P|%t) CDHCPServerBCR::Open, handler error\n"));
        return -1;
    }

    int one = 1;
    if (ACE_OS::setsockopt (m_handler,
                            SOL_SOCKET,
                            SO_REUSEADDR,
                            (const char*) &one,
                            sizeof one) == -1)
      {
          ACE_OS::closesocket(m_handler);
          m_handler = ACE_INVALID_HANDLE;
          ACE_DEBUG ((LM_ERROR,"(%P|%t) CDHCPServerBCR::Open, setsockopt SO_REUSEADDR error\n"));
          return -1;
      }

    int broadcastone = 1;
    if (ACE_OS::setsockopt (m_handler,
                            SOL_SOCKET,
                            SO_BROADCAST,
                            (char *)&broadcastone,
                            sizeof broadcastone) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CDHCPServerBCR::Open, setsockopt SO_BROADCAST error\n"));
        return -1;
    }

    ACE_INET_Addr bindip(BOOTP_REQUEST_PORT, "0.0.0.0");
    if (ACE_OS::bind (m_handler,
                             reinterpret_cast<sockaddr *> (bindip.get_addr ()),
                             bindip.get_size ()) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_ERROR((LM_ERROR,
                 ACE_TEXT ("(%P|%t) CDHCPServerBCR::open: %p\n"),
                 ACE_TEXT ("bind error")));
    
        return -1;
    }

    int size = 262144;
    if (ACE_OS::setsockopt(m_handler, SOL_SOCKET,
                                    SO_RCVBUF,
                                    (char *) &size,
                                    sizeof (size)) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CDHCPServerBCR::Open, setsockopt SO_RCVBUF error. size = %d\n", size));
        return -1;
    }

    if (ACE_OS::setsockopt(m_handler, SOL_SOCKET,
                                    SO_SNDBUF,
                                    (char *) &size,
                                    sizeof (size)) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CDHCPServerBCR::Open, setsockopt SO_SNDBUF error. size = %d\n", size));
        return -1;
    }

    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPServerBCR::Open, register_handler %x\n",m_reactor));

    // Register with the reactor for input.
    if (m_reactor->register_handler (this,ACE_Event_Handler::READ_MASK) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_ERROR_RETURN ((LM_ERROR,
                 ACE_TEXT ("(%P|%t) CDHCPServerBCR::open: %p\n"),
                 ACE_TEXT ("register_handler for input")),
                -1);
    }

    return 0;
}

//handle for input
int CDHCPServerBCR::handle_input (ACE_HANDLE fd)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPServerBCR::handle_input fd=%d\n",fd));

    return m_server.handle_input(fd);
}

// Get handle
ACE_HANDLE CDHCPServerBCR::get_handle (void) const
{
    return m_handler;
}
// Close handle
int CDHCPServerBCR::handle_close (ACE_HANDLE handle,
                        ACE_Reactor_Mask close_mask)
{
    return 0;
}


