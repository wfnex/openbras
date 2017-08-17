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
#include "CRadiusScheme.h"

CRadiusScheme::CRadiusScheme(CRadiusManager &mgr,const std::string &schemename)
    :m_mgr(mgr)
    ,m_schemname(schemename)
    ,m_PrimaryAuth(this)
    ,m_PrimaryAcc(this)
    ,m_SecondaryAuth(this)
    ,m_SecondaryAcc(this)
{
}
CRadiusScheme::~CRadiusScheme()
{
    m_PrimaryAuth.StopConnect();
    m_PrimaryAcc.StopConnect();
    m_SecondaryAuth.StopConnect();
    m_SecondaryAcc.StopConnect();
}

//Start Primary Auth Connect
int CRadiusScheme::SetUpPrimaryAuthConn(ACE_INET_Addr &serveraddr)
{
    int result = m_PrimaryAuth.StartConnect(serveraddr);
    return result;
}

//Start Primary Acct Connect
int CRadiusScheme::SetUpPrimaryAcctConn(ACE_INET_Addr &serveraddr)
{
    int result = m_PrimaryAcc.StartConnect(serveraddr);
    return result;
}

//Start Secondary Auth Connect
int CRadiusScheme::SetUpSecondaryAuthConn(ACE_INET_Addr &serveraddr)
{
    int result = m_SecondaryAuth.StartConnect(serveraddr);
    //connect = &m_SecondaryAuth;
    return result;
}

//Start Secondary Acct Connect
int CRadiusScheme::SetUpSecondaryAcctConn(ACE_INET_Addr &serveraddr)
{
    int result = m_SecondaryAcc.StartConnect(serveraddr);
    //connect = &m_SecondaryAcc;
    return result;
}

//Send primary Access Message
int CRadiusScheme::SendAccessMessageP(CRadiusMessage &accessReqMsg,TransactionResponse callback)
{
    return m_PrimaryAuth.SendMessage(accessReqMsg,callback);
}

//Send primary Acct Message
int CRadiusScheme::SendAcctMessageP(CRadiusMessage &acctReqMsg,TransactionResponse callback)
{
    return m_PrimaryAcc.SendMessage(acctReqMsg,callback);
}

//Send Secondary Access Message
int CRadiusScheme::SendAccessMessageS(CRadiusMessage &accessReqMsg,TransactionResponse callback)
{
    return m_SecondaryAuth.SendMessage(accessReqMsg,callback);
}

//Send Secondary Acct Message
int CRadiusScheme::SendAcctMessageS(CRadiusMessage &acctReqMsg,TransactionResponse callback)
{
    return m_SecondaryAcc.SendMessage(acctReqMsg,callback);
}

int CRadiusScheme::SendAccessMessage(CRadiusMessage &accessReqMsg,TransactionResponse callback)
{
    return 0;
}

int CRadiusScheme::SendAcctMessage(CRadiusMessage &acctReqMsg,TransactionResponse callback)
{
    return 0;
}


