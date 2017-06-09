/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
**********************************************************************/
#include "CPortalManager.h"
#include <stdlib.h>
#include "CPortalConfig.h"

#ifndef _DEBUG
#define _DEBUG 1
#endif

int main(int argc, char **argv)
{    
#if defined(_DEBUG)
    ACE::debug (1);
    ACE_OSTREAM_TYPE *output = new ofstream ("portal.log");
    ACE_LOG_MSG->msg_ostream (output, 1);
    ACE_LOG_MSG->set_flags (ACE_Log_Msg::OSTREAM|ACE_Log_Msg::VERBOSE);
    ACE_LOG_MSG->clr_flags (ACE_Log_Msg::STDERR);
#endif

    ACE_DEBUG ((LM_DEBUG,"Starting Portal process...\n"));

    ACE_Reactor reactor;
    ACE_Reactor::instance(&reactor);

    /////////////////

    //ACE_INET_Addr httpaddr = CPortalConfig::Instance()->GetHttpListenAdd();
    //ACE_INET_Addr portaladdr = CPortalConfig::Instance()->GetPortalListenAddr();
    CPortalManager PortalMgr;

    if (PortalMgr.Init() == -1)
    {
        ACE_DEBUG ((LM_ERROR,"Failed to start Portal process.\n"));
        return -1;
    }

    ///////////////////////

    while (reactor.reactor_event_loop_done () == 0)
    {
        reactor.run_reactor_event_loop ();
    }

    //delete_reactor();
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("(%P|%t)  Reactor event loop finished ")
                ACE_TEXT ("successfully.\n")));

    ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%P|%t)  Reactor Main finished\n")));

    return 0;
}

