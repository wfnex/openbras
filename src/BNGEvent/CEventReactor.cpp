#include "CEventReactor.h"

IEventReactor &IEventReactor::Instance()
{
    static CEventReactor event;
    return event;
}

CEventReactor::CEventReactor()
    :m_reactor(ACE_Reactor::instance())
    ,m_bfire(true)
{
    ACE_DEBUG ((LM_ERROR,"(%P|%t) CEventReactor::CEventReactor\n"));

    m_myThreadId = ACE_Thread::self ();
}

CEventReactor::~CEventReactor()
{
    ACE_DEBUG ((LM_ERROR,"(%P|%t) CEventReactor::~CEventReactor\n"));

    //CancelTimer();  // Commented as if the IP of NIC is not the one specified in the configuration file, the IPOE 
                                   // process starts abnormally.
                                   //
                                   // pure virtual method called
                                   // terminate called without an active exception
                                   // Aborted (core dumped)
}

int CEventReactor::handle_timeout (const ACE_Time_Value &current_time,
                            const void *act)
{
    ACE_DEBUG ((LM_ERROR,"(%P|%t) CEventReactor::handle_timeout\n"));

    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, -1);
    
    std::list<IEvent *>::iterator it = m_eventqueue.begin();
    while (it != m_eventqueue.end())
    {
        IEvent *pevent = *it;
        if (pevent)
        {
            pevent->Fire();
            delete pevent;
        }
        it = m_eventqueue.erase(it);
    }
    
    m_bfire = true;
    
    return -1;
}

void CEventReactor::StartEventSchedule()
{
    ACE_DEBUG ((LM_DEBUG, "(%P|%t) CEventReactor::StartEventSchedule\n"));
    
    if (!m_bfire)
    {
        ACE_DEBUG ((LM_DEBUG, "(%P|%t) m_bfire is false.\n"));
        return;
    }
    
    if (m_reactor)
    {
        m_reactor->schedule_timer(this, 0, ACE_Time_Value(0,0), ACE_Time_Value(0,0));
        m_bfire = false;
    }
}

void CEventReactor::CancelTimer()
{
    ACE_DEBUG ((LM_DEBUG, "(%P|%t) CEventReactor::CancelTimer\n"));

    if (m_reactor)
    {
        m_reactor->cancel_timer(this);
    }    
}

int CEventReactor::ScheduleEvent(IEvent *pevent)
{
    ACE_DEBUG ((LM_DEBUG, "(%P|%t) CEventReactor::ScheduleEvent, fireflag=%d\n", m_bfire));

    ACE_thread_t currentThreadId = ACE_Thread::self ();
    if (ACE_OS::thr_equal (m_myThreadId, currentThreadId))
    {
        ACE_DEBUG ((LM_DEBUG, 
                    "ScheduleEvent executed in the thread which creates CEventReactor::Instance(), "
                    "so fire the event immediately.\n"));
        pevent->Fire();
        return 0;
    }

    ACE_DEBUG ((LM_DEBUG,
                "ScheduleEvent executed in another thread which doesn't create CEventReactor::Instance(), "
                "so push the event into the list and fire it in the thread which creates CEventReactor::Instance().\n"));

    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, -1);
        
    m_eventqueue.push_back(pevent);
    StartEventSchedule();
    
    return 0;
}

int CEventReactor::Close()
{
    ACE_DEBUG ((LM_DEBUG, "(%P|%t) CEventReactor::Close, fireflag=%d\n", m_bfire));

    CancelTimer();
}


