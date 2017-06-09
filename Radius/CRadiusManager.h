/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved. 
**********************************************************************/
#ifndef CRADIUSMANAGER_H
#define CRADIUSMANAGER_H
#include "aceinclude.h"
#include <string>
#include <unordered_map>

class CRadiusScheme;
class CRadiusManager
{
public:
    CRadiusManager();
    ~CRadiusManager();
    int Init();
    int CreateRadiusScheme(std::string &name,CRadiusScheme *&radius);
    int DestroyRadiusScheme(std::string &name);
    int FindRadiusScheme(std::string &name,CRadiusScheme *&radius);
    CRadiusConfig &GetConfig()
    {
        return m_config;
    }
    int TestRadiusAccess();
private:
    ACE_Thread_Mutex m_mutex;
    CRadiusConfig m_config;
    std::unordered_map<std::string, CRadiusScheme *> m_schemes;
};


#endif//CPORTALMANAGER_H

