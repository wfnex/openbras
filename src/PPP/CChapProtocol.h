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


// Chap Server

#ifndef CCHAP_PROTOCOL_H
#define CCHAP_PROTOCOL_H

#include "aceinclude.h"
#include "CPPPProtocol.h"
#include "BaseDefines.h"
#include "aim_ex.h"
#include "CAuthen.h"
#include <string>

// MD5 related, temporarily put here
#define MD5_HASH_SIZE		16
#define MD5_MIN_CHALLENGE	16
#define MD5_MAX_CHALLENGE	24

/*
 * These limits apply to challenge and response packets we send.
 * The +4 is the +1 that we actually need rounded up.
 */
#define CHAL_MAX_PKTLEN	(PPP_HDRLEN + CHAP_HDRLEN + 4 + MAX_CHALLENGE_LEN + AUTHMGR_MAX_USERNAME_SIZE)
#define RESP_MAX_PKTLEN	(PPP_HDRLEN + CHAP_HDRLEN + 4 + MAX_RESPONSE_LEN + AUTHMGR_MAX_USERNAME_SIZE)

struct chap_server_state 
{
	int flags;
	int id;
	char *name;
	//struct chap_digest_type *digest;
	int challenge_xmits;
	int challenge_pktlen;
	unsigned char challenge[CHAL_MAX_PKTLEN];
	//char message[256];  // message field in CHAP success and failure, RADIUS generate it, so commentted here

    chap_server_state()
    {
        ::memset(this, 0, sizeof(struct chap_server_state)); 
    }
};

class CChapProtocolSvr : public CPPPProtocol, public ACE_Event_Handler
{
public:
    CChapProtocolSvr(IAuthenSvrSink *psink);
    virtual ~CChapProtocolSvr();

    // for class CPPPProtocol
    virtual void Init();
    virtual void Input(unsigned char *packet ,size_t size);
    virtual void Protrej();
    virtual void LowerUp();
    virtual void LowerDown();
    virtual void Open();
    virtual void Close(char *reason);

    // for class ACE_Event_Handler
    virtual int handle_timeout (const ACE_Time_Value &current_time,
                                const void *act = 0);      

    void ResponseAuthenResult(int result, std::string &reason);
    void chap_auth_peer(std::string &our_name, int digest_code);

    // Gets
    std::string &GetUserName() {return m_username;}

protected:
    void CancelTimer();
    void StartTimer(int seconds);

    void chap_handle_response(int id, unsigned char *pkt, int len);
    void chap_generate_challenge();

    // MD5 related, temporarily put here
    void chap_md5_generate_challenge(unsigned char *cp);

private:
    IAuthenSvrSink *m_psink;    
    int m_timeout_time;  // timeout for CHAP
    int m_max_transmits; // max number of transmits for challenge
    int m_rechallenge_time; // interval for rechallenge
    struct chap_server_state m_server;
    std::string m_username;
};

#endif // CCHAP_PROTOCOL_H

