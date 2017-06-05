/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
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

