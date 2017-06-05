/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
**********************************************************************/

#include "CDHCPAuthRespEventHndl.h"
#include "CDHCPServer.h"

CDHCPAuthRespEventHndl::CDHCPAuthRespEventHndl(CDHCPServer &server, const Auth_Response *response)
    : m_server(server)
{
    ACE_DEBUG ((LM_DEBUG,"CDHCPAuthRespEventHndl::CDHCPAuthRespEventHndl\n")); 
    ::memcpy(&m_response, response, sizeof(*response));
}

CDHCPAuthRespEventHndl::~CDHCPAuthRespEventHndl()
{
    ACE_DEBUG ((LM_DEBUG,"CDHCPAuthRespEventHndl::~CDHCPAuthRespEventHndl\n")); 
}

void CDHCPAuthRespEventHndl::Fire()
{
    ACE_DEBUG ((LM_DEBUG,"CDHCPAuthRespEventHndl::Fire\n")); 
    
    if (m_server.HandleAuthResponse(m_response) != 0)
    {
        ACE_DEBUG ((LM_ERROR, "dhcp server failed to handle auth response.\n"));
    }
}


