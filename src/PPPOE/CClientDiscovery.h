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
#ifndef CCLIENTDISCOVERY_H
#define CCLIENTDISCOVERY_H

#include "CReferenceControl.h"
#include "ace/Time_Value.h"
#include "pppoe.h"
#include "BaseDefines.h"

class CClientDiscovery : public CReferenceControl
{
public:
    CClientDiscovery(BYTE clientMac[ETH_ALEN]);
    virtual ~CClientDiscovery();

    BYTE *GetClientMac() {return m_clientMac;}
    WORD64 GetClientDiscoveryId();
    void SetClientMac(BYTE clientMac[ETH_ALEN]);
    WORD16 GetSession() {return m_sess;}
    void SetSession(WORD16 sess) {m_sess = sess;}
    const ACE_Time_Value &GetStartTime() {return m_startTime;}
    void SetStartTime(const ACE_Time_Value &startTime) {m_startTime = startTime;}
    void GetSvcName(CHAR svcName[PPPOE_MAX_SERVICE_NAME_LENGTH]);
    void SetSvcName(CHAR svcName[PPPOE_MAX_SERVICE_NAME_LENGTH]);
    WORD16 GetReqMtu() {return m_requestedMtu;}
    void SetReqMtu(WORD16 reqMtu) {m_requestedMtu = reqMtu;}
    PPPoETag &GetRelayId() {return m_relayId;}
    
private:
    BYTE m_clientMac[ETH_ALEN];
    WORD16 m_sess;		/* Session number */ // TBD!!!! May be unnecessary
    ACE_Time_Value m_startTime; //time_t startTime;		/* When session started */ // 计费时间由UserMgr负责，不需要pppoe
    CHAR m_serviceName[PPPOE_MAX_SERVICE_NAME_LENGTH];	/* Service name */ // !!!!分析rp-pppoe的代码及结合对pppoe 
                                                                           // rfc的理解，此字段貌似不需要保存。
    WORD16 m_requestedMtu;     /* Requested PPP_MAX_PAYLOAD  per RFC 4638 */

    //结合开源项目rp-pppoe-3.11
    //个人对m_relayId的理解: 在PPPoE discovery阶段，如果某个client的报文携带此字段，应予以保存。以便在后面的PADT中携带。
    //但在rp-pppoe中，对client和server采用了不同的方法，
    //client: 对应每个struct PPPoEConnectionStruct，在解析PADO、PADS时，保存relayId
    //server: 用全局变量relayId保存，这意味着，并未保存各client的relayId。
    //现在的解决方法: server不保存各client的relayId，发送的PADT中不携带此选项。
    //后续如果需要，一种参考解决方法: 
    // CPPPOEDiscoveryHandler::processPADR()中，此relayId放在CSession中，而不是此类中，因为这会涉及到对CClientDiscovery
    // 的管理。
    PPPoETag m_relayId;
    
};

#endif // CCLIENTDISCOVERY_H

