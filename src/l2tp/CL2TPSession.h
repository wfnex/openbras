/***********************************************************************
 * 
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


