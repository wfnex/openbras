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


