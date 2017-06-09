/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
**********************************************************************/

#ifndef CSESSIONMANAGER_H
#define CSESSIONMANAGER_H
#include "ISessionManager.h"
#include "aceinclude.h"

class CSessionManager : public ISessionManager 
{
public:
    CSessionManager();
    virtual ~CSessionManager();
    
    virtual int openWithSink(ISessionManagerSink *pSink);
    virtual int Close();
    
    virtual int addUserRequest(const Session_User_Ex* sInfo);
    virtual int deleteUserRequest(const Session_Offline* sInfo);
    virtual int modifyUserRequest(const Session_User_Ex* sInfo);
private:
    ISessionManagerSink *m_psink;
};


#endif//CSESSIONMANAGER_H

