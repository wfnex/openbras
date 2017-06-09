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

int CRadiusScheme::SetUpPrimaryAuthConn(const ACE_INET_Addr &serveraddr)
{
    int result = m_PrimaryAuth.StartConnect(serveraddr);
    return result;
}

int CRadiusScheme::SetUpPrimaryAcctConn(const ACE_INET_Addr &serveraddr)
{
    int result = m_PrimaryAcc.StartConnect(serveraddr);
    return result;
}

int CRadiusScheme::SetUpSecondaryAuthConn(const ACE_INET_Addr &serveraddr)
{
    int result = m_SecondaryAuth.StartConnect(serveraddr);
    //connect = &m_SecondaryAuth;
    return result;
}

int CRadiusScheme::SetUpSecondaryAcctConn(const ACE_INET_Addr &serveraddr)
{
    int result = m_SecondaryAcc.StartConnect(serveraddr);
    //connect = &m_SecondaryAcc;
    return result;
}

int CRadiusScheme::SendAccessMessageP(CRadiusMessage &accessReqMsg,TransactionResponse callback)
{
    return m_PrimaryAuth.SendMessage(accessReqMsg,callback);
}

int CRadiusScheme::SendAcctMessageP(CRadiusMessage &acctReqMsg,TransactionResponse callback)
{
    return m_PrimaryAcc.SendMessage(acctReqMsg,callback);
}

int CRadiusScheme::SendAccessMessageS(CRadiusMessage &accessReqMsg,TransactionResponse callback)
{
    return m_SecondaryAuth.SendMessage(accessReqMsg,callback);
}

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


