#ifndef IEVENTREACTOR_H
#define IEVENTREACTOR_H

#include "BaseDefines.h"

class IEvent 
{
public:
    virtual ~IEvent(){};   
    
    virtual void Fire() = 0;
};

class IEventReactor
{
public:
    virtual ~IEventReactor(){}

    static IEventReactor &Instance();
    
    virtual int ScheduleEvent(IEvent *pevent) = 0;
    virtual int Close() = 0;
};

#endif // IEVENTREACTOR_H

