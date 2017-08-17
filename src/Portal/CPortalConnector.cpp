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


#include "CPortalConnector.h"
#include "openportal.h"
#include "CPortalServerChannel.h"
#include "CPortalServerManager.h"
#include "CPortalManager.h"
#include "CPortalConfig.h"
#include "md5.h"

CPortalConnector::CPortalConnector(CPortalClient &client,PortalServerCfg &cfg)
    :m_handler(ACE_INVALID_HANDLE)
    ,m_portalclient(client)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalConnector::CPortalConnector\n"));
    ::memcpy(&m_cfg,&cfg,sizeof(cfg));
}

CPortalConnector::~CPortalConnector()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalConnector::~CPortalConnector\n"));
    StopDetect();
    // Commented by mazhh: There is no need to call Close() here, as ~CIPOEModule() will call it.
}

//Start Connect
int CPortalConnector::StartConnect(const ACE_INET_Addr &peeraddr)
{
    m_peeraddr = peeraddr;
    
    ACE_DEBUG ((LM_DEBUG, "CPortalConnector::StartConnect\n"));
    
    ACE_TCHAR localaddr_str[BUFSIZ]={0};
    m_peeraddr.addr_to_string (localaddr_str, sizeof localaddr_str);
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalConnector::StartConnect, peeraddr=%s\n", localaddr_str));
    
    m_handler = ACE_OS::socket (AF_INET, SOCK_DGRAM, 0);
    if (m_handler == ACE_INVALID_HANDLE)
    {
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalConnector::StartConnect, handle error\n"));
        return -1;
    }

    int one = 1;
    if (ACE_OS::setsockopt (m_handler,
                            SOL_SOCKET,
                            SO_REUSEADDR,
                            (const char*) &one,
                            sizeof one) == -1)
      {
          ACE_OS::closesocket(m_handler);
          m_handler = ACE_INVALID_HANDLE;
          ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalConnector::StartConnect, setsockopt SO_REUSEADDR error\n"));
          return -1;
      }

#if 0
    if ((ACE_Addr&)m_localaddr == ACE_Addr::sap_any)
    {
        if (ACE::bind_port (m_handler,
            INADDR_ANY,
            AF_INET) == -1)
        {
            ACE_OS::closesocket(m_handler);
            m_handler = ACE_INVALID_HANDLE;
            ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalClient::StartListen, bind_port  error\n"));
            return -1;
        }
    }
    else
    {
        if (ACE_OS::bind (m_handler,
                                 reinterpret_cast<sockaddr *> (m_localaddr.get_addr ()),
                                 m_localaddr.get_size ()) == -1)
        {
            ACE_OS::closesocket(m_handler);
            m_handler = ACE_INVALID_HANDLE;
            ACE_ERROR((LM_ERROR,
                     ACE_TEXT ("(%P|%t) CPortalClient::StartListen: %p\n"),
                     ACE_TEXT ("bind error")));

            return -1;
        }
    }
#endif
    int size = 262144; // 256 * 1024 = 262144
    
    if (ACE_OS::setsockopt(m_handler, SOL_SOCKET,
                                    SO_RCVBUF,
                                    (char *) &size,
                                    sizeof (size)) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalConnector::StartConnect, setsockopt SO_RCVBUF error. size = %d\n", size));
        return -1;
    }

    if (ACE_OS::setsockopt(m_handler, SOL_SOCKET,
                                    SO_SNDBUF,
                                    (char *) &size,
                                    sizeof (size)) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalConnector::StartConnect, setsockopt SO_SNDBUF error. size = %d\n", size));
        return -1;
    }

    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalConnector::StartConnect, register_handler\n"));

    // Register with the reactor for input.
    if (ACE_Reactor::instance()->register_handler (this,ACE_Event_Handler::READ_MASK) == -1)
    {
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        ACE_ERROR_RETURN ((LM_ERROR,
                 ACE_TEXT ("(%P|%t) CPortalConnector::StartConnect: %p\n"),
                 ACE_TEXT ("register_handler for input")),
                -1);
    }

    //StartDetect(

    return 0;
}

//Stop Connect
int CPortalConnector::StopConnect()
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalConnector::StopConnect\n"));

    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CPortalConnector::StopConnect \n")));

    if (ACE_Reactor::instance())
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CPortalConnector::StopConnect remove_handler\n")));
        ACE_Reactor::instance()->remove_handler (this,
                                                ACE_Event_Handler::ALL_EVENTS_MASK |
                                                ACE_Event_Handler::DONT_CALL);
    }
    
    if (m_handler != ACE_INVALID_HANDLE)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) CPortalConnector::StopConnect close socket \n"))); 
        ACE_OS::closesocket(m_handler);
        m_handler = ACE_INVALID_HANDLE;
    }

    return 0;
}

#if 0
int CPortalConnector::GetLocalAddr (ACE_INET_Addr &address) const
{
    address = m_localaddr;

    return 0;
}
#endif

//Timeout Handle
int CPortalConnector::handle_timeout (const ACE_Time_Value &current_time,
                          const void *act)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalConnector::handle_timeout m_detectcount=%d,isdead=%d\n",m_detectcount,m_isDead));

    if (m_detectcount > 5)
    {
        m_isDead=false;
    }
    
    SendDetect();
    m_detectcount++;

    return 0;
}

//Input Handle
int CPortalConnector::handle_input (ACE_HANDLE fd)
{
    int result = -1;
    size_t temp_len = 0;
    PORTAL_ATTRIBUTE_ENTRY_T *p_attrib = NULL;

    ACE_INET_Addr addrRecv;
    //ACE_INET_Addr localAddr;
    static char szBuf[1024*16];
    //GetLocalAddr(localAddr);
    int addr_len = addrRecv.get_size ();
    ssize_t rval_recv = ACE_OS::recvfrom(fd, szBuf, sizeof(szBuf),0, (sockaddr *)addrRecv.get_addr(),(int*)&addr_len);
    addrRecv.set_size (addr_len);
    ACE_TCHAR remote_str[80]={0};
    ACE_TCHAR local_str[80]={0};
    addrRecv.addr_to_string (remote_str, 80);
    //localAddr.addr_to_string(local_str,80);

    ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%P|%t) CPortalConnector::handle_input - ")
              ACE_TEXT ("activity occurred on handle %d!,%s\n"),
              m_handler,
              remote_str));
    
    ACE_DEBUG ((LM_INFO,
            ACE_TEXT ("(%P|%t) CPortalConnector::handle_input - ")
            ACE_TEXT ("message from %d bytes received.\n"),
            rval_recv));

    if ((rval_recv == -1) || (rval_recv == 0))
    {
          ACE_ERROR_RETURN ((LM_ERROR,
                             ACE_TEXT ("(%P|%t) CPortalConnector::handle_input - ")
                             ACE_TEXT ("closing daemon (fd = %d)\n"),
                             this->get_handle ()),
                            0); 
    }

    openportal_header *ppacket = (openportal_header*)szBuf;
    /*1. 报文长度小于最小值*/
    if(PORTAL_VERSION_ONE == ppacket->version)
    {
        temp_len = sizeof(openportal_header)- MAX_AUTHENTICATOR_LEN;
        if(rval_recv < sizeof(openportal_header)- MAX_AUTHENTICATOR_LEN)
        {
            return 0;
        }
    }
    else if(PORTAL_VERSION_TWO == ppacket->version)
    {
        temp_len = sizeof(openportal_header);
        if(rval_recv < sizeof(openportal_header))
        {
            return 0;
        }
    }
    else /*目前不是1就是2，其他都不对*/
    {
        return 0;
    }
    /*认证类型目前只有PAP和CHAP*/
    if((PORTAL_CHAP_AUTH != ppacket->auth_type)&&(PORTAL_PAP_AUTH != ppacket->auth_type))
    {
        return 0;
    }
    if(ppacket->attr_num > 0)  /*带属性字段*/
    {
        for(uint32_t attrib_num = 0; attrib_num < ppacket->attr_num; attrib_num++)
        {
            /*2. 属性还没扫描完，已经到报文末尾*/
            if(temp_len == rval_recv) 
            {
                return 0;
            } 
            
            p_attrib = (PORTAL_ATTRIBUTE_ENTRY_T *)((char*)ppacket + temp_len);
            if(NULL == p_attrib)
            {
                return 0;
            }
            
            /*3. 属性的长度跟属性数据长度不匹配，已经超过报文末尾*/
            if((temp_len + p_attrib->length) > rval_recv )
            {
                return 0;
            } 
            
            /*4. 属性的长度< 2 ，不符合协议标准*/
            if(p_attrib->length < PORTAL_SIZE_OF_ATTR_TYPE_AND_LEN_FIELDS)
            {
                return 0;
            }
            
            temp_len += p_attrib->length; /*已扫描长度增加，使之可以取到下一个属性*/
        }
        
    }

    if (ppacket->version !=m_cfg.version)
    {
        return 0;
    }
    
    if (ppacket->code != PORTAL_ACK_NTF_LOGOUT)
    {
        return 0;
    }
    
    if (m_last_detect_req_id == ntohs(ppacket->req_id))
    {
        m_isDead = false;
        m_detectcount=0;  
    }
    
    return 0;
}

//Get Handle
ACE_HANDLE CPortalConnector::get_handle (void) const
{
    return m_handler;
}

//Close Handle
int CPortalConnector::handle_close (ACE_HANDLE handle,
                        ACE_Reactor_Mask close_mask)
{
    return 0;
}

//Send Data
int CPortalConnector::SendData(const char *data, size_t datasize)
{
    ACE_TCHAR remote_str[80]={0};

    m_peeraddr.addr_to_string (remote_str, 80);

    ssize_t result=ACE_OS::sendto (get_handle(),
                         (char const *)data,
                         datasize,
                         0,
                         (sockaddr const *) m_peeraddr.get_addr (),
                         m_peeraddr.get_size ());

    ACE_DEBUG ((LM_DEBUG,
            ACE_TEXT ("(%P|%t)  CPortalConnector::SendData ")
            ACE_TEXT ("to %s,fd=%d,datasize=%d,result=%d\n"),
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

//Portal Serialize Header
void CPortalConnector::PortalSerializeHeader(openportal_header *pHeader, 
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

//Start Detect
void CPortalConnector::StartDetect(int second)
{
    if (second>0)
    {
        ACE_Time_Value interval(second);
        ACE_Reactor::instance()->schedule_timer(this, 0, interval, interval);
        m_isDetectedEnable = true;
    }
}

//StopDetect
void CPortalConnector::StopDetect()
{
    ACE_Reactor::instance()->cancel_timer(this);
    m_isDetectedEnable=false;
}

//Check channel status
bool CPortalConnector::IsChannelAlive()
{
    return !m_isDead;
}

//Send Detect
int CPortalConnector::SendDetect()
{
    char packet[MAXIMUM_PORTAL_RX_PACKET_SIZE]={0};
    m_last_detect_req_id++;
    size_t length=0;
    openportal_header *pHeader = (openportal_header *)packet;
    PortalSerializeHeader(pHeader, 
        PORTAL_NTF_LOGOUT, 
        PORTAL_CHAP_AUTH, 
        m_cfg.version,0,m_last_detect_req_id, 0, 0x00);

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
        ACE_DEBUG ((LM_ERROR,"(%P|%t) CPortalConnector::SendDetect error\n"));
        return -1;
    }   
    return 0;
}

//Add Authentication
void CPortalConnector::AddAuthentication(openportal_header *phead)
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
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalConnector::AddAuthentication begin\n"));

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
        ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalConnector::AddAuthentication attrib_num=%d\n",attrib_num));
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
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CPortalConnector::AddAuthentication end\n"));

    return;

}

//Get Server Id
uint8_t  CPortalConnector::GetServerId()
{
    return m_cfg.server_id;
}

//Get Server Ip
uint32_t  CPortalConnector::GetServerIP()
{
    return m_cfg.ip_address;
}

//Get Server Port
uint16_t  CPortalConnector::GetServerPort()
{
    return m_cfg.port;
}


