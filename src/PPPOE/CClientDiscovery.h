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

