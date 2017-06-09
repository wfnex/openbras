/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
**********************************************************************/

#include "CIPOEModule.h"
#include <stdlib.h>
#include "iniparser.h"

static bool finished = false;

#ifndef _DEBUG
#define _DEBUG 1
#endif

static void signalHandler(int signo)
{   
    std::cerr << "Shutting down" << endl;   
    finished = true;
#ifdef ACE_WIN32   
    ::Sleep(1000);
#else   
    ::usleep(1000000);
#endif
    ACE_Reactor::end_event_loop ();
}

void InstallSignal()
{
#ifdef ACE_WIN32
#else

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
#endif
}

std::string GetDHCPServerIP()
{
	std::string strip;
    dictionary * ini = NULL;
    char        * s  = NULL;

    ini = iniparser_load("ipoe.cfg");
    if (ini == NULL) {
        ACE_DEBUG ((LM_INFO, "GetDHCPServerIP, failed to iniparser_load ipoe.cfg.\n"));
        return "0.0.0.0" ;
    }
    
    iniparser_dump(ini, stderr);

    s = const_cast<char *>(iniparser_getstring(ini, "IPOE:dhcpserver", NULL));
	strip = std::string(s, strlen(s));
    iniparser_freedict(ini);
    return strip;
}

int main(int argc, char **argv)
{
    InstallSignal();
    
#if defined(_DEBUG)
    ACE::debug (1);
    ACE_OSTREAM_TYPE *output = new ofstream ("ipoe.log");
    ACE_LOG_MSG->msg_ostream (output, 1);
    ACE_LOG_MSG->set_flags (ACE_Log_Msg::OSTREAM|ACE_Log_Msg::VERBOSE);
    ACE_LOG_MSG->clr_flags (ACE_Log_Msg::STDERR);
#endif

    ACE_DEBUG ((LM_DEBUG,"Starting IPOE process...\n"));

    ACE_Reactor reactor;
    ACE_Reactor::instance(&reactor);

    /////////////////
    CIPOEModule module(&reactor);
	std::string dhcpserverip = GetDHCPServerIP();
    if (module.Start(dhcpserverip) == -1)
    {
        ACE_DEBUG ((LM_ERROR,"Failed to start IPOE process.\n"));
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

