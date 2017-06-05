/*
 * Copyright (c) 2014  Nanjing WFN Technology Co., Ltd.  All rights reserved.
*/

#include "CChap.h"

CChap::CChap(int code, CChap *pnext)
    :m_code(code)
    , m_pNextChap(pnext)
{
    
}
CChap::~CChap()
{
}

/*
 * Note: challenge and response arguments below are formatted as
 * a length byte followed by the actual challenge/response data.
 */
void CChap::GenerateChallenge(unsigned char *challenge)
{
}

int CChap::VerifyResponse(int id, char *name,
    unsigned char *secret, int secret_len,
    unsigned char *challenge, unsigned char *response,
    char *message, int message_space)
{
	return -1;
}

void CChap::MakeResponse(unsigned char *response, int id, char *our_name,
    unsigned char *challenge, char *secret, int secret_len,
    unsigned char *priv)
{

}

int CChap::CheckSuccess(int id, unsigned char *pkt, int len)
{
	return -1;
}

void CChap::HandleFailure(unsigned char *pkt, int len)
{
}


