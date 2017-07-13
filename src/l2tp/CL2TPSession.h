#ifndef CL2TPSESSION_H
#define CL2TPSESSION_H
#include "IL2TPInterface.h"
#include <stdint.h>
#include <unordered_map>
#include "CL2TPMessage.h"
class CL2TPTunnel;
class CL2TPSession : public IL2TPSession,public CReferenceControl
{
public:
    CL2TPSession(CL2TPTunnel &tunnel, uint16_t localcid, uint16_t peercid, L2tpSessionProxy &proxy, bool isProxy=false);
    virtual ~CL2TPSession();

    //IL2TPSession
    virtual void OpenWithSink(IL2TPSessionSink *psink);
    virtual int SendData(const char *data, size_t datasize);
    virtual uint16_t GetLocalCID() const;
    virtual uint16_t GetPeerCID() const;
    virtual uint16_t GetLocalTID() const;
    virtual uint16_t GetPeerTID() const;
    virtual void Disconnect();

    virtual uint32_t AddReference();
    virtual uint32_t ReleaseReference();

    void CallConnected();

    int StartCall();

    int HandleSessionPacket(const char *data, size_t datasize);
    int HandleControllMessage(const CL2TPControllMessage &controllmsg);

    int HandlePayLoad(const char *data, size_t datasize);
    int HandleOutgoingCallRequest(const CL2TPControllMessage &msg);
    int HandleOutgoingCallReply(const CL2TPControllMessage &msg);
    int HandleOutgoingCallConnected(const CL2TPControllMessage &msg);
    int HandleIncomingCallRequest(const CL2TPControllMessage &msg);
    int HandleIncomingCallReply(const CL2TPControllMessage &msg);
    int HandleIncomingCallConnected(const CL2TPControllMessage &msg);
    int HandleCallDisconnectNotify(const CL2TPControllMessage &msg);

private:
    CL2TPTunnel &m_tunnel;
    uint16_t m_localcid;
    uint16_t m_peercid;
    int m_fbit;
    int m_ourfbit;
    int m_lbit;
    bool m_isconnected;
    L2tpSessionProxy m_proxy;
    bool m_bisProxy;
    IL2TPSessionSink *m_psink;
};


#endif


