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


#ifndef CPPPOEDISCOVERY_HANDLER_H
#define CPPPOEDISCOVERY_HANDLER_H

#include "CPPPOEPacketHandler.h"
#include "BNGPError.h"
#include "openssl/md5.h"

/* Random seed for cookie generation */
#define SEED_LEN 16
#define MD5_LEN 16
#define COOKIE_LEN (MD5_LEN + sizeof(pid_t)) /* Cookie is 16-byte MD5 + PID of server */

/* Function passed to parsePacket */
typedef void ParseFunc(WORD16 type,
        		       WORD16 len,
        		       BYTE *data,
        		       void *extra);

class CPPPOE;
class CSession;

int MD5_Init(MD5_CTX *c);
int MD5_Update(MD5_CTX *c, const void *data, size_t len);
int MD5_Final(unsigned char *md, MD5_CTX *c);


class CPPPOEDiscoveryHandler : public CPPPOEPacketHandler
{
public:
    CPPPOEDiscoveryHandler(CPPPOE &pppoe);
    virtual ~CPPPOEDiscoveryHandler();

    CPPPOE &GetPppoe();
        
    SWORD32 Init();

    // For class CPPPOEPacketHandler
    virtual SWORD32 ProcessPacket(const CHAR *packet, ssize_t size);
    
    void processPADI(PPPoEPacket *packet, SWORD32 len);
    void processPADR(PPPoEPacket *packet, SWORD32 len);
    void processPADT(PPPoEPacket *packet, SWORD32 len);
    void sendErrorPADS(BYTE *dest, SWORD32 errorTag, CHAR *errorMsg);
    void sendPADT(CSession *session, CHAR const *msg);
    static SWORD32 parsePacket(PPPoEPacket *packet, ParseFunc *func, void *extra);

protected:
    BNGPResult InitCookieSeed();
    void genCookie(BYTE const *peerEthAddr, BYTE const *myEthAddr, BYTE const *seed, BYTE *cookie);
    
    static void parsePADITags(WORD16 type, WORD16 len, BYTE *data, void *extra);
    static void parsePADRTags(WORD16 type, WORD16 len, BYTE *data, void *extra);
    static void parseLogErrs(WORD16 type, WORD16 len, BYTE *data, void *extra);
    static void pktLogErrs(CHAR const *pkt, WORD16 type, WORD16 len, BYTE *data, void *extra);
    
    void dumpPacket(PPPoEPacket *packet, CHAR const *dir);
    void dumpHex(BYTE const *buf, SWORD32 len);
    
public:
    // ParseFunc加入这个参数(CCmAutoPtr<CClientDiscovery> &clientDiscovery)后，下面这几个起静态变量作用的成员可以放到
    // CClientDiscovery中了，一个client用一个CClientDiscovery实例。但由于是Reactor单进程模式，现在的解决方法仍可行。
    // 后续有需要再修改。
    static PPPoETag hostUniq;
    static PPPoETag relayId;
    static PPPoETag receivedCookie;
    static PPPoETag requestedService;
    /* Requested max_ppp_payload */
    static WORD16 max_ppp_payload;
    
private:
    CPPPOE &m_pppoe;
    BYTE m_cookieSeed[SEED_LEN];
};

#endif//CPPPOEDISCOVERY_HANDLER_H

