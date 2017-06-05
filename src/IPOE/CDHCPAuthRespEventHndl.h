/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
**********************************************************************/

#ifndef CDHCP_AUTH_RESP_EVENT_HNDL_H
#define CDHCP_AUTH_RESP_EVENT_HNDL_H

#include "aceinclude.h"
#include "aim_ex.h"
#include "CReferenceControl.h"
#include "IEventReactor.h"

class CDHCPServer;

class CDHCPAuthRespEventHndl : public IEvent
{
public:
    CDHCPAuthRespEventHndl(CDHCPServer &server, const Auth_Response *response);
    virtual ~CDHCPAuthRespEventHndl();
	
    virtual void Fire ();
	
private:
    CDHCPServer &m_server;
    Auth_Response m_response;
};

#endif // CDHCP_AUTH_RESP_EVENT_HNDL_H

