/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
**********************************************************************/

#include "CPPPOEEventHandler.h"
#include "CPPPOE.h"

CPPPOEAuthRespEvntHndl::CPPPOEAuthRespEvntHndl(CPPPOE &pppoe, const Auth_Response *response)
    : m_pppoe(pppoe)
{
    ACE_DEBUG ((LM_DEBUG,"CPPPOEAuthRespEvntHndl::CPPPOEAuthRespEvntHndl\n")); 
    ::memcpy(&m_response, response, sizeof(*response));
}

CPPPOEAuthRespEvntHndl::~CPPPOEAuthRespEvntHndl()
{
    ACE_DEBUG ((LM_DEBUG, "CPPPOEAuthRespEvntHndl::~CPPPOEAuthRespEvntHndl\n")); 
}

void CPPPOEAuthRespEvntHndl::Fire()
{
    ACE_DEBUG ((LM_DEBUG, "CPPPOEAuthRespEvntHndl::Fire\n"));

    if (m_pppoe.HandleAuthResponse(&m_response) != 0)
    {
        ACE_DEBUG ((LM_ERROR, "PPPOE server failed to handle auth response.\n"));
    }
}

CPPPOEKickUserEvntHndl::CPPPOEKickUserEvntHndl(CPPPOE &pppoe, const Sm_Kick_User *kickUser)
    : m_pppoe(pppoe)
{
    ACE_DEBUG ((LM_DEBUG, "CPPPOEKickUserEvntHndl::CPPPOEKickUserEvntHndl\n"));
    ::memcpy(&m_kickUser, kickUser, sizeof(m_kickUser));
}

CPPPOEKickUserEvntHndl::~CPPPOEKickUserEvntHndl()
{
    ACE_DEBUG ((LM_DEBUG, "CPPPOEKickUserEvntHndl::~CPPPOEKickUserEvntHndl\n"));
}

void CPPPOEKickUserEvntHndl::Fire()
{
    ACE_DEBUG ((LM_DEBUG, "CPPPOEKickUserEvntHndl::Fire\n"));
    
    if (m_pppoe.HandleKickUserNotify(&m_kickUser) != 0)
    {
        ACE_DEBUG ((LM_ERROR, "PPPOE server failed to handle kick user notification.\n"));
    }
}


