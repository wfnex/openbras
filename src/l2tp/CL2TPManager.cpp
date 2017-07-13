#include "CL2TPManager.h"
#include "CL2TPLAC.h"


ACE_Thread_Mutex mutex_;
ACE_Condition_Thread_Mutex condition_(mutex_);

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


