/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
**********************************************************************/

#ifndef CDHCP_KICK_USER_EVENT_HNDL_H
#define CDHCP_KICK_USER_EVENT_HNDL_H

#include "aceinclude.h"
#include "aim_ex.h"
#include "CReferenceControl.h"
#include "IEventReactor.h"

class CDHCPServer;

class CDHCPKickUserEventHndl : public IEvent
{
public:
    CDHCPKickUserEventHndl(CDHCPServer &server, const Sm_Kick_User* kickInfo);
    virtual ~CDHCPKickUserEventHndl();
	
    virtual void Fire ();
	
private:
    CDHCPServer &m_server;
    Sm_Kick_User m_kickInfo;
};

#endif // CDHCP_KICK_USER_EVENT_HNDL_H

