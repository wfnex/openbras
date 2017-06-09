/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved. 
 ***********************************************************************/

#ifndef CPPPOE_EVENT_HANDLER_H
#define CPPPOE_EVENT_HANDLER_H

#include "aceinclude.h"
#include "aim_ex.h"
#include "IEventReactor.h"

class CPPPOE;

class CPPPOEAuthRespEvntHndl : public IEvent
{
public:
    CPPPOEAuthRespEvntHndl(CPPPOE &pppoe, const Auth_Response *response);
    virtual ~CPPPOEAuthRespEvntHndl();
	
    virtual void Fire();
	
private:
    CPPPOE &m_pppoe;
    Auth_Response m_response;
};

class CPPPOEKickUserEvntHndl : public IEvent
{
public:
    CPPPOEKickUserEvntHndl(CPPPOE &pppoe, const Sm_Kick_User *kickUser);
    virtual ~CPPPOEKickUserEvntHndl();
	
    virtual void Fire();
	
private:
    CPPPOE &m_pppoe;
    Sm_Kick_User m_kickUser;
};

#endif // CPPPOE_EVENT_HANDLER_H

