/***********************************************************************
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

