/*
 * Copyright (c) 2016-2017  Nanjing StarOS Technology Co., Ltd.  All rights reserved.
*/
#ifndef IL2TPINTERFACE_H
#define IL2TPINTERFACE_H
#include "aceinclude.h"
#include "CReferenceControl.h"
#include <stdint.h>
#include <unordered_map>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/utsname.h>

#define MAXSTRLEN 120           /* Maximum length of common strings */

typedef unsigned char _u8;
typedef unsigned short _u16;
typedef unsigned long long _u64;
#define HELLO_DELAY 60          /* How often to send a Hello message */
#define CTBIT(ver) (ver & 0x8000)       /* Determins if control or not */
#define CLBIT(ver) (ver & 0x4000)       /* Length bit present.  Must be 1
                                           for control messages */

#define CZBITS(ver) (ver &0x37F8)       /* Reserved bits:  We must drop 
                                           anything with these there */

#define CFBIT(ver) (ver & 0x0800)       /* Presence of Ns and Nr fields
                                           flow bit? */

#define CVER(ver) (ver & 0x0007)        /* Version of encapsulation */


#define NZL_TIMEOUT_DIVISOR 4   /* Divide TIMEOUT by this and
                                   you know how often to send
                                   a zero byte packet */

#define PAYLOAD_BUF 10          /* Provide 10 expansion bytes
                                   so we can "decompress" the
                                   payloads and simplify coding */
#if 1
#define DEFAULT_MAX_RETRIES 5    /* Recommended value from spec */
#else
#define DEFAULT_MAX_RETRIES 95   /* give us more time to debug */
#endif

#define DEFAULT_RWS_SIZE   4    /* Default max outstanding 
                                   control packets in queue */
#define DEFAULT_TX_BPS		10000000        /* For outgoing calls, report this speed */
#define DEFAULT_RX_BPS		10000000
#define DEFAULT_MAX_BPS		10000000        /* jz: outgoing calls max bps */
#define DEFAULT_MIN_BPS		10000   /* jz: outgoing calls min bps */
#define PAYLOAD_FUDGE		2       /* How many packets we're willing to drop */
#define MIN_PAYLOAD_HDR_LEN 6

#define UDP_LISTEN_PORT  1701
                                /* FIXME: MAX_RECV_SIZE, what is it? */
#define MAX_RECV_SIZE 4096      /* Biggest packet we'll accept */

#define OUR_L2TP_VERSION 0x100  /* We support version 1, revision 0 */

#define PTBIT(ver) CTBIT(ver)   /* Type bit:  Must be zero for us */
#define PLBIT(ver) CLBIT(ver)   /* Length specified? */
#define PFBIT(ver) CFBIT(ver)   /* Flow control specified? */
#define PVER(ver) CVER(ver)     /* Version */
#define PZBITS(ver) (ver & 0x14F8)      /* Reserved bits */
#define PRBIT(ver) (ver & 0x2000)       /* Reset Sr bit */
#define PSBIT(ver) (ver & 0x0200)       /* Offset size bit */
#define PPBIT(ver) (ver & 0x0100)       /* Preference bit */

/* Values for version */
#define VER_L2TP 2
#define VER_PPTP 3

/* Some PPP sync<->async stuff */
#define fcstab  ppp_crc16_table

#define PPP_FLAG 0x7e
#define PPP_ESCAPE 0x7d
#define PPP_TRANS 0x20

#define PPP_INITFCS 0xffff
#define PPP_GOODFCS 0xf0b8
#define PPP_FCS(fcs,c) (((fcs) >> 8) ^ fcstab[((fcs) ^ (c)) & 0xff])

/* Values for Randomness sources */
#define RAND_DEV 0x0
#define RAND_SYS 0x1
#define RAND_EGD 0x2

/* Control Connection Management */
#define SCCRQ 	1               /* Start-Control-Connection-Request */
#define SCCRP 	2               /* Start-Control-Connection-Reply */
#define SCCCN 	3               /* Start-Control-Connection-Connected */
#define StopCCN 4               /* Stop-Control-Connection-Notification */
/* 5 is reserved */
#define Hello	6               /* Hello */
/* Call Management */
#define OCRQ	7               /* Outgoing-Call-Request */
#define OCRP	8               /* Outgoing-Call-Reply */
#define OCCN	9               /* Outgoing-Call-Connected */
#define ICRQ	10              /* Incoming-Call-Request */
#define ICRP	11              /* Incoming-Call-Reply */
#define ICCN	12              /* Incoming-Call-Connected */
/* 13 is reserved */
#define CDN	14              /* Call-Disconnect-Notify */
/* Error Reporting */
#define WEN	15              /* WAN-Error-Notify */
/* PPP Sesssion Control */
#define SLI	16              /* Set-Link-Info */

#define MAX_MSG 16

#define TBIT 0x8000
#define LBIT 0x4000
#define RBIT 0x2000
#define FBIT 0x0800

#define VENDOR_ID 0             /* We don't have any extensions
                                   so we shoouldn't have to
                                   worry about this */

/*
 * Macros to extract information from length field of AVP
 */

#define AMBIT(len) (len & 0x8000)       /* Mandatory bit: If this is
                                           set on an unknown AVP, 
                                           we MUST terminate */

#define AHBIT(len) (len & 0x4000)       /* Hidden bit: Specifies
                                           information hiding */

#define AZBITS(len) (len & 0x3C00)      /* Reserved bits:  We must
                                           drop anything with any
                                           of these set.  */

#define ALENGTH(len) (len & 0x03FF)     /* Length:  Lenth of AVP */

#define MAXAVPSIZE 1023

#define MAXTIME 300             /* time to wait before checking
                                   Ns and Nr, in ms */

#define MBIT 0x8000             /* for setting */
#define HBIT 0x4000             /* Set on hidden avp's */

#define ASYNC_FRAMING 2
#define SYNC_FRAMING 1

#define ANALOG_BEARER 2
#define DIGITAL_BEARER 1

#define VENDOR_ERROR 6

#define ERROR_RESERVED 3
#define ERROR_LENGTH 2
#define ERROR_NOTEXIST 1
#define ERROR_NORES 4
#define ERROR_INVALID 6
#define RESULT_CLEAR 1
#define RESULT_ERROR 2
#define RESULT_EXISTS 3

#ifndef MIN
#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#endif

#define FIRMWARE_REV	0x0690  /* Revision of our firmware (software, in this case) */

#define VENDOR_NAME "wfnex.com"

struct control_hdr
{
    _u16 ver;                   /* Version and more */
    _u16 length;                /* Length field */
    _u16 tid;                   /* Tunnel ID */
    _u16 cid;                   /* Call ID */
    _u16 Ns;                    /* Next sent */
    _u16 Nr;                    /* Next received */
} __attribute__((packed));


struct payload_hdr
{
    _u16 ver;                   /* Version and friends */
    _u16 length;                /* Optional Length */
    _u16 tid;                   /* Tunnel ID */
    _u16 cid;                   /* Caller ID */
    _u16 Ns;                    /* Optional next sent */
    _u16 Nr;                    /* Optional next received */
    _u16 o_size;                /* Optional offset size */
//    _u16 o_pad;                 /* Optional offset padding */
} __attribute__((packed));

struct payload_hdr_mini
{
    _u16 ver;                   /* Version and friends */
    _u16 length;                /* Optional Length */
    _u16 tid;                   /* Tunnel ID */
    _u16 cid;                   /* Caller ID */
} __attribute__((packed));


struct avp_hdr
{
    _u16 length;
    _u16 vendorid;
    _u16 attr;
} __attribute__((packed));

struct unaligned_u16 {
	_u16	s;
} __attribute__((packed));


struct buffer
{
    int type;
    char *rstart;
    char *rend;
    char *start;
    int len;
    int maxlen;
};


inline void swaps (void *buf_v, int len)
{
    /* Reverse byte order (if proper to do so) 
        to make things work out easier */
    int x;
    struct hw { _u16 s; } __attribute__ ((packed)) *p = (struct hw *) buf_v;
    for (x = 0; x < len / 2; x++, p++)
        p->s = ntohs(p->s); 
}


inline void extract (void *buf, uint16_t *tunnel, uint16_t *call)
{
    /*
     * Extract the tunnel and call #'s, and fix the order of the 
     * version
     */

    struct payload_hdr *p = (struct payload_hdr *) buf;
    if (PLBIT (p->ver))
    {
        *tunnel = p->tid;
        *call = p->cid;
    }
    else
    {
        *tunnel = p->length;
        *call = p->tid;
    }
}

inline void fix_hdr (void *buf)
{
    /*
     * Fix the byte order of the header
     */

    struct payload_hdr *p = (struct payload_hdr *) buf;
    _u16 ver = ntohs (p->ver);
    if (CTBIT (p->ver))
    {
        /*
         * Control headers are always
         * exactly 12 bytes big.
         */
        swaps (buf, 12);
    }
    else
    {
        int len = 6;
        if (PSBIT (ver))
            len += 2;
        if (PLBIT (ver))
            len += 2;
        if (PFBIT (ver))
            len += 4;
        swaps (buf, len);
    }
}

/**/

/*-------------------L2TP代理验证结构结构中的长度宏----------------*/
#define MAX_IRLCPREQ_LEN              64                 /*初始收到LCP请求的最大长度*/
#define MAX_LSLCPREQ_LEN              64                 /*最后发出LCP协商的最大长度*/
#define MAX_LRLCPREQ_LEN              64                 /*最后收到LCP协商的最大长度*/
#define MAX_AUTHENNAME_LEN            73/*16*/                 /*代理验证名字的最大长度*/
#define MAX_AUTHENCHAL_LEN            16                 /*代理验证challenge的最大长度*/
#define MAX_AUTHENRESP_LEN            16                 /*代理验证回答的最大长度*/
#define MAX_RCV_CHALLENGE_LEN  (64)

/*-------------------------代理验证类型宏--------------------------*/
#define L2TP_PROXYAUTHTYPE_RSV         0  /*保留类型*/
#define L2TP_PROXYAUTHTYPE_TEXTPASS    1  /*明文用户名/密码交换*/
#define L2TP_PROXYAUTHTYPE_CHAP        2  /*代理验证使用CHAP*/
#define L2TP_PROXYAUTHTYPE_PAP         3  /*代理验证使用 PAP*/
#define L2TP_PROXYAUTHTYPE_NOAUTH      4  /*没有认证*/


struct L2tpSessionProxy
{
    uint8_t   sInitRcvLcpConf[MAX_IRLCPREQ_LEN];  /*初始收到LCP请求，字长不定 */
    uint16_t  wInitRcvLcpConfLen;                 /*初始收到的LCP的长度*/
    uint8_t   sLastSendLcpConf[MAX_LSLCPREQ_LEN]; /*最后发出LCP协商，字长不定 */
    uint16_t  wLastSendLcpConfLen;                /*最后发出LCP协商的长度*/
    uint8_t   sLastRcvLcpConf[MAX_LRLCPREQ_LEN];  /*最后收到LCP协商，字长不定*/
    uint16_t  wLastRcvLcpConfLen;                 /*最后收到LCP协商的长度*/
    uint16_t  wAuthenType;                        /*代理验证类型*/
    uint8_t   sAuthenName[MAX_AUTHENNAME_LEN];    /*代理验证名字，字长不定*/
    uint16_t  wAuthenNameLen;                     /*代理验证名字长度*/
    uint8_t   sAuthenChallenge[MAX_RCV_CHALLENGE_LEN];/*代理验证challenge*/
    uint16_t  wAuthenChallengeLen;                /*代理验证challenge长度 */
    uint16_t  wAuthenID;                          /*代理验证ID*/
    uint8_t   sAuthenResponse[MAX_AUTHENRESP_LEN];/*代理验证回答*/
    uint16_t  wAuthenResponseLen;                 /*代理验证回答长度*/
};


/*设置代理验证部分数据*/
#define SETS_SESSION_PROXY(s,s1)  (s= s1)

#define SETS_IRLCPREQ(s,s1,len)  memcpy(((L2tpSessionProxy*)s)->sInitRcvLcpConf,s1,len);     \
                                 ((L2tpSessionProxy*)s)->wInitRcvLcpConfLen=len

#define SETS_LSLCPREQ(s,s1,len)  memcpy(((L2tpSessionProxy*)s)->sLastSendLcpConf,s1,len);    \
                                 ((L2tpSessionProxy*)s)->wLastSendLcpConfLen=len

#define SETS_LRLCPREQ(s,s1,len)  memcpy(((L2tpSessionProxy*)s)->sLastRcvLcpConf,s1,len);    \
                                 ((L2tpSessionProxy*)s)->wLastRcvLcpConfLen=len

#define SETS_AUTHENTYPE(s,s1)    ((((L2tpSessionProxy*)s)->wAuthenType) = s1)

#define SETS_AUTHENNAME(s,s1,len)             \
                                 memcpy(((L2tpSessionProxy*)s)->sAuthenName,s1,len);      \
                                 ((L2tpSessionProxy*)s)->wAuthenNameLen = len

#define SETS_AUTHENCHAL(s,s1,len)      \
                                 memcpy(((L2tpSessionProxy*)s)->sAuthenChallenge,s1,len);    \
                                 ((L2tpSessionProxy*)s)->wAuthenChallengeLen = len

#define SETS_AUTHENID(s,s1)    \
                                 (((L2tpSessionProxy*)s)->wAuthenID = s1)

#define SETS_AUTHENRESP(s,s1,len)  \
                                 memcpy(((L2tpSessionProxy*)s)->sAuthenResponse,s1,len);              \
                                 ((L2tpSessionProxy*)s)->wAuthenResponseLen = len

class IL2TPSession;

class IL2TPTunnel;
class IL2TPTunnelSink;

class  IL2TPSessionSink 
{
public:
    virtual void OnDataRecv(const char *data, size_t datasize) = 0;
    virtual void OnSessionDisconnect(int reason,IL2TPSession *pSession) = 0;

protected:
    virtual ~IL2TPSessionSink() {}    
};


class  IL2TPSession : public IReferenceControl
{
public:
    virtual void OpenWithSink(IL2TPSessionSink *psink) = 0;
    virtual int SendData(const char *data, size_t datasize) = 0;
    virtual uint16_t GetLocalCID() const = 0;
    virtual uint16_t GetPeerCID() const = 0;
    virtual uint16_t GetLocalTID() const = 0;
    virtual uint16_t GetPeerTID() const = 0;
    virtual void Disconnect() = 0;
protected:
    virtual ~IL2TPSession() {} 
};


class  IL2TPTunnelIndication 
{
public:
    virtual void OnTunnelIndication(
        int aReason,
        IL2TPTunnel *pmakecall) = 0;

protected:
    virtual ~IL2TPTunnelIndication() {}
};

class IL2TPTunnelSink 
{
public:
    virtual void OnCallIndication(
        int aReason,
        IL2TPSession *pmakecall) = 0;


    virtual void OnDisconnect(int reason,IL2TPTunnel *ptunnel) = 0;

protected:
    virtual ~IL2TPTunnelSink() {}  
};

class IL2TPTunnel : public IReferenceControl
{
public:
    virtual ~IL2TPTunnel(){}

    virtual int Open(IL2TPTunnelSink *psink) = 0;
    virtual int MakeIncomingCall(L2tpSessionProxy &proxy,
        bool bUserProxy=false) = 0;
    virtual uint16_t GetLocalTID() const =0;
    virtual uint16_t GetPeerTID() const = 0;

    virtual ACE_INET_Addr GetPeerAddress() = 0;

    virtual int Disconnect() = 0;
};

class IL2TPLAC: public IReferenceControl
{
public:
    virtual ~IL2TPLAC(){}
    virtual int MakeTunnel(IL2TPTunnelIndication *psink,
        const ACE_INET_Addr &serveraddr) = 0;
};

class IL2TPManager
{
public:
    virtual ~IL2TPManager(){}
    static IL2TPManager* Instance();
    virtual int CreateLAC(CCmAutoPtr<IL2TPLAC> &CallMake) = 0;
};


#endif//CL2TPMAKECALL_H

