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


#ifndef CPPPOE_H
#define CPPPOE_H

#include "aceinclude.h"
#include "CReferenceControl.h"
#include "STDInclude.h"
#include "pppoe.h"
#include "BaseDefines.h"
#include "CPPPOEDiscoveryHandler.h"
#include "CPPPOESessionHandler.h"
#include "CEtherIntf.h"
#include <string>
#include "ISessionManager.h"
#include "CAddSessionId.h"

#ifndef DEBUG_CONFIG_INTERFACE
#define DEBUG_CONFIG_INTERFACE
#endif 

#ifdef DEBUG_INTERFACE
#undef DEBUG_INTERFACE
#endif

#ifndef DEBUG_INTERFACE
#include "IAuthManager.h"
#endif

#ifdef DEBUG_INTERFACE
typedef struct
{
    WORD16 sessionId;
    CHAR userName[100];
    CHAR passwd[100];
} AUTH_REQUEST;

typedef struct
{
    WORD16 sessionId;
    WORD32 result;
    WORD32 subscriberIP;
    WORD32 primaryDNS;
    WORD32 secondaryDNS;
    CHAR reason[100];
} AUTH_RESPONSE;
#endif

#ifndef DEBUG_PKT // 报文打印开关
#define DEBUG_PKT
#endif



#ifndef DEBUG_PAP
#define DEBUG_PAP
#undef DEBUG_CHAP
#endif

#ifndef DEBUG_CHAP
#define DEBUG_CHAP
#undef DEBUG_PAP
#endif


class CClientDiscovery;
class CSession;
class CPPPOEEventHandler;
class CAddSessionId;

typedef std::unordered_map<WORD64, CCmAutoPtr<CClientDiscovery>> ClientDiscoveryType;
typedef std::unordered_map<WORD16, CCmAutoPtr<CSession>> SESSIONType;

/* 对MRU\MTU的说明，整个PPPOE、PPP配置及协商过程中涉及到3份MTU\MRU值:
 * 1.全局配置VBUI的MTU，
 * 2.PPPOE协商相关的MRU(包括本端的初始值、最终协商得到的值，后面PPP同)，RFC2516规定不得超过1492，RFC4638给出了处理
 *   超过1492的算法.
 * 3.PPP协商相关的MRU
 * 个人认为最为合理的、充分符合RFC的处理:
 * 用全局配置的MTU作为第2步(本端)PPPOE协商的初始值，利用RFC4638的算法进行协商(移植的rp-pppoe看起来实现了RFC4638)，
 * 将协商得到的最终值最为第3步的初始值，再进行PPP MRU相关的协商。
 * 印象中按照RFC4638规定，如果PPP协商得到的MRU大于1492，需要用几个这么长的保活报文进行验证。
 * 在这种处理方法下，如果client的pppoe tag中没有PPP-Max-Payload，则第1份值(VBUI MTU)将作为第3步的初始化值，后面此项
 * 的协商就是PPP相关的处理了。
 *
 * 现有的实现方法:
 * 1、2两步，第3步没有实现，即PPPOE协商到的PPP-Max-Payload没有带入PPP的协商(印象中rp-pppoe是将协商得到的此值带给
 * pppd的。)第3步用1492作为初始化值。
 * 这么做的理由，类似PPP MRU协商，如果协商双方中的任一方大于或小于1500，协商会向1500方向收敛。因为1500是必须支持的。
 * 此处选成1492理由类似。因为1492也应该是必须支持的。
 */
 
// Configurations
class IPPPOECfgInterface
{
public:    
    virtual ~IPPPOECfgInterface() {}

    // PPPoE related configurations
    virtual CHAR *GetACName() = 0;
    virtual CHAR *GetSvcName() = 0;
    virtual void SetACName(const CHAR acName[PPPOE_MAX_ACNAME_LENGTH]) = 0;
    virtual void SetSvcName(const CHAR svcName[PPPOE_MAX_SERVICE_NAME_LENGTH]) = 0;
    
    // VBUI related configurations
    virtual void GetVBUIIntfName(CHAR acIntfName[ETHER_INTF_NAME_SIZE+1]) = 0;
    virtual void SetVBUIIntfName(const CHAR acIntfName[ETHER_INTF_NAME_SIZE+1]) = 0;
    virtual WORD16 GetVBUIIntfMtu() = 0;
    virtual void SetVBUIIntfMtu(WORD16 mtu) = 0;
    virtual WORD32 GetVBUIIntfIP() = 0;  // In network byte order
    virtual void SetVBUIIntfIP(WORD32 ipAddr) = 0;
    virtual void SetVBUIIntfIP(std::string &ipAddr) = 0;

    // Authetication related configurations
    virtual void SetAuthType(uint16_t authType) = 0;  // The value of authType is the enumeraton of AIM_AUTH_TYPE in aim_ex.h
    virtual void SetHostName(std::string &hostName) = 0;    
};

class CPPPOE : public IAuthManagerSink, public ISessionManagerSink, public IPPPOECfgInterface
{
public:
    CPPPOE();
    virtual ~CPPPOE();

    void Init(const ACE_CString &ifname, const ACE_CString &ifip);
    int Start();

    CPPPOEDiscoveryHandler &GetPppoeDiscoveryHndl();
    CPPPOESessionHandler &GetPppoeSessionHndl();
    CEtherIntf &GetEtherIntf();
		
    // m_clientDiscoveries related operations
    int CreateClientDiscovery(BYTE clientMac[ETH_ALEN], CCmAutoPtr<CClientDiscovery> &discovery);
    int AddClientDiscovery(WORD64 clientMac, CCmAutoPtr<CClientDiscovery> &discovery);
    int RemoveClientDiscovery(WORD64 clientMac);
    int RemoveClientDiscovery(CCmAutoPtr<CClientDiscovery> &discovery);

    // m_sessionMgr related operations
    int CreateSession(CCmAutoPtr<CSession> &session);
    int AddSession(WORD16 sessionid, CCmAutoPtr<CSession> &session);
    int RemoveSession(WORD16 sessionid);
    int RemoveSession(CCmAutoPtr<CSession> &session);
    CSession * FindSession(WORD16 sessionid);

    WORD16 AllocId();
     void FreeId(uint16_t id);
     WORD32 GetIp();

    void OnLCPDown(WORD16 sessionId, const std::string &reason);
    void OnPPPOEAuthRequest(Auth_Request &authReq);
    int HandleAuthResponse(const Auth_Response *response);

    // For interface IAuthManagerSink
    int OnAuthResponse(const Auth_Response *response);

    // For interface ISessionManagerSink
    int OnAddUserResponse(const UM_RESPONSE &response);     // 增删改用户响应暂时不用实现
    int OnDeleteUserResponse(const UM_RESPONSE &response);
    int OnModifyUserResponse(const UM_RESPONSE &response);
	int OnKickUserNotify(const Sm_Kick_User* kickInfo);

    SWORD32 AddSubscriber(Session_User_Ex &sInfo);
    SWORD32 DelSubscriber(Session_Offline &sInfo);
    int HandleKickUserNotify(const Sm_Kick_User* kickInfo);

    // For interface IPPPOECfgInterface
    // PPPoE related configurations
    CHAR *GetACName();
    CHAR *GetSvcName();
    void SetACName(const CHAR acName[PPPOE_MAX_ACNAME_LENGTH]);
    void SetSvcName(const CHAR svcName[PPPOE_MAX_SERVICE_NAME_LENGTH]);
    // VBUI related configurations
    void GetVBUIIntfName(CHAR acIntfName[ETHER_INTF_NAME_SIZE+1]);
    void SetVBUIIntfName(const CHAR acIntfName[ETHER_INTF_NAME_SIZE+1]);
    WORD16 GetVBUIIntfMtu();
    void SetVBUIIntfMtu(WORD16 mtu);
    WORD32 GetVBUIIntfIP();
    void SetVBUIIntfIP(WORD32 ipAddr);
    void SetVBUIIntfIP(std::string &ipAddr);
    // Authetication related configurations
    void SetAuthType(uint16_t authType);  // The value of authType is the enumeraton of AIM_AUTH_TYPE in aim_ex.h
    void SetHostName(std::string &hostName);  
   

protected:
    void ClearAllClientDiscovery();
    void ClearAllSession();
                    
private:
    CPPPOEDiscoveryHandler m_DiscoveryHandle;
    CPPPOESessionHandler m_SessionHandle;
    ClientDiscoveryType m_clientDiscoveryMgr;  // 暂时不使用m_clientDiscoveryMgr，设计目的是类似m_sessionMgr那样对每个
                                               // client进行管理，但后面移植过程中发现暂时没有必要
    SESSIONType m_sessionMgr;
    WORD16 m_nSessionIdBase;
    CEtherIntf m_etherIntf;
    CAddSessionId m_psessionid;
    // Configurations
    CHAR m_acName[PPPOE_MAX_ACNAME_LENGTH];         /* AC Name for pppoe negotiation */
    CHAR m_svcName[PPPOE_MAX_SERVICE_NAME_LENGTH];  /* Service Name for pppoe negotiation */
    uint16_t m_authType;
    std::string m_hostName;
    ACE_Thread_Mutex m_mutex4ClientDiscoveryMgr;  // Mutex for m_clientDiscoveryMgr
    ACE_Thread_Mutex m_mutex4SessionMgr;            // Mutex for m_sessionMgr
};

#endif//CPPPOE_H

