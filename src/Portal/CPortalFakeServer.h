/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
**********************************************************************/
#ifndef CPORTALFAKESERVER_H
#define CPORTALFAKESERVER_H
#include "aceinclude.h"
#include <stdint.h>
#include <unordered_map>
#include "CTCPTransport.h"
#include "CReferenceControl.h"
#include "openportal.h"

class CPortalManager;
class CPortalFakeServer;
class CPortalHTTPChannel : public CTCPTransport, public CReferenceControl
{
public:
    CPortalHTTPChannel();
    CPortalHTTPChannel(uint32_t id, CPortalFakeServer *pserver);
    virtual ~CPortalHTTPChannel();
    void SetPortalServer(CPortalFakeServer *pserver);
    virtual size_t OnHandleMessage(const ACE_Message_Block &aData);
    virtual void OnRcvBufferOverFlow(size_t maxbuffersize);
    virtual void OnConnected();
    inline uint32_t GetID() const
    {
        return m_id;
    }
private:
    uint32_t m_id;
    CPortalFakeServer *m_pserver;
};

typedef ACE_Acceptor<CPortalHTTPChannel, ACE_SOCK_ACCEPTOR> ACCEPTOR;

class CPortalConfig;
class CPortalFakeServer : public ACCEPTOR
{
public:
    CPortalFakeServer(CPortalManager &mgr);
    virtual ~CPortalFakeServer();
    virtual int StartListen(const ACE_INET_Addr &httplistenip);
    virtual void StopListen();
    virtual int accept_svc_handler (CPortalHTTPChannel *handler);
    virtual int make_svc_handler (CPortalHTTPChannel *&sh);
    virtual int AddChannel(CPortalHTTPChannel *pvs);
    virtual int RemoveChannel(CPortalHTTPChannel *pvs);
    virtual CPortalHTTPChannel *FindChannel(int handler);
    CPortalConfig &GetPortalConfig();
private:
    uint32_t m_nHttIDBase;
    typedef std::unordered_map<uint32_t, CCmAutoPtr<CPortalHTTPChannel>> UNBINDVSType;
    UNBINDVSType    m_UnBindVSs;
    CPortalManager &m_mgr;
};



#endif//CPORTALFAKESERVER_H
