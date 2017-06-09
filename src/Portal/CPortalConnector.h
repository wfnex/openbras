/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved. 
**********************************************************************/

#ifndef CPORTALCONNECTOR_H
#define CPORTALCONNECTOR_H

#include "aceinclude.h"
#include <stdint.h>
#include <unordered_map>
#include "openportal.h"
#include "CReferenceControl.h"

class CPortalClient;
class CPortalConnector : public CReferenceControl,public ACE_Event_Handler
{
public:
    CPortalConnector(CPortalClient &client,PortalServerCfg &cfg);
    virtual ~CPortalConnector();
    virtual int StartConnect(const ACE_INET_Addr &peeraddr);
    virtual int StopConnect();
    void StartDetect(int second);
    void StopDetect();
    bool IsChannelAlive();
    int SendDetect();

    virtual int handle_timeout (const ACE_Time_Value &current_time,
                              const void *act = 0);

    //ACE_Event_Handler interface，子类可以重载这些函数
    virtual int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE);
    virtual ACE_HANDLE get_handle (void) const;
    virtual int handle_close (ACE_HANDLE handle,
                            ACE_Reactor_Mask close_mask);

    int GetLocalAddr (ACE_INET_Addr &address) const;

    int SendData(const char *data, size_t datasize);
    void PortalSerializeHeader(openportal_header *pHeader, 
                                    uint8_t code, 
                                    uint8_t auth_type,
                                    uint8_t version_type,
                                    uint16_t serial_no, 
                                    uint16_t req_id, 
                                    uint32_t sub_ip, 
                                    uint8_t error_code);
    void AddAuthentication(openportal_header *phead);
    uint8_t GetServerId();
    uint32_t GetServerIP();
    uint16_t GetServerPort();
protected:
    ACE_HANDLE m_handler;
    //ACE_INET_Addr m_localaddr;
    ACE_INET_Addr m_peeraddr;
    CPortalClient &m_portalclient;
    bool m_isDetectedEnable;
    bool m_isDead;
    int m_detectcount;
    uint16_t m_last_detect_req_id;
    PortalServerCfg m_cfg;

};

#endif//CDHCPSERVER_H


