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

