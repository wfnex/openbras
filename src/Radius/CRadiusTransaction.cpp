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

#include "CRadiusTransaction.h"
#include "CRadiusConnector.h"
CRadiusTransaction::CRadiusTransaction(uint8_t identifier,
        CRadiusConnector *pconnector, 
        CRadiusMessage &requestmsg,
        TransactionResponse callback)
    :m_requestcount(0)
    ,m_identifier(identifier)
    ,m_pconnector(pconnector)
    ,m_callback(callback)
    ,m_requestmsg(requestmsg)
{
    ACE_DEBUG((LM_DEBUG,ACE_TEXT ("(%P|%t) CRadiusTransaction::CRadiusTransaction, identifier=%d\n"),identifier));

    m_timeout = m_pconnector->GetTimeOut();
    m_retrans = m_pconnector->GetRetrans();
    ACE_Time_Value debal(m_timeout);
    ACE_Time_Value intervals(m_timeout);
    if (ACE_Reactor::instance()->schedule_timer(this,0,debal,intervals) == -1)
    {
        ACE_ERROR((LM_ERROR,ACE_TEXT ("(%P|%t) can't register with reactor\n")));
    }
}

CRadiusTransaction::~CRadiusTransaction()
{
    ACE_DEBUG((LM_DEBUG,ACE_TEXT ("(%P|%t) CRadiusTransaction::~CRadiusTransaction\n")));
    ACE_Reactor::instance()->cancel_timer(this);

}

//Add Reference
uint32_t CRadiusTransaction::AddReference()
{
    return CReferenceControl::AddReference();
}

//Release Reference
uint32_t CRadiusTransaction::ReleaseReference()
{
    return CReferenceControl::ReleaseReference();
}

//Receive Response
void CRadiusTransaction::RecvResponse(CRadiusMessage *pResponse)
{
    if (m_callback)
    {
        m_callback(pResponse,TRRESPONSE);
    }
    ACE_Reactor::instance()->cancel_timer(this);
    m_pconnector->RemoveTransaction(m_identifier);
}

//Timeout Handle
int CRadiusTransaction::handle_timeout (const ACE_Time_Value &current_time,
                            const void *act)

{
    ACE_DEBUG((LM_DEBUG,ACE_TEXT ("(%P|%t) CTransaction::handle_time_out\n")));

    if (m_requestcount >= m_retrans)
    {
        if (m_callback)
        {
            m_callback(NULL,TRTIMEOUT);
        }
        m_pconnector->RemoveTransaction(m_identifier);
    }
    else
    {
        SendRequest();
        m_requestcount++;
    }

    return 0;
}

//Send Request
void CRadiusTransaction::SendRequest()
{
    m_pconnector->SendData(reinterpret_cast<const char *>(m_requestmsg.data().buffer),
        ntohs(m_requestmsg.data().msgHdr.length));
}




