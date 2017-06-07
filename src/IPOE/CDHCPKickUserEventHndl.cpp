/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
**********************************************************************/

#include "CDHCPKickUserEventHndl.h"
#include "CDHCPServer.h"
#include "BaseDefines.h"

CDHCPKickUserEventHndl::CDHCPKickUserEventHndl(CDHCPServer &server, const Sm_Kick_User* kickInfo)
    : m_server(server)
{
    ACE_DEBUG ((LM_DEBUG,"CDHCPKickUserEventHndl::CDHCPKickUserEventHndl\n")); 
    ::memcpy(&m_kickInfo, kickInfo, sizeof(*kickInfo));
}

CDHCPKickUserEventHndl::~CDHCPKickUserEventHndl()
{
    ACE_DEBUG ((LM_DEBUG,"CDHCPKickUserEventHndl::~CDHCPKickUserEventHndl\n")); 
}

void CDHCPKickUserEventHndl::Fire()
{
    ACE_DEBUG ((LM_DEBUG, "CDHCPKickUserEventHndl::Fire\n")); 

    WORD64 sessionId = m_server.GetSessionID(&m_kickInfo);
    
    int retVal = m_server.RemoveSession(sessionId);
    if (retVal != 0)
    {
        ACE_DEBUG ((LM_ERROR, "DHCP server failed to remove session with id %#x.\n", sessionId));
    }
}


