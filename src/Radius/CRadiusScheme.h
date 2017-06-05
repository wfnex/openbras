/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
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
