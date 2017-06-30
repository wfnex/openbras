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
#ifndef _FLEXBNG_AAA_H_
#define _FLEXBNG_AAA_H_

#include "BaseDefines.h"

#define WFNOS 1

/*�û��������󳤶�*/
#define MAX_DOMAIN_NAME_LEN                   (32)
/*�û�����󳤶�*/
#define MAX_SUBCRI_NAME_LEN                   (128)
/*�û�������󳤶�*/
#define MAX_SUBCRI_PWD_LEN                    (32)
/*MD5���ܽ������*/
#define MD5_RESULT_LENGTH                     (16)

#define CHAP_CHALLENGE_LEN             16
#define CHAP_RESPONSE_LEN              16
#define CHAP_PASSWORD_LEN              (CHAP_RESPONSE_LEN + 1)

#define MS_CHAP_LM_RESPONSE_LEN      (24)  /**<  @brief ms_chap_response��LM-RESPONSE�ֶ�,24�ֽ� */
#define MS_CHAP_NT_RESPONSE_LEN      (24)  /**<  @brief ms_chap_response��NT-RESPONSE�ֶ�,24�ֽ� */
#define MS_CHAP1_PEER_CHALLEGE_LEN   (8)   /**<  @brief ΢��CHAP1��ս�ֳ���8���ֽ� */

/*MS-CHAP2-REPONSE������س���*/
#define MS_CHAP2_RESPONSE_LEN        (24)  /**<  @brief ms_chap_response,24�ֽ� */
#define MS_CHAP2_PEER_CHALLEGE_LEN   (16)  /**<  @brief ΢��CHAP2��֤��ս�ֳ���16�ֽ� */
#define MS_CHAP2_RSV_LEN             (8)   /**<  @brief ΢��CHAP2��֤��Ӧ���� */

/* MS-CHAP-RESPONSE �ֶ� */
#define MS_CHAP_FLAG_LM_RESPONSE_ONLY      (0)/**<  @brief ΢��CHAP2��֤��Ӧflag���֮LM-RESPONSE-ONLY */
#define MS_CHAP_FLAG_NT_RESPONSE_FIRST     (1)/**<  @brief ΢��CHAP2��֤��Ӧflag���֮NT-RESPONSE-FIRST */

/*Э��涨MS-CHAP2-Success���Գ��ȣ������޸�*/
#define MS_CHAP2_SUC_LEN             (42)  /**<  @brief ΢��CHAP V2��֤�ɹ����صĻ�Ӧ��Ϣ�ַ���֧�ֵĳ��� */
#define MS_CHAP_ERR_LEN              (256) /**<  @brief ΢��CHAP��֤���صĴ����Ӧ��Ϣ�ַ���֧�ֵ���󳤶� */

#define MS_CHAP2_PEER_RESPONSE      ((WORD32)  (42))

#define DSLAM_IDENTIFIER_LEN                  (50)

#define ONU_ID_MAX_LEN       ((WORD32) (24))

#define DSLAM_TYPE_STRING_LEN          (2)/**<  @brief DSLAM�豸�����ַ������� */
#define ONU_PORT_TYPE_STRING_LEN       (3)/**<  @brief ONU�豸�˿������ַ������� */
#define ONU_ACCESS_METHOD_STRING_LEN   (2)/**<  @brief ONU�豸���뷽���ַ������� */
#define AGENT_CIR_ID_LENGTH				(128)

#define ANCP_AGENT_ID_LEN            (64)  /**<  @brief DSLAM�豸���͵�agent-id�ַ���ʶ֧�ֵ���󳤶� */

#define AGENT_REMOTE_ID_LENGTH          ANCP_AGENT_ID_LEN

// Authentication Manager
#define AUTHMGR_MAC_LENGTH (6)
#define AUTHMGR_MAX_DOMAIN_SIZE (32)
#define AUTHMGR_MAX_AUTH_RESULT_DESC_SIZE (32)
#define AUTHMGR_MAX_USERNAME_SIZE (32)
#define AUTHMGR_MAX_PASSWD_SIZE AUTHMGR_MAX_USERNAME_SIZE
#define AUTHMGR_MAX_CHALLENGE_SIZE (16)
#define AUTHMGR_MAX_NAS_PORT_ID_SIZE (256)
#define AUTHMGR_MAX_CALLING_STATION_ID_SIZE (64)

typedef enum  {
	 USER_TYPE_START,				//无效的用户类�?
	 USER_TYPE_IPOE = 1,	        //IPOE用户类型
	 USER_TYPE_PORTAL,
	 USER_TYPE_PPPOE,			    //PPPOE用户类型 
	 USER_TYPE_L2TP,				//L2TP用户类型 
	 USER_TYPE_HOST,				//HOST用户类型
	 USER_TYPE_V6HOST,			    //HOST V6用户类型
	 USER_TYPE_CGN, 			    //CGN用户类型
	 USER_TYPE_L3NI,				//三层接入用户类型
	 USER_TYPE_L2TP_LNS,			//L2TP LNS用户类型
	 USER_TYPE_L2TP_LNS_V6, 		//L2TP LNS V6用户类型
	 USER_TYPE_IPOE_V4, 			//支持相同MAC用户
	 USER_TYPE_L3HOST,				//L3静态接入用户类�?
	 USER_TYPE_RADIUS_PROXY,		//代理用户
	 USER_TYPE_END 				    //无效的用户类�?  	 
}SessionUserType;		

typedef enum {
	AIM_REQ_TYPE_AUTHEN,
    AIM_REQ_TYPE_AUTHOR
}AIM_REQ_TYPE;

typedef enum {
    AUTH_PAP,
    AUTH_CHAP,
    AUTH_MSCHAP,
    AUTH_EAP
}AIM_AUTH_TYPE;

/**  用户接入端口类型值定义，具体参见RFC2865 */
typedef enum RADIUS_NAS_PORT_TYPE_VALUE
{
    RADIUS_NAS_PORT_TYPE_ASYNC               = 0, /**<  @brief ASYNC接入端口类型 */
    RADIUS_NAS_PORT_TYPE_SYNC                = 1, /**<  @brief SYNC接入端口类型 */
    RADIUS_NAS_PORT_TYPE_ISDN_SYNC           = 2, /**<  @brief ISDN-SYNC接入端口类型 */
    RADIUS_NAS_PORT_TYPE_ISDN_ASYNC_V120     = 3, /**<  @brief ISDN-ASYNC-V120接入端口类型 */
    RADIUS_NAS_PORT_TYPE_ISDN_ASYNC_V110     = 4, /**<  @brief ISDN-ASYNC-V110接入端口类型 */
    RADIUS_NAS_PORT_TYPE_VIRTUAL             = 5, /**<  @brief VIRTUAL接入端口类型，l2tp用户使用 */
    RADIUS_NAS_PORT_TYPE_PIAFS               = 6, /**<  @brief PIAFS接入端口类型 */
    RADIUS_NAS_PORT_TYPE_HDLC_CLEAR_CHANNEL  = 7, /**<  @brief HDLC-CLEAR-CHANNEL接入端口类型 */
    RADIUS_NAS_PORT_TYPE_X25                 = 8, /**<  @brief X25接入端口类型 */
    RADIUS_NAS_PORT_TYPE_X75                 = 9, /**<  @brief X75接入端口类型 */
    RADIUS_NAS_PORT_TYPE_G3FAX               = 10,/**<  @brief G3FAX接入端口类型 */
    RADIUS_NAS_PORT_TYPE_SDSL                = 11,/**<  @brief SDSL接入端口类型 */
    RADIUS_NAS_PORT_TYPE_ADSL_CAP            = 12,/**<  @brief ADSL-CAP接入端口类型 */
    RADIUS_NAS_PORT_TYPE_ADSL_DMT            = 13,/**<  @brief ADSL-DMT接入端口类型 */
    RADIUS_NAS_PORT_TYPE_IDSL                = 14,/**<  @brief IDSL接入端口类型 */
    RADIUS_NAS_PORT_TYPE_ETHERNET            = 15,/**<  @brief ETHERNET接入端口类型 */
    RADIUS_NAS_PORT_TYPE_XDSL                = 16,/**<  @brief XDSL接入端口类型 */
    RADIUS_NAS_PORT_TYPE_CABLE               = 17,/**<  @brief CABLE接入端口类型 */
    RADIUS_NAS_PORT_TYPE_WIRELESS            = 18,/**<  @brief WIRELESS接入端口类型 */
    RADIUS_NAS_PORT_TYPE_WIRELESS_IEEE802_11 = 19,/**<  @brief WIRELESS-IEEE802-11接入端口类型 */
    RADIUS_NAS_PORT_TYPE_MAX                 = 20 /**<  @brief 接入端口类型最大值，无意�?*/
}RADIUS_NAS_PORT_TYPE_VALUE;

/**  用户接入协议值定义，具体参见RFC2865 */
typedef enum RADIUS_FRAMED_PROTOCOL_VALUES
{
    FRAMED_PROTOCOL_PPP      = 1,/**<  @brief PPP接入协议 */
    FRAMED_PROTOCOL_SLIP     = 2,/**<  @brief SLIP接入协议 */
    FRAMED_PROTOCOL_ARAP     = 3,/**<  @brief ARAP接入协议 */
    FRAMED_PROTOCOL_GANDALF  = 4,/**<  @brief GANDALF接入协议 */
    FRAMED_PROTOCOL_XYLOGICS = 5,/**<  @brief XYLOGICS接入协议 */
    FRAMED_PROTOCOL_X75      = 6,/**<  @brief X75接入协议 */
    FRAMED_PROTOCOL_MAX      = 7 /**<  @brief 接入协议最大值，无意�?*/
}RADIUS_FRAMED_PROTOCOL_VALUES;

/**  RADIUS 服务类型值定义，具体参见RFC2865 */
typedef enum RADIUS_SERVICE_TYPE_VALUES
{
    SERVICE_TYPE_LOGIN                   = 1, /**<  @brief LOGIN服务类型 */
    SERVICE_TYPE_FRAMED                  = 2, /**<  @brief FRAMED服务类型 */
    SERVICE_TYPE_CALLBACK_LOGIN          = 3, /**<  @brief CALLBACK-LOGIN服务类型 */
    SERVICE_TYPE_CALLBACK_FRAMED         = 4, /**<  @brief CALLBACK-FRAMED服务类型 */
    SERVICE_TYPE_OUTBOUND                = 5, /**<  @brief OUTBOUND服务类型 */
    SERVICE_TYPE_ADMINISTRATIVE          = 6, /**<  @brief ADMINISTRATIVE服务类型 */
    SERVICE_TYPE_NAS_PROMPT              = 7, /**<  @brief NAS-PROMPT服务类型 */
    SERVICE_TYPE_AUTHENTICATE_ONLY       = 8, /**<  @brief AUTHENTICATE-ONLY服务类型 */
    SERVICE_TYPE_CALLBACK_NAS_PROMPT     = 9, /**<  @brief CALLBACK-NAS-PROMPT服务类型 */
    SERVICE_TYPE_CALL_CHECK              = 10,/**<  @brief CALL-CHECK服务类型 */
    SERVICE_TYPE_CALLBACK_ADMINISTRATIVE = 11,/**<  @brief CALLBACK-ADMINISTRATIVE服务类型 */
    SERVICE_TYPE_MAX                     = 12 /**<  @brief 服务类型最大值，无意�?*/
}RADIUS_SERVICE_TYPE_VALUES;

/**
*   �û�����
*/
typedef struct SUBSCRI_PWD
{
    CHAR    str[MAX_SUBCRI_PWD_LEN];      /**<@brief  ���� */
} SUBSCRI_PWD;

typedef struct PAP_AUTH_INFO
{
    SUBSCRI_PWD subPwd;
} PAP_AUTH_INFO;

/*CHAP��֤��Ϣ*/
typedef struct
{
    CHAR    str[CHAP_PASSWORD_LEN + 3];/*Ϊ����4�ֽڶ��룬���˴��޸�*/
} CHAP_PASSWORD;


typedef struct
{
    CHAR    str[CHAP_CHALLENGE_LEN];
} CHANLLENGE;

typedef struct CHAP_AUTH_INFO
{
    CHAP_PASSWORD   chapPwd;
    BYTE            chapId;       /**<@brief CHAP��֤ʱ��1�ֽ�CHAP-ID */
    BYTE            reserved[3];
    CHANLLENGE      challenge;
} CHAP_AUTH_INFO;
typedef struct MS_CHAPV1_PASSWORD
{
    char  str[MS_CHAP_NT_RESPONSE_LEN];
} MS_CHAPV1_PASSWORD;
typedef struct MS_CHAPV1_CHANLLENGE
{
    char  str[MS_CHAP1_PEER_CHALLEGE_LEN];
} MS_CHAPV1_CHANLLENGE;

typedef struct MS_CHAPV2_PASSWORD
{
    char  str[MS_CHAP_NT_RESPONSE_LEN];
} MS_CHAPV2_PASSWORD;

typedef struct MS_CHAPV1_AUTH_INFO
{
    MS_CHAPV1_PASSWORD LMResp;
    MS_CHAPV1_PASSWORD NTResp;
    MS_CHAPV1_CHANLLENGE        MSChapV1Challenge;
    BYTE           ChapID;           /* CHAP��֤ʱ��1�ֽ�CHAP-ID ��Ҫ���͸�radius��װ������*/
    BYTE NTFlag;        /* If 1, ignore the LM response field */
    BYTE  Idle[2];
} MS_CHAPV1_AUTH_INFO;

typedef struct MS_CHAPV2_AUTH_INFO
{
    MS_CHAPV2_PASSWORD           ChapV2Resp;
    CHANLLENGE                             MSChapV2Challenge;
    CHANLLENGE                              peerCHAPV2Challenge;
    BYTE      peerCHAPV2Response[MS_CHAP2_PEER_RESPONSE ];
    BYTE           ChapID;          /* CHAP��֤ʱ��1�ֽ�CHAP-ID ��Ҫ���͸�radius��װ������*/
    BYTE   Idle;
} MS_CHAPV2_AUTH_INFO;

/**
*   �û���λ��Ϣ
*/
typedef struct USER_ANI_PORT_INFO
{
    CHAR    accNodeIdentifier[DSLAM_IDENTIFIER_LEN +1]; /**<@brief ����ڵ��ʶ(��DSLAM�豸)���ַ����м䲻���пո� */
    BYTE    aniXpiEnable;          /**<@brief 0û�У�1�� */
    WORD16  aniXpi;                /**<@brief ��ѡ��     */
    WORD16  aniXci;                /**<@brief ��ѡ��ANI_XPI.ANI_XCI����Ҫ��Я��CPE���ҵ����Ϣ��
                                   ���ڱ�ʶδ����ҵ���������� ���ڶ�PVCӦ�ó����¿ɱ�ʶ�����ҵ��*/
    BYTE    aniRack;               /**<@brief ����ڵ���ܺţ���֧�ֽ���ϵ�DSLAM�豸�� 0~15  */
    BYTE    aniFrame;              /**<@brief ����ڵ�����0~31  */
    BYTE    aniSlot;               /**<@brief ����ڵ�ۺ�0~127   */
    BYTE    aniSubSlot;            /**<@brief ����ڵ��Ӳۺ�0~31  */
    BYTE    aniPort;               /**<@brief ����ڵ�˿ں�0~255 */
    BYTE  ONU_Slot;
    BYTE  ONU_Subslot;
    BYTE  ONUPort_ID;
    WORD32 dslam_port_type;
    BYTE  AniONU_ID[ONU_ID_MAX_LEN];
    BYTE  ONUPortType[ONU_PORT_TYPE_STRING_LEN];/*ptm|atm|eth*/
    WORD16  ONUPortXpi;
    WORD16  ONUPortXci;
    BYTE  ONUAccessMethod[ONU_ACCESS_METHOD_STRING_LEN];/*{EP|GP}����ʾ���û����õĽ��뼼����EP��ʾEPON������GP��ʾGPON������*/
    BYTE   AgentCirId[AGENT_CIR_ID_LENGTH];
    WORD16 CirIdLength;
    WORD16 RemoteIdLength;
    BYTE   AgentRemoteId[AGENT_REMOTE_ID_LENGTH];
    WORD32 PortLocType;
    CHAR   cDslamType[DSLAM_TYPE_STRING_LEN];/*AD|V2*/
} USER_ANI_PORT_INFO;

/**
 *  ����ҵ��ʹ�õ�·��Ϣ����
 */
typedef struct T_BRAS_VCC_CIR_ID
{               
    BYTE                    panel;             /**< @brief ��λ�� */
    BYTE                    subPanel;          /**< @brief �ӿ��� */
    WORD16                  porttype;          /**<@brief  �˿�����*/ 
    WORD32                  port;              /**< @brief �˿ں� */
    WORD16                  vlanId;            /**< @brief vlan id */
    WORD16                  secVlanId;         /**< @brief second-vlan id, ˫��vlan��Ч */
    WORD32                  outIdx;            /**< @brief ���ӿ����� */
    WORD16                  qinqEtherType;     /**< @brief qinq��̫���� */
    WORD16                  reserve2;          /* padding bits */
    WORD32                  vccIndex;          /**< @brief vcc�ӿ����� */
    WORD32                  vbuiIndex;         /**< @brief vbui�ӿ����� */
    WORD32                  L3IfIndex;         /**< @brief l3-access�ӿ����� */
    WORD32                  protocolType;      /**< @brief ����Э������ */
} BRAS_VCC_CIR_ID;


typedef union AuthInfo
{
    PAP_AUTH_INFO   pap;    /**<@brief �û���PAP ������Ϣ*/
    CHAP_AUTH_INFO  chap;   /**<@brief �û���CHAP������Ϣ*/
    MS_CHAPV1_AUTH_INFO mschapV1;/**<@brief �û���MSCHAPV1������Ϣ*/
    MS_CHAPV2_AUTH_INFO mschapV2;/**<@brief �û���MSCHAPV2������Ϣ*/
} AuthInfo;

typedef struct _Auth_Request {	
	BYTE user_type;	    // 用户类型,pppoe ipoe�?
    BYTE reserved[3];
	BYTE aaaRequestType;
	BYTE authtype;
	BYTE chapid;
	BYTE nasPortType;
    WORD16 session;
    BYTE mac[AUTHMGR_MAC_LENGTH];  // 用户的MAC地址
    CHAR username[AUTHMGR_MAX_USERNAME_SIZE];  // username和userpasswd约定最后一个字节存�?，即最大支持的用户名和密码�?AUTHMGR_MAX_USERNAME_SIZE - 1)字节
    CHAR userpasswd[AUTHMGR_MAX_PASSWD_SIZE];  // 在CHAP认证中，userpasswd[0]...userpasswd[15]�?6个字节复用来存放用户回复的response
    CHAR domain[AUTHMGR_MAX_DOMAIN_SIZE];
	BYTE challenge[AUTHMGR_MAX_CHALLENGE_SIZE];
	CHAR nasPortID[AUTHMGR_MAX_NAS_PORT_ID_SIZE];
	CHAR callingStationId[AUTHMGR_MAX_CALLING_STATION_ID_SIZE];
	WORD32 framedProtocol;
	WORD32 serviceType;
	WORD16 nasVlan;
	WORD16 nasPort;
//#ifdef WFNOS
    WORD32              subIp;                  /**< @brief �û��ĵ�ַ */
    USER_ANI_PORT_INFO aniInfo;
    BRAS_VCC_CIR_ID vccinfo;
    AuthInfo auth_info;
//#endif
}Auth_Request;

typedef struct _Auth_Response {
    WORD16 session;
    BYTE   mac[AUTHMGR_MAC_LENGTH];
    WORD32 vrf;
    WORD32 user_ip;
    CHAR   domain[AUTHMGR_MAX_DOMAIN_SIZE];
    WORD32 authResult; // 0表示认证成功，非0表示失败
    CHAR   authResultDesc[AUTHMGR_MAX_AUTH_RESULT_DESC_SIZE]; // 认证结果描述，比�?authentication success"/"authentication fail"
    WORD32 subscriberIP; // radius分配给subsriber的IP，主机序
    WORD32 subscriberIPMask;  // Subscribe IP netmask assigned by radius, in host byte order. Used in IPOE.
    WORD32 leaseTime;
    WORD32 gatewayaddr;  // Relay router/gateway address assigned by radius, in host byte order. Used in IPOE.
    WORD32 primaryDNS;  // radius分配给subsriber的Primary DNS IP，主机序
    WORD32 secondaryDNS; // radius分配给subsriber的Secondary DNS IP，主机序
}Auth_Response;

// Session Manager
#define SESSION_MGR_MAX_USERNAME_SIZE (32)
#define SESSION_MGR_MAC_SIZE (6)
#define SESSION_MGR_MAX_DOMAIN_SIZE (32)

typedef struct _Session_User_Ex {
    WORD16 session;
    BYTE  mac[SESSION_MGR_MAC_SIZE];
	BYTE  user_type;	         // 用户类型,pppoe ipoe�?
	BYTE  access_type; 	         // 用户接入类型 ipv4 ipv6或dual stack
	BYTE  auth_type;	         // 认证类型如radius认证
	BYTE  auth_state;			 // 用户认证状�?
	CHAR  user_name[SESSION_MGR_MAX_USERNAME_SIZE];
	CHAR  domain[SESSION_MGR_MAX_DOMAIN_SIZE];	
    WORD32   vrf;
    WORD32   user_ip;            // radius分配给subsriber的IP，主机序
    WORD32   primary_dns;        // radius分配给subsriber的Primary DNS IP，主机序
    WORD32   secondary_dns;      // radius分配给subsriber的Secondary DNS IP，主机序   
	WORD32   pool_id;		     // ip pool id�?
	WORD32   intern_vlan;		 // 接入内层vlan�?
	WORD32   extern_vlan;		 // 接入外层vlan�?
} Session_User_Ex;

typedef struct _Sm_Kick_User {
    WORD16 session;
    WORD32 userip;
    WORD32   vrf;
    BYTE mac[SESSION_MGR_MAC_SIZE];
}Sm_Kick_User;

typedef struct _Session_Offline {
    WORD16 user_type;
	WORD16 session;
	BYTE  mac[SESSION_MGR_MAC_SIZE];
}Session_Offline;

typedef struct {
    WORD32 result;
	WORD16 session;
	BYTE mac[SESSION_MGR_MAC_SIZE];
}UM_RESPONSE;

#endif//_FLEXBNG_AAA_H_
