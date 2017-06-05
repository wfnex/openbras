/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
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

uint32_t CRadiusTransaction::AddReference()
{
    return CReferenceControl::AddReference();
}

uint32_t CRadiusTransaction::ReleaseReference()
{
    return CReferenceControl::ReleaseReference();
}

void CRadiusTransaction::RecvResponse(CRadiusMessage *pResponse)
{
    if (m_callback)
    {
        m_callback(pResponse,TRRESPONSE);
    }
    ACE_Reactor::instance()->cancel_timer(this);
    m_pconnector->RemoveTransaction(m_identifier);
}

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

void CRadiusTransaction::SendRequest()
{
    return m_pconnector->SendData(reinterpret_cast<const char *>(m_requestmsg.data().buffer),
        ntohs(m_requestmsg.data().msgHdr.length));
}




