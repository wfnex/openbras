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

