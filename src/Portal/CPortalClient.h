/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
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


