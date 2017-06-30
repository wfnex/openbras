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

#ifndef CRADIUSTRANSACTION_H
#define CRADIUSTRANSACTION_H

#include "aceinclude.h"
#include <stdint.h>

#include "CReferenceControl.h"
#include "CRadiusMessage.h"

#define TRTIMEOUT 0
#define TRRESPONSE 1
#define TRSENDERROR 2

class CRadiusConnector;

typedef std::function<void(CRadiusMessage *pRaMsg, int type)> TransactionResponse;

class CRadiusTransaction  : public ACE_Event_Handler,public CReferenceControl
{
public:
    CRadiusTransaction(uint8_t identifier,
        CRadiusConnector *pconnector, 
        CRadiusMessage &requestmsg,
        TransactionResponse callback);
    virtual ~CRadiusTransaction();
    //CReferenceControl
    virtual uint32_t AddReference();
    virtual uint32_t ReleaseReference();
    virtual void RecvResponse(CRadiusMessage *pResponse);
    virtual int handle_timeout (const ACE_Time_Value &current_time,
                                const void *act = 0);


    uint8_t GetId() const {return m_identifier;}
    virtual void SendRequest();
protected:
    uint32_t m_requestcount;
    uint8_t m_identifier;
    uint32_t m_timeout;
    uint32_t m_retrans;
    CRadiusConnector *m_pconnector;
    TransactionResponse m_callback;
    CRadiusMessage m_requestmsg;
};

#endif

