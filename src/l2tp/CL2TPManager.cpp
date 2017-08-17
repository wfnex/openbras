/***********************************************************************
 * 
Copyright (c) 2017, The OpenBRAS project authors. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.

  * Neither the name of OpenBRAS nor the names of its contributors may
    be used to endorse or promote products derived from this software
    without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************/


#include "CL2TPManager.h"
#include "CL2TPLAC.h"


ACE_Thread_Mutex mutex_;
ACE_Condition_Thread_Mutex condition_(mutex_);

//Provide interface
IL2TPManager* IL2TPManager::Instance()
{
    static CL2TPManager mgr;
    static bool bisinit = false;
    if (bisinit == false)
    {
        mgr.Start();
        bisinit = true;
    }
    return &mgr;
}


CL2TPManager::CL2TPManager()
{
}

CL2TPManager::~CL2TPManager()
{
}

//Start server
int CL2TPManager::Start()
{
    ACE_DEBUG ((LM_DEBUG,"CL2TPManager::Start\n"));  

    if (ACE_Thread_Manager::instance ()->spawn (CL2TPManager::server_worker, this) == -1)
    {
        ACE_ERROR_RETURN ((LM_ERROR,
                           ACE_TEXT ("(%t) %p\n"),
                           ACE_TEXT ("Unable to spawn server thread")),
                          -1);
    }

    mutex_.acquire();
    condition_.wait();
    mutex_.release();

    return 0;    
}

//Server cycle for work
ACE_THR_FUNC_RETURN
CL2TPManager::server_worker (void *p)
{
    ACE_UNUSED_ARG(p);
    //disable_signal (SIGPIPE, SIGPIPE);
    ACE_DEBUG ((LM_DEBUG,"CL2TPManager::server_worker\n")); 

    ACE_Reactor reactor;
    ACE_Reactor::instance(&reactor);
    mutex_.acquire();
    condition_.signal();
    mutex_.release();

    while (reactor.reactor_event_loop_done () == 0)
    {
        reactor.run_reactor_event_loop ();
    }

    return NULL;
}


//Create LAC
int CL2TPManager::CreateLAC(CCmAutoPtr<IL2TPLAC> &CallMake)
{
    CCmAutoPtr<IL2TPLAC> lac(new CL2TPLAC(*this));
    if (lac.Get()==NULL)
    {
        return -1;
    }
    CallMake = lac;
    return 0;
}


