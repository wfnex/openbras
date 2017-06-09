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

int CDHCPServerBCR::handle_input (ACE_HANDLE fd)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CDHCPServerBCR::handle_input fd=%d\n",fd));

    return m_server.handle_input(fd);
}
ACE_HANDLE CDHCPServerBCR::get_handle (void) const
{
    return m_handler;
}
int CDHCPServerBCR::handle_close (ACE_HANDLE handle,
                        ACE_Reactor_Mask close_mask)
{
    return 0;
}


