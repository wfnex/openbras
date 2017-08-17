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
#include "CAddSessionId.h"

CAddSessionId::CAddSessionId()
    :m_startid(0),m_endid(0)
{
}

CAddSessionId::~CAddSessionId()
{
}

//Init SessionId List
void CAddSessionId::InIt(uint16_t startid,uint16_t endid)
{
    m_startid = startid;
    m_endid = endid;
    for(int i=m_startid;i<=m_endid;i++)
    {
        m_SessionId.push_back(i);
    }
}

//Alloc SessionId
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

//Free SessionId
void CAddSessionId::FreeId(uint16_t id)
{
    if(id >= m_startid && id <= m_endid)
        m_SessionId.push_back(id);
    else
        ACE_DEBUG((LM_ERROR, "CAddSessionId::FreeId(), m_SessionId NULL.\n"));
}
