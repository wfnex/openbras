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
/* OpenPortal: protocol between client and server. */
#ifndef OPENPORTAL_H
#define OPENPORTAL_H 1
#include <stdint.h>


#ifdef SWIG
#define OPORTAL_ASSERT(EXPR) /* SWIG can't handle OPORTAL_ASSERT. */
#elif !defined(__cplusplus)
/* Build-time assertion for use in a declaration context. */
#define OPORTAL_ASSERT(EXPR) \
extern int (*build_assert(void))[ sizeof(struct { \
unsigned int build_assert_failed : (EXPR) ? 1 : -1; })]
#else /* __cplusplus */
#define OPORTAL_ASSERT(_EXPR) typedef int build_assert_failed[(_EXPR) ? 1 : -1]
#endif /* __cplusplus */

#ifndef SWIG
#define OPORTAL_PACKED __attribute__((packed))
#else
#define OPORTAL_PACKED /* SWIG doesn't understand __attribute. */
#endif


/**
* portal版本类型
**/
typedef enum 
{
    PORTAL_VERSION_ONE = 0x01,
    PORTAL_VERSION_TWO = 0x02
}PORTAL_VERSION_TYPE;



/**
* portal认证类型
**/
typedef enum 
{
    PORTAL_CHAP_AUTH = 0x00,
    PORTAL_PAP_AUTH  = 0x01
}PORTAL_AUTH_PROTOCOL;


/**
* web强推标记
**/
typedef enum 
{
    PORTAL_WEB_INVALID   = 0,
    PORTAL_WEB_FORCE,
    PORTAL_WEB_AUTH,
}PORTAL_WEB_FLAG;


/**
* portal challenge请求返回结果
**/
typedef enum 
{
    PORTAL_CHALLENGE_REQUEST_OK = 0, /**<@brief 请求Challenge成功 */
    PORTAL_CHALLENGE_REQUEST_REJECT, /**<@brief 请求Challenge被拒绝 */
    PORTAL_CHALLENGE_REQUEST_ONLINE, /**<@brief 此链接已建立 */
    PORTAL_CHALLENGE_REQUEST_WAITING,/**<@brief 有一个用户正在认证过程中，请稍后再试 */
    PORTAL_CHALLENGE_REQUEST_FAIL,   /**<@brief 此用户请求Challenge失败（发生错误） */
    
    PORTAL_NUMBER_OF_CHALLENGE_ACK_ERROR_CODES
}PORTAL_CHALLENGE_ACK_ERROR_CODE;

/**
* portal下线结果应答类型
**/
typedef enum 
{
    PORTAL_LOGOUT_ACK_OK = 0,  /**<@brief 此用户下线成功 */
    PORTAL_LOGOUT_ACK_REJECT,  /**<@brief 此用户下线被拒绝 */
    PORTAL_LOGOUT_ACK_FAIL,    /**<@brief 此用户下线失败（发生错误） */
    
    PORTAL_NUMBER_OF_LOGOUT_ACK_ERROR_CODES
}PORTAL_LOGOUT_ACK_ERROR_CODE;

typedef enum
{
    PORTAL_AUTH_ACK_OK = 0,            /*此用户认证成功 */
    PORTAL_AUTH_ACK_REJECT,            /*此用户认证请求被拒绝 */
    PORTAL_AUTH_ACK_ONLINE,            /*此链接已建立 */
    PORTAL_AUTH_ACK_WAITING,           /*有一个用户正在认证过程中，请稍后再试 */
    PORTAL_AUTH_ACK_FAIL,              /*此用户认证失败（发生错误） */
    PORTAL_NUMBER_OF_AUTH_ACK_ERROR_CODES
}PORTAL_AUTH_ACK_ERROR_CODE;


/**
* portal报文类型
**/
typedef enum 
{
    PORTAL_REQ_CHALLENGE        = 0x01,
    PORTAL_ACK_CHALLENGE        = 0x02,
    PORTAL_REQ_AUTHEN           = 0x03,
    PORTAL_ACK_AUTHEN           = 0x04,
    PORTAL_REQ_LOGOUT           = 0x05,
    PORTAL_ACK_LOGOUT           = 0x06,
    PORTAL_AFF_ACK_AUTHEN       = 0x07,
    PORTAL_NTF_LOGOUT           = 0x08,
    PORTAL_REQ_INFO             = 0x09,
    PORTAL_ACK_INFO             = 0x0a,
    PORTAL_NTF_USERDISCOVER     = 0x0b,
    PORTAL_NTF_USERIPCHANGE     = 0x0c,
    PORTAL_AFF_NTF_USERIPCHANGE = 0x0d,
    PORTAL_ACK_NTF_LOGOUT       = 0x0e
}PORTAL_CODE;


/**
* portal属性
**/
typedef enum    
{
    PORTAL_USER_NAME      = 0x01,
    PORTAL_PASSWORD       = 0x02,
    PORTAL_CHAP_CHALLENGE = 0x03,
    PORTAL_CHAP_PASSWORD  = 0x04,
    PORTAL_TEXT_INFO      = 0x05,
    PORTAL_UP_LINK_FLUX   = 0x06,
    PORTAL_DOWN_LINK_FLUX = 0x07,
    PORTAL_VLAN_PORT      = 0x08,
    PORTAL_IP_CONFIG      = 0x09,
    PORTAL_BAS_IP         = 0x0a,
    PORTAL_USER_MAC       = 0x0b,
    PORTAL_DELAY_TIME     = 0x0c
}PORTAL_ATTRIBUTE_TYPE;


#define MAXIMUM_PORTAL_RX_PACKET_SIZE           (uint32_t)4096
#define PORTAL_CLIENT_PORT                      (uint32_t)2000
#define MAX_PORTAL_NAS_PORT_ID_LEN              (uint32_t)32

#define MAX_PORTAL_DEBUG_STRING_LEN             (uint32_t)300
#define MAX_PORTAL_DEBUG_PARA_LEN               (uint32_t)32

#define PORTAL_WEB_SERVER_MAX_KEY_LEN            (uint32_t)16
#define PORTAL_WEB_SERVER_MAX_URL_LEN            (uint32_t)128
#define PORTAL_WEB_SERVER_MAX_UAS_NAME_LEN       (uint32_t)32
#define PORTAL_WEB_SERVER_MAX_USER_NAME_LEN      (uint32_t)32
#define PORTAL_WEB_SERVER_MAX_UAS_ID_LEN         (uint32_t)32
#define PORTAL_WEB_SERVER_MAX_USER_MAC_KEY_LEN   (uint32_t)32
#define PORTAL_WEB_SERVER_DEFAULT_UDP_PORT       (uint16_t)50100
#define PORTAL_WEB_SERVER_DEFAULT_LISTENING_PORT (uint16_t)2000

#define MAX_AUTHENTICATOR_LEN                   (uint32_t)16
#define MAX_PORTAL_BUFFER_LEN                   (uint32_t)1024 

#define PORTAL_VERSION                          (uint32_t)0x01

#define PORTAL_SIZE_OF_ATTR_TYPE_AND_LEN_FIELDS (uint32_t)2
#define PORTAL_SOCKET_CLOSE_TIMEOUT             (uint32_t)2


#define PORTAL_MAX_SOCKET_CTRL_LEN              (uint32_t)200
#define PORTAL_AUTH_REQUEST_TIMEOUT             (uint32_t)30
#define UDP_HEADER_LEN                          (uint32_t)8
#define IP_HEADER_LEN                           (uint32_t)20
#define MAX_PORTAL_AUTH_STR_LEN                 (uint32_t)512

#define MAX_HTTP_REDIRECT_PACKET_LEN            (uint32_t)1024
#define PORTAL_MAX_HTTP_CONTEXT_ONE_LEN         (uint32_t)68
#define PORTAL_MAX_HTTP_CONTEXT_THREE_LEN       (uint32_t)141
#define MAX_PARA_LEN                            (uint32_t)512

#define MAX_PORTAL_IP_ADDRESS_STRING_LEN        (uint32_t)32
#define PORTAL_LOGOUT_NOTIFY_DESTIN_PORT        (uint32_t)50100
#define PORTAL_IP_CONFIG_SECOND_ADDRESS_ALLOT   (uint32_t)0xFFFFFFFF
#define PORTAL_IP_CONFIG_BAS_NOTIFY             (uint32_t)0x00000001

#define PORTAL_TCP_SOCKET_KEEPALIVE_TIME        (uint32_t)30
#define PORTAL_TCP_RE_TRANS_NUMBER              (uint32_t)3       /* 重传3次，默认16次*/
#define PORTAL_TCP_BREAK_WAIT_TIME              (uint32_t)5000    /* 断链时，TIME_WAIT状态停留5s，默认1分钟*/

#define PORTAL_MAX_RESTRICT_RECORD_NUM          (uint32_t)1000
#define PORTAL_MAX_TIME_STRING_LEN              (uint32_t)64
#define PORTAL_SERVER_WEB_OFF_NOTIFY_NUM        (uint32_t)50/*SERVER Dead状态每次扫描web off用户数50个*//*for web_off_notify*/
#define PORTAL_URL_PARAM_HEAD_LEN               (uint32_t)16
#define PORTAL_URL_LINK_CHARACTER_MAX_LEN       (uint32_t)6
#define PORTAL_HTTP_STRING_EOF_LEN              (uint32_t)2
#define PORTAL_SOCKET_ERR_TIMER                 (uint32_t)5
#define PORTAL_SOCKET_MAX_NUMBER                (uint32_t)2500
#define PORTAL_MAX_MAC_STR_LEN                  (uint32_t)20 /*url支持携带用户mac的长度*/
#define PORTAL_CHAP_CHALLENGE_LEN               (uint32_t)16
#define PORTAL_HTTP_TYPE_GET                    "GET"
#define PORTAL_HTTP_TYPE_POST                   "POST"

typedef struct PORTAL_ATTRIBUTE_ENTRY
{
    uint8_t       type;
    uint8_t       length;          /**包括type在内的长度值*/
    uint8_t       value[0];        /** 属性字段，变长*/
    //uint8_t       reserved[2];
}PORTAL_ATTRIBUTE_ENTRY_T;


/* Header on all portal packets. */
struct openportal_header
{
    uint8_t     version;         /* OPORTAL_VERSION. */
    uint8_t     code;            /* One of OPORTAL_TYPE_*. constants*/
    uint8_t     auth_type;       /* One of OPORTAL_AUTH_TYPE_*. constants*/
    uint8_t     reserverd;      /* Pad to 32 bits */
    uint16_t     serial_no;       /* Transaction id associated with server */
    uint16_t     req_id;          /* Transaction id associated with client */
    uint32_t     ip_address;      /* Client IP Address */
    uint16_t     user_port;       /* not user ,0 */
    uint8_t       error_code;       /* One of OPORTAL_ERROR_TYPE_*. constants*/
    uint8_t       attr_num;        /* Attribute number */
    uint8_t       authenticator[MAX_AUTHENTICATOR_LEN]; /*Authenticator (portal v2.0)*/
};

struct PortalServerCfg
{
    uint32_t            ip_address;   
    uint8_t             version;
    uint8_t             server_id;
    bool                uasip_valid_flag;
    uint8_t             server_state;
    uint16_t            port;
    uint16_t            use_counter;
    uint32_t            uas_ip;
    uint32_t            uasifindex;
    uint32_t            server_detect_timer;
    uint32_t            server_detect_count;
    uint16_t            listening_port;
    char                key[PORTAL_WEB_SERVER_MAX_KEY_LEN + 1];
    bool                first_setting_flag;
    char                url[PORTAL_WEB_SERVER_MAX_URL_LEN + 1];
    char                uas_name[PORTAL_WEB_SERVER_MAX_UAS_NAME_LEN + 1];
    char                user_name[PORTAL_WEB_SERVER_MAX_USER_NAME_LEN + 1];
    char                uas_id[PORTAL_WEB_SERVER_MAX_UAS_ID_LEN + 1];
    char                user_mac_key[PORTAL_WEB_SERVER_MAX_USER_MAC_KEY_LEN + 1];
};  


//OPORTAL_ASSERT(sizeof(struct openportal_header) == 16);

#endif

