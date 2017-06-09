/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
**********************************************************************/
#ifndef CPORTALSERVERCHANNEL_H
#define CPORTALSERVERCHANNEL_H

#include "aceinclude.h"
#include <stdint.h>
#include <unordered_map>
#include "CReferenceControl.h"
#include "openportal.h"
#include "IAuthManager.h"

class CPortalClient;
class CPortalServerManager;
class CPortalServerChannel : public CReferenceControl,public ACE_Event_Handler
{
public:
    CPortalServerChannel(CPortalServerManager &smgr,
        const ACE_INET_Addr &local_addr,
        const ACE_INET_Addr &remote_addr,
        PortalServerCfg *pcfg);
    virtual ~CPortalServerChannel();
    ACE_INET_Addr GetPeerAddr();
    ACE_INET_Addr GetLocalAddr();

    /// Get the I/O handle.
    virtual ACE_HANDLE get_handle (void) const;
    
    /// Set the I/O handle.
    virtual void set_handle (ACE_HANDLE fd);

    int SendData(const char *data, size_t datasize);
    int RcvData(const char *msg, size_t msgsize);
    void MakeChallenge(char *pStr, size_t len);
    void PrintPortalHead(openportal_header *pHeader);
protected:
    int HandleChallengeRequest(const char *msg, size_t msgsize);
    int HandleChallengeResponse(const char *msg, size_t msgsize);
    int HandleAuthenRequest(const char *msg, size_t msgsize);
    int HandleAuthenResponse(const char *msg, size_t msgsize);
    int HandleLogoutRequest(const char *msg, size_t msgsize);
    int HandleLogoutResponse(const char *msg, size_t msgsize);
    int HandleAuthenAFFResponse(const char *msg, size_t msgsize);
    int HandleNTFLogout(const char *msg, size_t msgsize);
    int HandleInfoResponse(const char *msg, size_t msgsize);
    int HandleInfoRequest(const char *msg, size_t msgsize);
    int HandleNTFUserDiscovery(const char *msg, size_t msgsize);
    int HandleUserIPChangeNotify(const char *msg, size_t msgsize);
    int HandleAFFNTFUserIPChange(const char *msg, size_t msgsize);
    int HandleNTFLogoutResponse(const char *msg, size_t msgsize);
    bool CheckAuthenticator(openportal_header *pheader);
    void AddAttribute(char *vpPacket, 
                        size_t *vpOffset, 
                        uint8_t tlv_type, 
                        uint8_t tlv_length, 
                        char *vpTlv_value);
    void AddAuthentication(openportal_header *phead);
    void PortalSerializeHeader(openportal_header *pHeader, 
                                        uint8_t code, 
                                        uint8_t auth_type,
                                        uint8_t version_type,
                                        uint16_t serial_no, 
                                        uint16_t req_id, 
                                        uint32_t sub_ip, 
                                        uint8_t error_code);

public:
    int OnAuthResponse(const Auth_Response *response);
    int NotifyLogoutRequest(uint32_t userid);
public:
    PortalServerCfg &GetServerCfg();
    void SetServerCfg(PortalServerCfg &cfg);
private:
    CPortalServerManager &m_smgr;
    //CPortalClient &m_portalClient;
    ACE_INET_Addr m_local_addr;
    ACE_INET_Addr m_remote_addr;
    bool m_isDetectedEnable;
    bool m_isDead;
    int m_detectcount;
    uint16_t m_last_detect_req_id;
    uint16_t m_reqidbase;
    PortalServerCfg m_cfg;
    ACE_HANDLE m_handler;
};


#endif//CPORTALSERVERCHANNEL_H

