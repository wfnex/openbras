#ifndef CEVENTREACTOR_H
#define CEVENTREACTOR_H

#include "IEventReactor.h"
#include "aceinclude.h"
#include <list>

class CEventReactor : public IEventReactor, public ACE_Event_Handler
{
public:
    CEventReactor();
    virtual ~CEventReactor();
    
    virtual int ScheduleEvent(IEvent *pevent);
    virtual int Close();
    
protected:
    virtual int handle_timeout (const ACE_Time_Value &current_time,
                        const void *act = 0);

    void StartEventSchedule();
    void CancelTimer();
    
private:
    ACE_Reactor * m_reactor;
    std::list<IEvent *> m_eventqueue;
    bool m_bfire;                           // Flag to protect one CEventReactor instance from calling schedule_timer() more than once.
    ACE_Thread_Mutex m_mutex;  // Mutex for m_eventqueue
    ACE_thread_t m_myThreadId;
};


#endif//CEVENTREACTOR_H

