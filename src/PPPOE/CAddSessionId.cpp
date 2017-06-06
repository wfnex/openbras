#include "CAddSessionId.h"

CAddSessionId::CAddSessionId(uint16_t startid,uint16_t endid)
    :m_startid(startid),m_endid(endid)
{
}

CAddSessionId::~CAddSessionId()
{
}

void CAddSessionId::InIt(uint16_t startid,uint16_t endid)
{
    m_startid = startid;
    m_endid = endid;
    for(int i=m_startid;i<=m_endid;i++)
    {
        m_SessionId.push_back(i);
    }
}

uint16_t CAddSessionId::GetId()
{
    if(m_SessionId.empty())
    {
        ACE_DEBUG((LM_ERROR, "CAddSessionId::GetId(), m_SessionId NULL.\n"));
        return 0;
    }
    uint16_t id = m_SessionId.front();
    m_SessionId.pop_front();
    return id;
}

void CAddSessionId::FreeId(uint16_t id)
{
    if(id >= m_startid && id <= m_endid)
        m_SessionId.push_back(id);
    else
        ACE_DEBUG((LM_ERROR, "CAddSessionId::FreeId(), m_SessionId NULL.\n"));
}
