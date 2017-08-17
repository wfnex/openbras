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

#include "CPortalServerChannel.h"
#include "CPortalUserMgr.h"
#include "md5.h"
#include "CPortalServerManager.h"
#include "CPortalManager.h"

CPortalServerChannel::CPortalServerChannel(CPortalServerManager &smgr,
        const ACE_INET_Addr &local_addr,
        const ACE_INET_Addr &remote_addr,
        PortalServerCfg *pcfg)
        :m_smgr(smgr)
        ,m_local_addr(local_addr)
        ,m_remote_addr(remote_addr)
        ,m_isDetectedEnable(true)
        ,m_isDead(true)
        ,m_detectcount(0)
        ,m_last_detect_req_id(0)
        ,m_reqidbase(0)
        ,m_handler(ACE_INVALID_HANDLE)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::CPortalServerChannel,=%d\n",this)); 
    ::memcpy(&m_cfg, pcfg, sizeof(m_cfg));
}

CPortalServerChannel::~CPortalServerChannel()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::~CPortalServerChannel\n")); 
}

//Get Peer Addr
ACE_INET_Addr CPortalServerChannel::GetPeerAddr()
{
    return m_remote_addr;
}

//Get Local Addr
ACE_INET_Addr CPortalServerChannel::GetLocalAddr()
{
    return m_local_addr;
}

/// Get the I/O handle.
ACE_HANDLE CPortalServerChannel::get_handle (void) const
{
    return m_handler;
}

/// Set the I/O handle.
void CPortalServerChannel::set_handle (ACE_HANDLE fd)
{
    m_handler = fd;
}

//Portal Serialize Header
void CPortalServerChannel::PortalSerializeHeader(openportal_header *pHeader, 
                                                        uint8_t code, 
                                                        uint8_t auth_type,
                                                        uint8_t version_type,
                                                        uint16_t serial_no, 
                                                        uint16_t req_id, 
                                                        uint32_t sub_ip, 
                                                        uint8_t error_code)
{
    pHeader->version    = version_type;
    pHeader->code       = code;
    pHeader->auth_type  = auth_type;
    pHeader->reserverd   = 0x00;
    pHeader->serial_no  = serial_no;
    pHeader->req_id     = req_id;
    pHeader->ip_address = sub_ip;
    pHeader->user_port  = 0x00;
    pHeader->error_code = error_code;
    pHeader->attr_num   = 0x00;   
}

//Send Data
int CPortalServerChannel::SendData(const char *data, size_t datasize)
{
    ACE_TCHAR remote_str[80]={0}, local_str[80]={0};

    m_local_addr.addr_to_string (local_str, 80);
    m_remote_addr.addr_to_string (remote_str, 80);

    ssize_t result=ACE_OS::sendto (get_handle(),
                         (char const *)data,
                         datasize,
                         0,
                         (sockaddr const *) m_remote_addr.get_addr (),
                         m_remote_addr.get_size ());

    ACE_DEBUG ((LM_DEBUG,
            ACE_TEXT ("(%P|%t)  CPortalServerChannel::SendData ")
            ACE_TEXT ("from %s, to %s,fd=%d,datasize=%d,result=%d\n"),
            local_str,
            remote_str,
            get_handle(),
            datasize,
            result));
    if (result<0)
    {
        return -1;
    }
    return 0;
}

//Auth Response
int CPortalServerChannel::OnAuthResponse(const Auth_Response *response)
{
    ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::OnAuthResponse,begin %d\n",this)); 
    uint8_t error_code = PORTAL_AUTH_ACK_OK;
    size_t sendsize = 0;
    char *ppacket = NULL;
    size_t packetsize = 0;
    char packet[MAXIMUM_PORTAL_RX_PACKET_SIZE]={0};
    openportal_header *presponse = (openportal_header *)packet;

    CPortalUser *puser = NULL;

    m_smgr.GetPortalManager().GetPortalUserMgr().FindUser(response->user_ip,response->vrf,puser);
    if (puser)
    {
        puser->GetAuthInfo(ppacket, packetsize);
    }
    else
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::OnAuthResponse,can not find user=%d\n",response->user_ip)); 
        return -1;
    }
    
    if ((ppacket == NULL) || (packetsize == 0))
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::OnAuthResponse,can not find authinfo,packetsize=%d,ppacket=%d\n",packetsize,ppacket));
        return -1;
    }

    if (response->authResult == 0)
    {
        error_code = PORTAL_AUTH_ACK_OK;
        puser->SetAuthResult(true);
    }
    else
    {
        error_code = PORTAL_AUTH_ACK_FAIL;
        puser->SetAuthResult(false);
    }
    openportal_header *prequest = (openportal_header *)ppacket;


    presponse->version    = prequest->version;
    presponse->code       = PORTAL_ACK_AUTHEN;
    presponse->auth_type  = prequest->auth_type;
    presponse->reserverd   = 0x00;
    presponse->serial_no  = prequest->serial_no;
    presponse->req_id     = 0;
    presponse->ip_address = prequest->ip_address;
    presponse->user_port  = 0x00;
    presponse->error_code = (uint8_t)error_code;
    presponse->attr_num   = 0x00; 


    if(PORTAL_VERSION_ONE == prequest->version)
    {
        sendsize = sizeof(openportal_header) - MAX_AUTHENTICATOR_LEN;
    }
    else if(PORTAL_VERSION_TWO == prequest->version)/*版本二为兼容华为设计 */
    {
        sendsize = sizeof(openportal_header);
    }

    if(PORTAL_VERSION_TWO == prequest->version)
    {
        ::memcpy(presponse->authenticator, prequest->authenticator, MAX_AUTHENTICATOR_LEN);
        AddAuthentication(presponse);
    }

    ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::OnAuthResponse, send Response size=%d\n",sendsize)); 

    if (SendData((char*)presponse, sendsize) == -1)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::HandleChallengeRequest error\n"));
    } 

    if (error_code == PORTAL_AUTH_ACK_FAIL)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::OnAuthResponse, error remove user\n")); 
        m_smgr.GetPortalManager().GetPortalUserMgr().RemoveUser(response->user_ip,response->vrf);  
    }

    ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::OnAuthResponse, end %d\n",this)); 
    return 0;
}

//Receive Data
int CPortalServerChannel::RcvData(const char *msg, size_t msgsize)
{
    ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::RcvData msgsize=%d\n",msgsize)); 

    int result = -1;
    size_t temp_len = 0;
    PORTAL_ATTRIBUTE_ENTRY_T *p_attrib = NULL;

    openportal_header *ppacket = (openportal_header*)msg;
    /*1. 报文长度小于最小值*/
    if(PORTAL_VERSION_ONE == ppacket->version)
    {
        temp_len = sizeof(openportal_header)- MAX_AUTHENTICATOR_LEN;
        if(msgsize < sizeof(openportal_header)- MAX_AUTHENTICATOR_LEN)
        {
            return -1;
        }
    }
    else if(PORTAL_VERSION_TWO == ppacket->version)
    {
        temp_len = sizeof(openportal_header);
        if(msgsize < sizeof(openportal_header))
        {
            return -1;
        }
    }
    else /*目前不是1就是2，其他都不对*/
    {
        return -1;
    }
    /*认证类型目前只有PAP和CHAP*/
    if((PORTAL_CHAP_AUTH != ppacket->auth_type)&&(PORTAL_PAP_AUTH != ppacket->auth_type))
    {
        return -1;
    }
    if(ppacket->attr_num > 0)  /*带属性字段*/
    {
        for(uint32_t attrib_num = 0; attrib_num < ppacket->attr_num; attrib_num++)
        {
            /*2. 属性还没扫描完，已经到报文末尾*/
            if(temp_len == msgsize) 
            {
                return -1;
            } 
            
            p_attrib = (PORTAL_ATTRIBUTE_ENTRY_T *)((char*)msg + temp_len);
            if(NULL == p_attrib)
            {
                return -1;
            }
            
            /*3. 属性的长度跟属性数据长度不匹配，已经超过报文末尾*/
            if((temp_len + p_attrib->length) > msgsize )
            {
                return -1;
            } 
            
            /*4. 属性的长度< 2 ，不符合协议标准*/
            if(p_attrib->length < PORTAL_SIZE_OF_ATTR_TYPE_AND_LEN_FIELDS)
            {
                return -1;
            }
            
            temp_len += p_attrib->length; /*已扫描长度增加，使之可以取到下一个属性*/
        }
        
    }

    /*5. 用户是否认证过*/
    //CPortalUser *puser = NULL;
    //CPortalUserMgr::Instance()->FindUser(ntohl(ppacket->ip_address),0, puser);
    //if (puser != NULL)
    //{
    //    return 0;
    //}

    if (ppacket->version !=m_cfg.version)
    {
        return -1;
    }
    
    switch (ppacket->code)
    {
        case PORTAL_REQ_CHALLENGE:
            result=HandleChallengeRequest(msg,msgsize);
            break;
        case PORTAL_ACK_CHALLENGE:
            result=HandleChallengeResponse(msg,msgsize);
            break;
        case PORTAL_REQ_AUTHEN:
            result=HandleAuthenRequest(msg,msgsize);
            break;
        case PORTAL_ACK_AUTHEN:
            result=HandleAuthenResponse(msg,msgsize);
            break;
        case PORTAL_REQ_LOGOUT:
            result=HandleLogoutRequest(msg,msgsize);
            break;
        case PORTAL_ACK_LOGOUT:
            result=HandleLogoutResponse(msg,msgsize);
            break;
        case PORTAL_AFF_ACK_AUTHEN:
            result=HandleAuthenAFFResponse(msg,msgsize);
            break;
        case PORTAL_NTF_LOGOUT:
            result=HandleNTFLogout(msg,msgsize);
            break;
        case PORTAL_REQ_INFO:
            result=HandleInfoResponse(msg,msgsize);
            break;
        case PORTAL_ACK_INFO:
            result=HandleInfoRequest(msg,msgsize);
            break;
        case PORTAL_NTF_USERDISCOVER:
            result=HandleNTFUserDiscovery(msg,msgsize);
            break;
        case PORTAL_NTF_USERIPCHANGE:
            result=HandleUserIPChangeNotify(msg,msgsize);
            break;
        case PORTAL_AFF_NTF_USERIPCHANGE:
            result=HandleAFFNTFUserIPChange(msg,msgsize);
            break;
        case PORTAL_ACK_NTF_LOGOUT:
            result=HandleNTFLogoutResponse(msg,msgsize);
            break;
        default:
            result=-1;
            break;
    }
    
    return result;
}

//Check Authenticator
bool CPortalServerChannel::CheckAuthenticator(openportal_header *pheader)
{
    PORTAL_ATTRIBUTE_ENTRY_T *p_attrib      = NULL;
    uint32_t                    i             = 0;
    uint32_t                    temp_len      = 0;
    uint32_t                    attrib_num    = 0;
    uint32_t                    buffer_length = 0;  
    char                      auth_str[MAX_PORTAL_AUTH_STR_LEN] = {0};  
    char                      digest[MAX_AUTHENTICATOR_LEN]     = {0}; 
    MD5_CTX                   Md5Ctx;
    openportal_header    PacketHeader;

    if(NULL == pheader)
    {
        return false; 
    }   

    if (PORTAL_VERSION_ONE == pheader->version)
    {
        return true;
    }
       
    ::memset(&Md5Ctx, 0, sizeof(MD5_CTX));
    ::memset(&PacketHeader,0,sizeof(openportal_header));
    
    /*packet head  no authenticator*/
    temp_len = sizeof(openportal_header) - MAX_AUTHENTICATOR_LEN;

    ::memcpy(&PacketHeader, pheader, temp_len);
    
    ::memcpy(auth_str, (CHAR *)&PacketHeader, temp_len);
    buffer_length = buffer_length + temp_len;

    /*authenticator  0(16) */
    ::memset(&auth_str[buffer_length] ,0 ,MAX_AUTHENTICATOR_LEN);
    buffer_length = buffer_length + MAX_AUTHENTICATOR_LEN;

    /*attrib*/
    temp_len = sizeof(openportal_header);
    for(attrib_num = 0; attrib_num < pheader->attr_num; attrib_num++)
    {
        p_attrib = (PORTAL_ATTRIBUTE_ENTRY_T *)((char*)pheader + temp_len);
        if(NULL == p_attrib)
        {
           return false;
        }        
        if((buffer_length + p_attrib->length) > MAX_PORTAL_AUTH_STR_LEN)
        {
           return false;
        }
        ::memcpy((CHAR *)&auth_str[buffer_length] ,p_attrib , p_attrib->length);

        buffer_length += p_attrib->length;/*统计属性总长度*/
        temp_len += p_attrib->length; /*已扫描长度增加，使之可以取到下一个属性*/
    }
    
    /*share key*/
    if(0 != strlen(m_cfg.key))  //2011-1-19 支持key为空
    {
        if((buffer_length + strlen(m_cfg.key)) > MAX_PORTAL_AUTH_STR_LEN)
        {
           return false;
        }
        ::memcpy((CHAR *)&auth_str[buffer_length] , m_cfg.key, strlen(m_cfg.key));
        buffer_length = buffer_length + strlen(m_cfg.key);
    }
    
    /*MD5 +++ */
    MD5_Init(&Md5Ctx);
    MD5_Update(&Md5Ctx,(BYTE *)auth_str, buffer_length);
    MD5_Final((BYTE *)digest, &Md5Ctx);
    
    if (0 != ::memcmp((char *)digest, (char *)pheader->authenticator, MAX_AUTHENTICATOR_LEN))
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::CheckAuthenticator failure\n")); 
        return false;
    }
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::CheckAuthenticator OK!!!!!!!!\n")); 

    return true;

}

//Challenge Request Handle
int CPortalServerChannel::HandleChallengeRequest(const char *msg, size_t msgsize)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::HandleChallengeRequest,msgsize=%d\n",msgsize)); 
    size_t length = 0;
    char packet[MAXIMUM_PORTAL_RX_PACKET_SIZE]={0};
    char challenge[PORTAL_CHAP_CHALLENGE_LEN]={0};
    openportal_header *ppacket = (openportal_header*)msg;
    openportal_header *presponse = (openportal_header *)packet;
    PrintPortalHead(ppacket);

    if (CheckAuthenticator(ppacket) == false)
    {
        ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::HandleChallengeRequest,CheckAuthenticator failure\n"));
        return -1;
    }

    if((PORTAL_VERSION_TWO == ppacket->version)&&(strlen(m_cfg.key) != 0))
    {
        ::memcpy(challenge,m_cfg.key,MAX_AUTHENTICATOR_LEN);   
    }
    else
    {
        MakeChallenge(challenge,sizeof(challenge));
    }

    presponse->version    = ppacket->version;
    presponse->code       = PORTAL_ACK_CHALLENGE;
    presponse->auth_type  = ppacket->auth_type;
    presponse->reserverd   = 0x00;
    presponse->serial_no  = ppacket->serial_no;
    presponse->req_id     = 0;
    presponse->ip_address = ppacket->ip_address;
    presponse->user_port  = 0x00;
    presponse->error_code = (uint8_t)PORTAL_CHALLENGE_REQUEST_OK;
    presponse->attr_num   = 0x01; 

    if(PORTAL_VERSION_ONE == ppacket->version)
    {
        length = sizeof(openportal_header) - MAX_AUTHENTICATOR_LEN;
    }
    else if(PORTAL_VERSION_TWO == ppacket->version)/*版本二为兼容华为设计 */
    {
        length = sizeof(openportal_header);
    }
    
    AddAttribute((char*)presponse, &length, PORTAL_CHAP_CHALLENGE, 18, m_cfg.key);

    if(PORTAL_VERSION_TWO == ppacket->version)
    {
        ::memcpy(presponse->authenticator, ppacket->authenticator, MAX_AUTHENTICATOR_LEN);
        AddAuthentication(presponse);
    }

    PrintPortalHead(presponse);

    if (SendData((char*)presponse, length) == -1)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::HandleChallengeRequest error\n"));
        return -1;
    }

    return 0;
}

//Make Challenge
void CPortalServerChannel::MakeChallenge(char *pStr, size_t len)
{
    uint8_t           round   = len / sizeof( WORD32 );
    uint8_t           apped   = len % sizeof( WORD32 );
    uint8_t           ii      = 0;
    static uint8_t    rdApp   = 0;
    uint32_t         rd      = 0;

    if (NULL == pStr)
    {
        return; 
    }

    for ( ii = 0; ii < round; ii++ )
    {
        rd = ::rand();
        ::memcpy( pStr + ii * sizeof(uint32_t), &rd, sizeof( uint32_t ) );
    }

    if ( 0 != apped )
    {
        for ( ii = 0; ii < apped; ii++ )
        {
            pStr[ii + round * sizeof(uint32_t)] = rdApp++;
        }
    }
    
    return;
}

//Challenge Response Handle
int CPortalServerChannel::HandleChallengeResponse(const char *msg, size_t msgsize)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::HandleChallengeResponse\n")); 

    //openportal_header *ppacket = (openportal_header*)msg;
    return 0;
}

//Auth Request Handle
int CPortalServerChannel::HandleAuthenRequest(const char *msg, size_t msgsize)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::HandleAuthenRequest begin, msgsize=%d\n",msgsize));
    uint8_t error_code = PORTAL_AUTH_ACK_OK;
    size_t length = msgsize;
    size_t sendsize = 0;
    uint32_t                    attrib_num = 0;
    uint32_t                    attrib_value_length = 0;
    PORTAL_ATTRIBUTE_ENTRY_T *p_attrib   = NULL;
    char packet[MAXIMUM_PORTAL_RX_PACKET_SIZE]={0};
    openportal_header *presponse = (openportal_header *)packet;

    openportal_header *prequest = (openportal_header*)msg;
    Auth_Request authrequest;
    ::memset(&authrequest, 0, sizeof(authrequest));

    if (msgsize == 0)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::HandleAuthenRequest2 msg=%d,length=%d\n",msg,length));
        return -1;
    }

    if (CheckAuthenticator(prequest) == false)
    {
        return -1;
    }

    /*5. 用户是否认证过*/
    CPortalUser *puser = NULL;
    m_smgr.GetPortalManager().GetPortalUserMgr().FindUser(ntohl(prequest->ip_address),0, puser);
    if (puser != NULL)
    {
        if (puser->IsAuthOK())
        {
            error_code = PORTAL_AUTH_ACK_ONLINE;
        }
        else
        {
            error_code = PORTAL_AUTH_ACK_WAITING;
        }
    }
    else
    {
        CPortalUser *pnewuser=NULL;
        if (m_smgr.GetPortalManager().GetPortalUserMgr().CreateUser(ntohl(prequest->ip_address),0,pnewuser) != 0)
        {
            ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::HandleAuthenRequest CreateUser error\n"));
            error_code = PORTAL_AUTH_ACK_REJECT;
        }
        if (pnewuser == NULL)
        {
            ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::HandleAuthenRequest CreateUser puser == NULL\n"));
            error_code = PORTAL_AUTH_ACK_REJECT;
        }
        else
        {
            pnewuser->SetAuthInfo(msg, length);
            pnewuser->SetPortalServerIP(m_remote_addr);
            pnewuser->SetPortalLocalIP(m_local_addr);
        }
    }

    ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::HandleAuthenRequest error_code =%d\n",error_code));

    if (error_code != PORTAL_AUTH_ACK_OK)
    {
        presponse->version    = prequest->version;
        presponse->code       = PORTAL_ACK_AUTHEN;
        presponse->auth_type  = prequest->auth_type;
        presponse->reserverd   = 0x00;
        presponse->serial_no  = prequest->serial_no;
        presponse->req_id     = 0;
        presponse->ip_address = prequest->ip_address;
        presponse->user_port  = 0x00;
        presponse->error_code = (uint8_t)error_code;
        presponse->attr_num   = 0x00; 


        if(PORTAL_VERSION_ONE == prequest->version)
        {
            sendsize = sizeof(openportal_header) - MAX_AUTHENTICATOR_LEN;
        }
        else if(PORTAL_VERSION_TWO == prequest->version)/*版本二为兼容华为设计 */
        {
            sendsize = sizeof(openportal_header);
        }

        if(PORTAL_VERSION_TWO == prequest->version)
        {
            ::memcpy(presponse->authenticator, prequest->authenticator, MAX_AUTHENTICATOR_LEN);
            AddAuthentication(presponse);
        }

        if (SendData((char*)presponse, sendsize) == -1)
        {
            ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::HandleChallengeRequest error\n"));
            return -1;
        } 
    }
    else
    {
        authrequest.user_type=USER_TYPE_PORTAL;
        authrequest.subIp=ntohl(prequest->ip_address);
        if(PORTAL_CHAP_AUTH == prequest->auth_type)
        {
            authrequest.authtype = AUTH_CHAP;
        }
        else
        {
            authrequest.authtype = AUTH_PAP;
        }

        if(prequest->version == PORTAL_VERSION_ONE)
        {
            sendsize = sizeof(openportal_header)- MAX_AUTHENTICATOR_LEN;
        }
        else if(prequest->version == PORTAL_VERSION_TWO )
        {
            sendsize = sizeof(openportal_header);
        }  

        for(attrib_num = 0; attrib_num < prequest->attr_num; attrib_num++)
        {
            p_attrib = (PORTAL_ATTRIBUTE_ENTRY_T *)((char*)prequest + sendsize);
            attrib_value_length = p_attrib->length - PORTAL_SIZE_OF_ATTR_TYPE_AND_LEN_FIELDS;
            
            switch(p_attrib->type)
            {
                case PORTAL_USER_NAME: /*user name*/   
                    ::memset(authrequest.username, 0x00, sizeof(authrequest.username));
                    if(attrib_value_length > sizeof(authrequest.username) - 1)
                    {
                        return -1;
                    }

                    ::memcpy(authrequest.username, p_attrib->value, attrib_value_length);
                    break;

                case PORTAL_PASSWORD:  /*password*/
                    ::memset(authrequest.auth_info.pap.subPwd.str, 0x00, sizeof(authrequest.auth_info.pap.subPwd));

                    if(attrib_value_length > sizeof(authrequest.auth_info.pap.subPwd) - 1)   
                    {
                        return -1;
                    }

                    ::memcpy(authrequest.auth_info.pap.subPwd.str, p_attrib->value, attrib_value_length);
                    break;

                case PORTAL_CHAP_CHALLENGE:    /*challenge*/
                    ::memset(authrequest.auth_info.chap.challenge.str, 0x00, sizeof(authrequest.auth_info.chap.challenge));

                    if(attrib_value_length > sizeof(authrequest.auth_info.chap.challenge))   
                    {
                        return -1;
                    }

                    ::memcpy(authrequest.auth_info.chap.challenge.str, p_attrib->value, attrib_value_length);
                    break;

                case PORTAL_CHAP_PASSWORD: /*chap_password*/
                    ::memset(authrequest.auth_info.chap.chapPwd.str, 0x00, sizeof(authrequest.auth_info.chap.chapPwd));

                    if(attrib_value_length > sizeof(authrequest.auth_info.chap.chapPwd)) 
                    {
                        return -1;
                    }

                    ::memcpy(authrequest.auth_info.chap.chapPwd.str, p_attrib->value, attrib_value_length);
                    break;
                   
                default: 
                    break;
            }
            
            length += p_attrib->length;
        }
        
        if (IAuthManager::instance().AuthRequest(&authrequest) == -1)
        {
            ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::HandleAuthenRequest error\n"));
            return -1;
        }
    }

    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::HandleAuthenRequest end\n"));

    return 0;

}

//Auth Response Handle
int CPortalServerChannel::HandleAuthenResponse(const char *msg, size_t msgsize)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::HandleAuthenResponse\n"));

    //openportal_header *ppacket = (openportal_header*)msg;
    return 0;

}

//Logout Request Handle
int CPortalServerChannel::HandleLogoutRequest(const char *msg, size_t msgsize)
{
    size_t length = 0;

    char packet[MAXIMUM_PORTAL_RX_PACKET_SIZE]={0};
    openportal_header *prequest = (openportal_header*)msg;
    openportal_header *presponse = (openportal_header *)packet;
    CPortalUser *puser = NULL;

    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::HandleLogoutRequest, presonse=%d\n",presponse));

    PrintPortalHead(prequest);
    if (CheckAuthenticator(prequest) == false)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::HandleLogoutRequest CheckAuthenticator failure\n"));
        return -1;
    }
    
    m_smgr.GetPortalManager().GetPortalUserMgr().FindUser(ntohl(prequest->ip_address),0, puser);
    if (puser)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::HandleLogoutRequest FindUser failure\n"));
        m_smgr.GetPortalManager().GetPortalUserMgr().RemoveUser(ntohl(prequest->ip_address),0);
    }

    presponse->version    = prequest->version;
    presponse->code       = PORTAL_ACK_LOGOUT;
    presponse->auth_type  = prequest->auth_type;
    presponse->reserverd   = 0x00;
    presponse->serial_no  = prequest->serial_no;
    presponse->req_id     = 0;
    presponse->ip_address = prequest->ip_address;
    presponse->user_port  = 0x00;
    presponse->error_code = (uint8_t)PORTAL_LOGOUT_ACK_OK;
    presponse->attr_num   = 0x00; 

    if(PORTAL_VERSION_ONE == prequest->version)
    {
        length = sizeof(openportal_header) - MAX_AUTHENTICATOR_LEN;
    }
    else if(PORTAL_VERSION_TWO == prequest->version)/*版本二为兼容华为设计 */
    {
        length = sizeof(openportal_header);
    }

    
    if(PORTAL_VERSION_TWO == prequest->version)
    {
        AddAuthentication(presponse);
    }

    if (SendData((char*)presponse, length) == -1)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::HandleChallengeRequest error\n"));
        return -1;
    }
    return 0;

}

//Logout Response Handle
int CPortalServerChannel::HandleLogoutResponse(const char *msg, size_t msgsize)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::HandleLogoutResponse\n"));

    //openportal_header *ppacket = (openportal_header*)msg;
    return 0;

}

//Auth AFF Response Handle
int CPortalServerChannel::HandleAuthenAFFResponse(const char *msg, size_t msgsize)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::HandleAuthenAFFResponse\n"));
    //openportal_header *ppacket = (openportal_header*)msg;
    return 0;
}

//NTF Logout Handle
int CPortalServerChannel::HandleNTFLogout(const char *msg, size_t msgsize)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::HandleNTFLogout\n"));
    //openportal_header *ppacket = (openportal_header*)msg;
    return 0;
}

//Info Response Handle
int CPortalServerChannel::HandleInfoResponse(const char *msg, size_t msgsize)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::HandleInfoResponse\n"));
    //openportal_header *ppacket = (openportal_header*)msg;
    return 0;
}

//Info Request Handle
int CPortalServerChannel::HandleInfoRequest(const char *msg, size_t msgsize)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::HandleInfoRequest\n"));
    //openportal_header *ppacket = (openportal_header*)msg;
    return 0;
}

//NTF User Discovery Handle
int CPortalServerChannel::HandleNTFUserDiscovery(const char *msg, size_t msgsize)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::HandleNTFUserDiscovery\n"));
    //openportal_header *ppacket = (openportal_header*)msg;
    return 0;
}

//UserIp Change Notify Handle
int CPortalServerChannel::HandleUserIPChangeNotify(const char *msg, size_t msgsize)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::HandleUserIPChangeNotify\n"));
    //openportal_header *ppacket = (openportal_header*)msg;
    return 0;
}

//AFF NTF UserIp Change Handle
int CPortalServerChannel::HandleAFFNTFUserIPChange(const char *msg, size_t msgsize)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::HandleUserIPChangeNotify\n"));
    //openportal_header *ppacket = (openportal_header*)msg;
    return 0;
}

//NTF Logout Response Handle 
int CPortalServerChannel::HandleNTFLogoutResponse(const char *msg, size_t msgsize)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::HandleUserIPChangeNotify\n"));

    return 0;
}

//Add Attribute
void CPortalServerChannel::AddAttribute(char *vpPacket, 
                        size_t *vpOffset, 
                        uint8_t tlv_type, 
                        uint8_t tlv_length, 
                        char *vpTlv_value)
{
    PORTAL_ATTRIBUTE_ENTRY_T *p_attrib = NULL;
    if((NULL == vpPacket) ||(NULL == vpOffset) || (NULL == vpTlv_value))
    {
        ACE_ASSERT(0);
        return; 
    }

    p_attrib = (PORTAL_ATTRIBUTE_ENTRY_T*)(vpPacket + *vpOffset);

    p_attrib->type = tlv_type;
    p_attrib->length = tlv_length;
    ::memcpy(p_attrib->value, vpTlv_value, (tlv_length - sizeof(tlv_type) - sizeof(tlv_length)));

    *vpOffset += tlv_length;

    return;
}

//Add Authentication
void CPortalServerChannel::AddAuthentication(openportal_header *phead)
{
    PORTAL_ATTRIBUTE_ENTRY_T *p_attrib      = NULL;
    uint32_t                    i             = 0;
    uint32_t                    temp_len      = 0;
    uint32_t                    attrib_num    = 0;
    uint32_t                    buffer_length = 0;
    char                      auth_str[MAX_PORTAL_AUTH_STR_LEN] = {0};
    uint8_t                     digest[MAX_AUTHENTICATOR_LEN] = {0};
    MD5_CTX                   Md5Ctx;
    openportal_header           Packet;

    if(NULL == phead)
    {
        return; 
    }
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::AddAuthentication begin\n"));

    ::memset(&Md5Ctx,0,sizeof(MD5_CTX));
    ::memset(&Packet,0,sizeof(openportal_header));

    ::memcpy(&Packet, phead, sizeof(openportal_header));

    /*packet header  + request request_authenticator*/
    ::memcpy((char*)auth_str, (char*)&Packet, sizeof(openportal_header));
    buffer_length = buffer_length + sizeof(openportal_header);

    /*attrib*/
    temp_len = sizeof(openportal_header);
    for(attrib_num = 0; attrib_num < phead->attr_num; attrib_num++)
    {
        ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::AddAuthentication attrib_num=%d\n",attrib_num));
        p_attrib = (PORTAL_ATTRIBUTE_ENTRY_T *)((char*)phead + temp_len);
        if(NULL == p_attrib)
        {
           return;
        }

        if((buffer_length + p_attrib->length) > MAX_PORTAL_AUTH_STR_LEN)
        {
           return;
        }
        ::memcpy((CHAR *)&auth_str[buffer_length], p_attrib, p_attrib->length);
        
        buffer_length += p_attrib->length;/*统计属性总长度*/
        temp_len += p_attrib->length; /*已扫描长度增加，使之可以取到下一个属性*/
    }

    /*share key*/
    if(0 != strlen(m_cfg.key))  
    {
        if((buffer_length + strlen(m_cfg.key)) > MAX_PORTAL_AUTH_STR_LEN)
        {
           return;
        }
        ::memcpy((CHAR *)&auth_str[buffer_length],m_cfg.key, strlen(m_cfg.key));
        
        buffer_length = buffer_length + strlen(m_cfg.key);
    }

    /*MD5 +++ */
    MD5_Init(&Md5Ctx);
    MD5_Update(&Md5Ctx,(BYTE *)auth_str,buffer_length);
    MD5_Final(digest, &Md5Ctx);

    ::memcpy(phead->authenticator,digest, MAX_AUTHENTICATOR_LEN);   
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::AddAuthentication end\n"));

    return;

}

//Get Server Config
PortalServerCfg &CPortalServerChannel::GetServerCfg()
{
    return m_cfg;
}

//Set Server Config
void CPortalServerChannel::SetServerCfg(PortalServerCfg &cfg)
{
    ::memcpy(&m_cfg, 0, sizeof(PortalServerCfg));
}

// Logout Request Notify
int CPortalServerChannel::NotifyLogoutRequest(uint32_t userid)
{
    char packet[MAXIMUM_PORTAL_RX_PACKET_SIZE]={0};
    m_reqidbase++;
    size_t length=0;

    openportal_header *pHeader = (openportal_header *)packet;
    PortalSerializeHeader(pHeader, 
        PORTAL_NTF_LOGOUT, 
        PORTAL_CHAP_AUTH, 
        m_cfg.version,0,m_reqidbase, userid, 0x00);

    if(PORTAL_VERSION_ONE == m_cfg.version)
    {
        length = sizeof(openportal_header)-MAX_AUTHENTICATOR_LEN;
    }
    else if(PORTAL_VERSION_TWO == m_cfg.version)
    {
        length = sizeof(openportal_header);
        ::memset(pHeader->authenticator, 0, MAX_AUTHENTICATOR_LEN);
        AddAuthentication(pHeader);
    } 
    if (SendData(packet, sizeof(openportal_header)) == -1)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalServerChannel::NotifyLogoutRequest error\n"));
        return -1;
    }   
    return 0;
}

//Print Portal Head
void CPortalServerChannel::PrintPortalHead(openportal_header *pHeader)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalServerChannel::PrintPortalHead:\n")); 
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) version:%d\n",pHeader->version)); 
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) code:%d\n",pHeader->code));
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) auth_type:%d\n",pHeader->auth_type));
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) serial_no:%d\n",ntohs(pHeader->serial_no)));
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) req_id:%d\n",ntohs(pHeader->req_id)));
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) ip_address:%d\n",ntohl(pHeader->ip_address)));
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) user_port:%d\n",ntohs(pHeader->user_port)));
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) error_code:%d\n",pHeader->error_code));
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) attr_num:%d\n",pHeader->attr_num));
}


