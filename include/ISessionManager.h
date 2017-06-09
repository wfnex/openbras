// Interface for session manager

#ifndef __ISESSIONMANAGER_H__
#define __ISESSIONMANAGER_H__

#include "aim_ex.h"

class ISessionManagerSink 
{
public:
    virtual ~ISessionManagerSink(){};
    virtual int OnAddUserResponse(const UM_RESPONSE &response) = 0; 
    virtual int OnDeleteUserResponse(const UM_RESPONSE &response) = 0;
    virtual int OnModifyUserResponse(const UM_RESPONSE &response) = 0;
    virtual int OnKickUserNotify(const Sm_Kick_User* kickInfo) = 0;
};

class ISessionManager
{
public:
    static ISessionManager& instance();
    virtual ~ISessionManager(){};

    virtual int openWithSink(ISessionManagerSink *pSink) = 0;
    virtual int Close() = 0;

    virtual int addUserRequest(const Session_User_Ex* sInfo) = 0;
    virtual int deleteUserRequest(const Session_Offline* sInfo) = 0;
    virtual int modifyUserRequest(const Session_User_Ex* sInfo) = 0;
};

#endif//__ISESSIONMANAGER_H__
