/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
**********************************************************************/

#ifndef CDHCPSERVERBCR_H
#define CDHCPSERVERBCR_H

#include "aceinclude.h"
#include <stdint.h>
#include <unordered_map>
#include "CDHCPSession.h"
#include "IAuthManager.h"

class CDHCPServer;

// Class for broadcast and multicast UDP socket
class CDHCPServerBCR : public ACE_Event_Handler
{
public:
    CDHCPServerBCR(CDHCPServer &server);
    virtual ~CDHCPServerBCR();
    
    int Open(ACE_Reactor *reactor);

    // For interface ACE_Event_Handler
    virtual int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE);
    virtual ACE_HANDLE get_handle (void) const;
    virtual int handle_close (ACE_HANDLE handle,
                            ACE_Reactor_Mask close_mask);

private:
    CDHCPServer &m_server;
    ACE_HANDLE m_handler;
    ACE_Reactor * m_reactor;
};

#endif//CDHCPSERVER_H

