/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
**********************************************************************/
#ifndef CL2TPSERVERCHANNEL_H
#define CL2TPSERVERCHANNEL_H
#include "IL2TPInterface.h"
#include <stdint.h>
#include <unordered_map>
#include "CL2TPMessage.h"
#include "CL2TPSession.h"

class CL2TPLAC;


typedef std::unordered_map<uint16_t,CCmAutoPtr<CL2TPSession>> SESSIONType;

class CL2TPTunnel : public ACE_Event_Handler, public IL2TPTunnel,public CReferenceControl
{
public:
    CL2TPTunnel(CL2TPLAC &smgr, 
        const ACE_INET_Addr &remote_addr,
        uint16_t localtid,
        uint16_t peertid);
    virtual ~CL2TPTunnel();
    virtual uint32_t AddReference();
    virtual uint32_t ReleaseReference();


    //ACE_Event_Handler interface,??????????
    virtual int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE);
    virtual ACE_HANDLE get_handle (void) const;
    virtual int handle_close (ACE_HANDLE handle,
                            ACE_Reactor_Mask close_mask);
    virtual int handle_timeout (const ACE_Time_Value &current_time,
                              const void *act = 0);

    //IL2TPSession
    virtual int Open(IL2TPTunnelSink *psink);
    virtual int MakeIncomingCall(L2tpSessionProxy &proxy,
        bool bUserProxy=false);
    virtual uint16_t GetLocalTID() const{return m_ourtid;}
    virtual uint16_t GetPeerTID() const{return m_peertid;}

    virtual ACE_INET_Addr GetPeerAddress(){return m_remote_addr;}

    virtual int Disconnect();


    int AddSession(uint16_t callid,CCmAutoPtr<CL2TPSession> &session);
    CL2TPSession *FindSession(uint16_t callid);
    int RemoveSession(uint16_t callid);

    uint16_t GetIncreaseCid();
    uint16_t GetIncreaseNS();
    void DeCreaseNS();
    void InCreaseNR();

    int ControlZLB(CL2TPTunnel *tunnel, CL2TPSession *session=NULL);
    int CheckControlMessage(const CL2TPControllMessage & msg,CL2TPTunnel *t, CL2TPSession *session=NULL);

    int SendData(const char *data,size_t datasize);

    inline uint16_t GetControlSeqNum() const { return m_control_seq_num;}
    inline uint16_t GetControlRecSeqNum() const { return m_control_rec_seq_num;}
    int HandleSessionControllMessage(const CL2TPControllMessage &controllmsg);

    int UnInitSocket();
    int InitSocket();
    int StartConnectRequest();
    void OnSessionCallBack(int result ,CL2TPSession *psession);
    bool IsConnected(){return m_isconnected;}
protected:
    int HandleZLB(const CL2TPControllMessage &msg);
    int HandleControllMessage(const CL2TPControllMessage &controllmsg);
    int HandlePayLoadMessage(const CL2TPDataMessage & datamsg);
    int HandleStartControlConnectionRequest(const CL2TPControllMessage &msg);
    int HandleStartControlConnectionReply(const CL2TPControllMessage &msg);
    int HandleStartControlConnectionConnected(const CL2TPControllMessage &msg);
    int HandleStopControlConnectionNotification(const CL2TPControllMessage &msg);
    int HandleHello(const CL2TPControllMessage &msg);
    int HandleWANErrorNotify(const CL2TPControllMessage &msg);
    int HandleSetLinkInfo(const CL2TPControllMessage &msg);


private:
    CL2TPLAC &m_smgr;
    ACE_INET_Addr m_remote_addr;
    uint16_t m_ourtid;
    uint16_t m_peertid;
    int m_rws;                    /* Peer's Receive Window Size */
    uint16_t m_control_seq_num;       /* Sequence for next packet */
    uint16_t m_control_rec_seq_num;   /* Next expected to receive */
    int m_ourfc;
    int m_ourtb;

    uint16_t m_currentstate;

    SESSIONType m_sessions;
    uint16_t m_sessionidbase;
    bool m_isconnected;
    uint16_t m_helltimeoutcount;
    ACE_HANDLE m_handler;
    IL2TPTunnelSink *m_pTunnelSink;

    int m_requestcount;
};


#endif//CPORTALSERVERCHANNEL_H

