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


#ifndef CPORTALCLIENT_H
#define CPORTALCLIENT_H

#include "aceinclude.h"
#include <stdint.h>
#include <unordered_map>
#include <queue>
#include <vector>
#include <list>
#include "openportal.h"
#include "CReferenceControl.h"

class CPortalManager;
class CPortalConnector;
class CPortalClient : public ACE_Event_Handler
{
public:
    enum SelectAlgorithm
    {
        RoundRobin,
        LoadBalance,
        AlwaysFirst
    };

    CPortalClient(CPortalManager &mgr);
    virtual ~CPortalClient();
    void SetSelectAlg(SelectAlgorithm alg)
    {
        m_selectalg = alg;
    }
    int CreateConnector(PortalServerCfg &cfg);
    int DestroyConnector(uint8_t serverid);
    int DestroyConnector(uint32_t peerip, uint16_t peerport);
    CPortalConnector *FindActiveServer();
protected:
    SelectAlgorithm m_selectalg;
    ACE_Thread_Mutex m_mutex;
    std::list<CCmAutoPtr<CPortalConnector>> m_connectors;
    CPortalManager &m_portalmgr;
};

#endif//CDHCPSERVER_H


