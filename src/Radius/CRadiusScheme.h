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


#ifndef CRADIUSSCHEME_H
#define CRADIUSSCHEME_H

#include "aceinclude.h"
#include <stdint.h>
#include "CReferenceControl.h"

class CRadiusManager;
class CRadiusScheme : public CReferenceControl
{
public:
    CRadiusScheme(CRadiusManager &mgr,const std::string &schemename);
    virtual ~CRadiusScheme();
    int SetUpPrimaryAuthConn(ACE_INET_Addr &serveraddr);
    int SetUpPrimaryAcctConn(ACE_INET_Addr &serveraddr);
    int SetUpSecondaryAuthConn(ACE_INET_Addr &serveraddr);
    int SetUpSecondaryAcctConn(ACE_INET_Addr &serveraddr);
    CRadiusManager &GetRadiusManager()
    {
        return m_mgr;
    }
    int SendAccessMessageP(CRadiusMessage &accessReqMsg,TransactionResponse callback);
    int SendAcctMessageP(CRadiusMessage &acctReqMsg,TransactionResponse callback);
    int SendAccessMessageS(CRadiusMessage &accessReqMsg,TransactionResponse callback);
    int SendAcctMessageS(CRadiusMessage &acctReqMsg,TransactionResponse callback);
    int SendAccessMessage(CRadiusMessage &accessReqMsg,TransactionResponse callback);
    int SendAcctMessage(CRadiusMessage &acctReqMsg,TransactionResponse callback);
private:
    CRadiusManager &m_mgr;
    std::string m_schemname;
    CRadiusConnector m_PrimaryAuth;
    CRadiusConnector m_PrimaryAcc;
    CRadiusConnector m_SecondaryAuth;
    CRadiusConnector m_SecondaryAcc;
};

#endif//CRADIUSSCHEME_H
