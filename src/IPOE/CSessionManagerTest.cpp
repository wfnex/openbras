/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
**********************************************************************/

#include "CSessionManagerTest.h"

ISessionManager &ISessionManager::instance()
{
    static CSessionManager sessionmgr;
    return sessionmgr;
}

CSessionManager::CSessionManager()
    :m_psink(0)
{
}

CSessionManager::~CSessionManager()
{
}


int CSessionManager::openWithSink(ISessionManagerSink *pSink)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CSessionManager::openWithSink\n"));

    m_psink = pSink;
    return 0;
}

int CSessionManager::Close()
{

    m_psink = NULL;
    return 0;
}


int CSessionManager::addUserRequest(const Session_User_Ex* sInfo)
{
	if (sInfo == NULL)
	{
		return -1;
	}
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CSessionManager::addUserRequest\n"));

    return 0;
}

int CSessionManager::deleteUserRequest(const Session_Offline* sInfo)
{
	if (sInfo == NULL)
	{
		return -1;
	}
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CSessionManager::deleteUserRequest\n"));

    return 0;
}

int CSessionManager::modifyUserRequest(const Session_User_Ex* sInfo)
{
	if (sInfo == NULL)
	{
		return -1;
	}
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CSessionManager::modifyUserRequest\n"));

    return 0;
}



