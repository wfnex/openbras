/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
**********************************************************************/

#include "CPPPOE.h"
#include <stdlib.h>

#ifndef _DEBUG
#define _DEBUG
#endif

static bool finished = false;

static void signalHandler(int signo)
{   
    std::cerr << "Shutting down" << endl;   
    finished = true;
#ifdef WIN32   
    ::Sleep(1000);
#else   
    ::usleep(1000000);
#endif
    ACE_Reactor::end_event_loop ();
}

void InstallSignal()
{
    if (::signal( SIGPIPE, SIG_IGN) == SIG_ERR)  
    {      
        cerr << "Couldn't install signal handler for SIGPIPE" << endl;     
        exit(-1);  
    }
    if (::signal( SIGINT, signalHandler ) == SIG_ERR )   
    {      
        cerr << "Couldn't install signal handler for SIGINT" << endl;      
        exit( -1 );   
    }   
    if (::signal( SIGTERM, signalHandler ) == SIG_ERR )   
    {      
        cerr << "Couldn't install signal handler for SIGTERM" << endl;      
        exit( -1 );   
    }
}

int main(int argc, char **argv)
{
    InstallSignal();
    
#if defined(_DEBUG)
    ACE::debug (1);
    ACE_OSTREAM_TYPE *output = new ofstream ("pppoe.log");
    ACE_LOG_MSG->msg_ostream (output, 1);
    ACE_LOG_MSG->set_flags (ACE_Log_Msg::OSTREAM|ACE_Log_Msg::VERBOSE);
    ACE_LOG_MSG->clr_flags (ACE_Log_Msg::STDERR);
#endif

    ACE_DEBUG ((LM_DEBUG,"Starting PPPOE process...\n"));

    ACE_Reactor reactor;
    ACE_Reactor::instance(&reactor);

    /////////////////
    CPPPOE pppoe;

    pppoe.Init();

    if (pppoe.Start() == -1)
    {
        ACE_DEBUG ((LM_ERROR,"Failed to start PPPOE process.\n"));
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


