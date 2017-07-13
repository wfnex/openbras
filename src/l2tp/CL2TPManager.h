/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
**********************************************************************/
#ifndef CL2TPMANAGER_H
#define CL2TPMANAGER_H
#include "IL2TPInterface.h"

#include "CL2TPLAC.h"

#include "ace/Condition_Thread_Mutex.h"
#include "ace/Condition_Attributes.h"

class CL2TPManager : public IL2TPManager
{
public:
    CL2TPManager();
    virtual ~CL2TPManager();
    int Start();
    static ACE_THR_FUNC_RETURN server_worker (void *p);

    virtual int CreateLAC(CCmAutoPtr<IL2TPLAC> &CallMake);
private:

};


#endif//CPORTALMANAGER_H

