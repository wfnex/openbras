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


#ifndef CRADIUSCONNECTOR_H
#define CRADIUSCONNECTOR_H

#include "aceinclude.h"
#include <stdint.h>
#include <unordered_map>
#include "openportal.h"
#include "CReferenceControl.h"
//#include "CRadiusScheme.h"
//class CPortalClient;
class CRadiusMessage;
class CRadiusScheme;
class TransactionResponse;
class CRadiusTransaction;
class CRadiusConnector : public CReferenceControl,public ACE_Event_Handler
{
public:
    CRadiusConnector(CRadiusScheme* scheme);
    virtual ~CRadiusConnector();
    virtual int StartConnect(const ACE_INET_Addr &peeraddr);
    virtual int StopConnect();

    virtual int handle_timeout (const ACE_Time_Value &current_time,
                              const void *act = 0);

    //ACE_Event_Handler interface，子类可以重载这些函数
    virtual int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE);
    virtual ACE_HANDLE get_handle (void) const;
    virtual int handle_close (ACE_HANDLE handle,
                            ACE_Reactor_Mask close_mask);
    int SendData(const char *data, size_t datasize);

    std::string GetSharedKey();
    int GetTimeOut();
    int GetRetrans();
    int FindTransaction(uint8_t id, CCmAutoPtr<CRadiusTransaction> &trans);
    int SendMessage(CRadiusMessage &accessReqMsg,TransactionResponse callback);
protected:
    ACE_HANDLE m_handler;
    CCmAutoPtr<CRadiusScheme> m_pscheme;
    ACE_INET_Addr m_peeraddr;
    ACE_Thread_Mutex m_mutex;
    std::unordered_map<uint8_t, CCmAutoPtr<CRadiusTransaction> > m_trans;
    int m_accessRequest;       // Client send
    int m_accessAccept;       // Server send
    int m_accessReject;       // Server send
    int m_accountingRequest;  // Client send
    int m_accountingResponse;  // Server send
    int m_nIdentifierBase;
};

#endif//CDHCPSERVER_H


