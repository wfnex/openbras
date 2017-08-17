/***********************************************************************
 * 
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


#include "CL2TPMessage.h"

#define AVP_MAX 39

typedef int (CL2TPControllMessage::*avpcallType)(void *, int);
struct avp
{
    int num;                    /* Number of AVP */
    int m;                      /* Set M? */
    avpcallType handler;
    const char *description;          /* A name, for debugging */
};


struct avp avps[] = {

    {0, 1, &CL2TPControllMessage::message_type_avp, "Message Type"},
    {1, 1, &CL2TPControllMessage::result_code_avp, "Result Code"},
    {2, 1, &CL2TPControllMessage::protocol_version_avp, "Protocol Version"},
    {3, 1, &CL2TPControllMessage::framing_caps_avp, "Framing Capabilities"},
    {4, 1, &CL2TPControllMessage::bearer_caps_avp, "Bearer Capabilities"},
    {5, 0, NULL, "Tie Breaker"},
    {6, 0, &CL2TPControllMessage::firmware_rev_avp, "Firmware Revision"},
    {7, 0, &CL2TPControllMessage::hostname_avp, "Host Name"},
    {8, 1, &CL2TPControllMessage::vendor_avp, "Vendor Name"},
    {9, 1, &CL2TPControllMessage::assigned_tunnel_avp, "Assigned Tunnel ID"},
    {10, 1, &CL2TPControllMessage::receive_window_size_avp, "Receive Window Size"},
    {11, 1, &CL2TPControllMessage::challenge_avp, "Challenge"},
    {12, 0, NULL, "Q.931 Cause Code"},
    {13, 1, &CL2TPControllMessage::chalresp_avp, "Challenge Response"},
    {14, 1, &CL2TPControllMessage::assigned_call_avp, "Assigned Call ID"},
    {15, 1, &CL2TPControllMessage::call_serno_avp, "Call Serial Number"},
    {16, 1, NULL, "Minimum BPS"},
    {17, 1, NULL, "Maximum BPS"},
    {18, 1, &CL2TPControllMessage::bearer_type_avp, "Bearer Type"},
    {19, 1, &CL2TPControllMessage::frame_type_avp, "Framing Type"},
    {20, 1, &CL2TPControllMessage::packet_delay_avp, "Packet Processing Delay"},
    {21, 1, &CL2TPControllMessage::dialed_number_avp, "Dialed Number"},
    {22, 1, &CL2TPControllMessage::dialing_number_avp, "Dialing Number"},
    {23, 1, &CL2TPControllMessage::sub_address_avp, "Sub-Address"},
    {24, 1, &CL2TPControllMessage::tx_speed_avp, "Transmit Connect Speed"},
    {25, 1, &CL2TPControllMessage::call_physchan_avp, "Physical channel ID"},
    {26, 0, &CL2TPControllMessage::init_lcp_req_avp, "Initial Received LCP Confreq"},
    {27, 0, &CL2TPControllMessage::last_lcp_send_avp, "Last Sent LCP Confreq"},
    {28, 0, &CL2TPControllMessage::last_lcp_recv_avp, "Last Received LCP Confreq"},
    {29, 1, &CL2TPControllMessage::proxy_auth_type_avp, "Proxy Authen Type"},
    {30, 0, &CL2TPControllMessage::proxy_auth_name_avp, "Proxy Authen Name"},
    {31, 0, &CL2TPControllMessage::proxy_auth_challenge_avp, "Proxy Authen Challenge"},
    {32, 0, &CL2TPControllMessage::proxy_auth_id_avp, "Proxy Authen ID"},
    {33, 1, &CL2TPControllMessage::proxy_auth_response_avp, "Proxy Authen Response"},
    {34, 1, NULL, "Call Errors"},
    {35, 1, &CL2TPControllMessage::ignore_avp, "ACCM"},
    {36, 1, &CL2TPControllMessage::rand_vector_avp, "Random Vector"},
    {37, 1, NULL, "Private Group ID"},
    {38, 0, &CL2TPControllMessage::rx_speed_avp, "Receive Connect Speed"},
    {39, 1, &CL2TPControllMessage::seq_reqd_avp, "Sequencing Required"}
};

const char *msgtypes[] = {
    NULL,
    "Start-Control-Connection-Request",
    "Start-Control-Connection-Reply",
    "Start-Control-Connection-Connected",
    "Stop-Control-Connection-Notification",
    NULL,
    "Hello",
    "Outgoing-Call-Request",
    "Outgoing-Call-Reply",
    "Outgoing-Call-Connected",
    "Incoming-Call-Request",
    "Incoming-Call-Reply",
    "Incoming-Call-Connected",
    NULL,
    "Call-Disconnect-Notify",
    "WAN-Error-Notify",
    "Set-Link-Info"
};

const char *stopccn_result_codes[] = {
    "Reserved",
    "General request to clear control connection",
    "General error--Error Code indicates the problem",
    "Control channel already exists",
    "Requester is not authorized to establish a control channel",
    "The protocol version of the requester is not supported--Error Code indicates the highest version supported",
    "Requester is being shut down",
    "Finite State Machine error"
};

const char *cdn_result_codes[] = {
    "Reserved",
    "Call disconnected due to loss of carrier",
    "Call disconnected for the reason indicated in error code",
    "Call disconnected for administrative reasons",
    "Call failed due to lack of appropriate facilities being available (temporary condition)",
    "Call failed due to lack of appropriate facilities being available (permanent condition)",
    "Invalid destination",
    "Call failed due to no carrier detected",
    "Call failed due to lack of a dial tone",
    "Call was no established within time allotted by LAC",
    "Call was connected but no appropriate framing was detect"
};

CL2TPControllMessage::CL2TPControllMessage()
{
    Reset();
}

CL2TPControllMessage::CL2TPControllMessage(char *buffer, size_t buffersize)
{
     Reset();
     m_encodemode = 1;
     m_start = (char *)buffer;
     m_buffersize = buffersize;
     m_buffer = buffer+sizeof(control_hdr);
     m_currentsize = 0;
}


CL2TPControllMessage::~CL2TPControllMessage()
{
}

//Decode addr
int CL2TPControllMessage::Decode(char *buffer, size_t buffersize, const ACE_INET_Addr &peeraddr)
{
    m_peeraddr = peeraddr;
    if (buffersize < sizeof(control_hdr))
    {
        return -1;
    }
    
    struct control_hdr *phdr = (struct control_hdr*)(buffer);
    
    m_ver = ntohs(phdr->ver);
    m_length = ntohs(phdr->length);
    m_tid = ntohs(phdr->tid);
    m_cid = ntohs(phdr->cid);
    m_Ns = ntohs(phdr->Ns);
    m_Nr = ntohs(phdr->Nr);


    if (buffersize == sizeof(control_hdr))
    {
        m_bzlb = true;
        
    }
    else
    {
        m_bzlb = false;
        HandleAVPs(buffer,buffersize);
    }

    return 0;
}

//Reset
void CL2TPControllMessage::Reset()
{
    m_ver=0;
    m_length=0;
    m_tid=0;
    m_cid=0;
    m_Ns=0;
    m_Nr=0;
    m_messatetype = 0;
    m_assigtunenid = 0;
    m_assigcallid = 0;
    m_start = 0;
    m_buffer = 0;
    m_buffersize = 0;
    m_currentsize = 0;
    m_encodemode = -1;
    m_bzlb = false;
    ::memset(&m_proxy, 0, sizeof(m_proxy));
}

//Determine the status of zlb
bool CL2TPControllMessage::IsZLB() const
{
    return m_bzlb;
}

//AVPs handle
int CL2TPControllMessage::HandleAVPs(char *buffer, size_t buffersize)
{
    /*
     * buf's start should point to the beginning of a packet. We assume it's
     * a valid packet and has had check_control done to it, so no error
     * checking is done at this point.
     */
    avpcallType handler=NULL;

    struct avp_hdr *avp;
    int len = buffersize - sizeof (struct control_hdr);
    int firstavp = -1;
    int hidlen = 0;
    char *data = buffer + sizeof (struct control_hdr);
    avp = (struct avp_hdr *) data;
    while (len > 0)
    {
        /* Go ahead and byte-swap the header */
        swaps (avp, sizeof (struct avp_hdr));
        if (avp->attr > AVP_MAX)
        {
            if (AMBIT (avp->length))
            {
                return -1;
            }
            else
            {
                goto next;
            }
        }
        if (ALENGTH (avp->length) > len)
        {
            return -1;
        }
        if (avp->attr && firstavp)
        {
            return -1;
        }
        if (ALENGTH (avp->length) < sizeof (struct avp_hdr))
        {
            return -1;
        }
        if (AZBITS (avp->length))
        {
            if (AMBIT (avp->length))
            {
                return -1;
            }
            goto next;
        }

        hidlen = 0;
        handler = avps[avp->attr].handler;
        if (handler)
        {
            if ((this->*handler)(avp, ALENGTH (avp->length)))
            {
                if (AMBIT (avp->length))
                {
                    return -1;
                }
            }
        }
        else
        {
            if (AMBIT (avp->length))
            {
                return -1;
            }
        }
      next:
        if (hidlen)
        {
            /* Skip over the complete length of the hidden AVP */
            len -= ALENGTH (hidlen);
            data += ALENGTH (hidlen);
        }
        else
        {
            len -= ALENGTH (avp->length);
            data += ALENGTH (avp->length);      /* Next AVP, please */
        }
        avp = (struct avp_hdr *) data;
        firstavp = 0;
    }
    if (len != 0)
    {
        return -1;
    }
    return 0;
}

//messagetype avp
int CL2TPControllMessage::message_type_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::message_type_avp datalen=%d\n",datalen)); 
    struct unaligned_u16 *raw = (struct unaligned_u16*)data;
    m_messatetype = ntohs (raw[3].s);
    return 0;
}

//result code avp
int CL2TPControllMessage::result_code_avp (void *data,int datalen)
{
    ACE_UNUSED_ARG(data);

    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::result_code_avp datalen=%d\n",datalen)); 
    return 0;
}

//protocol version avp
int CL2TPControllMessage::protocol_version_avp (void *data,int datalen)
{
    ACE_UNUSED_ARG(data);

    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::protocol_version_avp datalen=%d\n",datalen)); 
    return 0;
}

//framing caps avp
int CL2TPControllMessage::framing_caps_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::framing_caps_avp datalen=%d\n",datalen)); 
    ACE_UNUSED_ARG(data);

    return 0;
}

//bearer caps avp
int CL2TPControllMessage::bearer_caps_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::bearer_caps_avp datalen=%d\n",datalen)); 
    ACE_UNUSED_ARG(data);

    return 0;
}

//firmware rev avp
int CL2TPControllMessage::firmware_rev_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::firmware_rev_avp datalen=%d\n",datalen)); 
    ACE_UNUSED_ARG(data);

    return 0;
}

//hostname avp
int CL2TPControllMessage::hostname_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::hostname_avp datalen=%d\n",datalen)); 
    ACE_UNUSED_ARG(data);

    return 0;
}

//vendor avp
int CL2TPControllMessage::vendor_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::vendor_avp datalen=%d\n",datalen)); 
    ACE_UNUSED_ARG(data);

    return 0;
}

//assigned tunnel avp
int CL2TPControllMessage::assigned_tunnel_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::assigned_tunnel_avp datalen=%d\n",datalen)); 
    /*
     * What is their TID that we must use from now on?
     */
    struct unaligned_u16 *raw = (struct unaligned_u16 *)data;
    
    m_assigtunenid = ntohs (raw[3].s);

    return 0;
}

//receive window size avp
int CL2TPControllMessage::receive_window_size_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::receive_window_size_avp datalen=%d\n",datalen));
    ACE_UNUSED_ARG(data);

    return 0;
}

//challenge avp
int CL2TPControllMessage::challenge_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::challenge_avp datalen=%d\n",datalen));
    ACE_UNUSED_ARG(data);

    return 0;
}

//chalresp avp
int CL2TPControllMessage::chalresp_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::chalresp_avp datalen=%d\n",datalen));
    ACE_UNUSED_ARG(data);

    return 0;
}

//assigned call avp
int CL2TPControllMessage::assigned_call_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::assigned_call_avp datalen=%d\n",datalen));
    /*
     * What is their CID that we must use from now on?
     */
    struct unaligned_u16 *raw = (struct unaligned_u16*)data;
    m_assigcallid=ntohs (raw[3].s);

    return 0;
}

//init lcp req avp
int CL2TPControllMessage::init_lcp_req_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::init_lcp_req_avp datalen=%d\n",datalen));
    SETS_IRLCPREQ(&m_proxy,data,datalen);

    return 0;
}

//last lcp send avp
int CL2TPControllMessage::last_lcp_send_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::last_lcp_send_avp datalen=%d\n",datalen));
    SETS_LSLCPREQ(&m_proxy,data,datalen);
    return 0;
}

//last lcp recv avp
int CL2TPControllMessage::last_lcp_recv_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::last_lcp_recv_avp datalen=%d\n",datalen));
    SETS_LRLCPREQ(&m_proxy,data,datalen);
    return 0;
}

//proxy auth type avp
int CL2TPControllMessage::proxy_auth_type_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::proxy_auth_type_avp datalen=%d\n",datalen));
    struct unaligned_u16 *raw = (struct unaligned_u16*)data;
    SETS_AUTHENTYPE(&m_proxy,ntohs (raw[3].s));

    return 0;
}

//proxy auth name avp
int CL2TPControllMessage::proxy_auth_name_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::proxy_auth_name_avp datalen=%d\n",datalen));
    SETS_AUTHENNAME(&m_proxy,data,datalen);

    return 0;
}

//proxy auth challenge avp
int CL2TPControllMessage::proxy_auth_challenge_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::proxy_auth_challenge_avp datalen=%d\n",datalen));
    SETS_AUTHENCHAL(&m_proxy,data,datalen);
    return 0;
}

//proxy auth id avp
int CL2TPControllMessage::proxy_auth_id_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::proxy_auth_id_avp datalen=%d\n",datalen));

    struct unaligned_u16 *raw = (struct unaligned_u16*)data;
    //m_assigcallid=ntohs (raw[3].s);
    SETS_AUTHENID(&m_proxy,ntohs (raw[3].s));

    return 0;
}

//proxy auth response avp
int CL2TPControllMessage::proxy_auth_response_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::proxy_auth_response_avp datalen=%d\n",datalen));
    SETS_AUTHENRESP(&m_proxy,data,datalen);

    return 0;
}


//call serno avp
int CL2TPControllMessage::call_serno_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::call_serno_avp datalen=%d\n",datalen));
    ACE_UNUSED_ARG(data);

    return 0;
}

//bearer type avp
int CL2TPControllMessage::bearer_type_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::bearer_type_avp datalen=%d\n",datalen));
    ACE_UNUSED_ARG(data);

    return 0;
}

//frame type avp
int CL2TPControllMessage::frame_type_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::frame_type_avp datalen=%d\n",datalen));
    ACE_UNUSED_ARG(data);

    return 0;
}

//packet delay avp
int CL2TPControllMessage::packet_delay_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::packet_delay_avp datalen=%d\n",datalen));
    ACE_UNUSED_ARG(data);

    return 0;
}

//dialed number avp 
int CL2TPControllMessage::dialed_number_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::dialed_number_avp datalen=%d\n",datalen));
    ACE_UNUSED_ARG(data);

    return 0;
}

//dialing number avp
int CL2TPControllMessage::dialing_number_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::dialing_number_avp datalen=%d\n",datalen));
    ACE_UNUSED_ARG(data);

    return 0;
}

//sub address avp
int CL2TPControllMessage::sub_address_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::sub_address_avp datalen=%d\n",datalen));
    ACE_UNUSED_ARG(data);

    return 0;
}

//tx speed avp
int CL2TPControllMessage::tx_speed_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::tx_speed_avp datalen=%d\n",datalen));
    ACE_UNUSED_ARG(data);

    return 0;
}

//call physchan avp
int CL2TPControllMessage::call_physchan_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::call_physchan_avp datalen=%d\n",datalen));
    ACE_UNUSED_ARG(data);

    return 0;
}
//ignore avp
int CL2TPControllMessage::ignore_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::ignore_avp datalen=%d\n",datalen));
    ACE_UNUSED_ARG(data);

    return 0;
}

//rand vector avp
int CL2TPControllMessage::rand_vector_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::rand_vector_avp datalen=%d\n",datalen));
    ACE_UNUSED_ARG(data);

    return 0;
}

//rx speed avp
int CL2TPControllMessage::rx_speed_avp (void *data,int datalen)
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::rx_speed_avp datalen=%d\n",datalen));
    ACE_UNUSED_ARG(data);

    return 0;
}

//seq reqd avp
int CL2TPControllMessage::seq_reqd_avp (void *data,int datalen)
{
    ACE_UNUSED_ARG(data);
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) CL2TPControllMessage::seq_reqd_avp datalen=%d\n",datalen));
    return 0;
}


struct half_words {
    _u16 s0;
    _u16 s1;
    _u16 s2;
    _u16 s3;
} __attribute__ ((packed));

//add nonmandatory header
void CL2TPControllMessage::add_nonmandatory_header(_u16 length, _u16 type) {
    struct avp_hdr *avp = (struct avp_hdr *) (m_buffer + m_currentsize);
    avp->length = htons (length);
    avp->vendorid = htons (VENDOR_ID);
    avp->attr = htons (type);
}

//add header
void CL2TPControllMessage::add_header(_u16 length, _u16 type) {
    add_nonmandatory_header(length|MBIT, type);
}

/* 
 * These routines should add avp's to a buffer
 * to be sent
 */

/* FIXME:  If SANITY is on, we should check for buffer overruns */

int CL2TPControllMessage::add_message_type_avp (_u16 type)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));
    add_header(0x8, 0);
    ptr->s0 = htons(type);
    m_currentsize += 0x8;
    return 0;
}

int CL2TPControllMessage::add_protocol_avp (_u16 protocol)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));
    add_header(0x8, 0x2);        /* Length and M bit */
    ptr->s0 = htons (protocol);
    m_currentsize += 0x8;
    return 0;
}

int CL2TPControllMessage::add_frame_caps_avp (_u16 caps)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));
    add_header(0xA, 0x3);
    ptr->s0 = 0;
    ptr->s1 = htons (caps);
    m_currentsize += 0xA;
    return 0;
}

int CL2TPControllMessage::add_bearer_caps_avp ( _u16 caps)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));
    add_header(0xA, 0x4);
    ptr->s0 = 0;
    ptr->s1 = htons (caps);
    m_currentsize += 0xA;
    return 0;
}

/* FIXME: I need to send tie breaker AVP's */

int CL2TPControllMessage::add_firmware_avp (_u16 version)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));
    add_nonmandatory_header(0x8, 0x6);
    ptr->s0 = htons (version);
    m_currentsize += 0x8;
    return 0;
}

int CL2TPControllMessage::add_hostname_avp (const char *hostname)
{
    size_t namelen = strlen(hostname);
    if (namelen > MAXAVPSIZE - 6) {
        namelen = MAXAVPSIZE - 6;
    }
    add_header(0x6 + namelen, 0x7);
    strncpy ((char *) (m_buffer + m_currentsize + sizeof(struct avp_hdr)),
        hostname, namelen);
    m_currentsize += 0x6 + namelen;
    return 0;
}

int CL2TPControllMessage::add_vendor_avp (const char *vendorname)
{
    add_nonmandatory_header(0x6 + strlen (vendorname), 0x8);
    strcpy ((char *) (m_buffer + m_currentsize + sizeof(struct avp_hdr)), vendorname);
    m_currentsize += 0x6 + strlen (vendorname);
    return 0;
}

int CL2TPControllMessage::add_tunnelid_avp (_u16 tid)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));
    add_header(0x8, 0x9);
    ptr->s0 = htons (tid);
    m_currentsize += 0x8;
    return 0;
}

int CL2TPControllMessage::add_avp_rws (_u16 rws)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));
    add_header(0x8, 0xA);
    ptr->s0 = htons (rws);
    m_currentsize += 0x8;
    return 0;
}

int CL2TPControllMessage::add_challenge_avp (unsigned char *c, int len)
{
    add_header((0x6 + len), 0xB);
    memcpy((char *) (m_buffer + m_currentsize + sizeof(struct avp_hdr)), c, len);
    m_currentsize += 0x6 + len;
    return 0;
}

int CL2TPControllMessage::add_chalresp_avp (unsigned char *c, int len)
{
    add_header((0x6 + len), 0xD);
    memcpy((char *) (m_buffer + m_currentsize + sizeof(struct avp_hdr)), c, len);
    m_currentsize += 0x6 + len;
    return 0;
}

int CL2TPControllMessage::add_randvect_avp (unsigned char *c, int len)
{
    add_header((0x6 + len), 0x24);
    memcpy((char *) (m_buffer + m_currentsize + sizeof(struct avp_hdr)), c, len);
    m_currentsize += 0x6 + len;
    return 0;
}

int CL2TPControllMessage::add_result_code_avp (_u16 result, _u16 error,
                         char *msg, int len)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));
    add_header((0xA + len), 0x1);
    ptr->s0 = htons (result);
    ptr->s1 = htons (error);
    memcpy ((char *) &ptr->s2, msg, len);
    m_currentsize += 0xA + len;
    return 0;
}

int CL2TPControllMessage::add_callid_avp (_u16 callid)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));

    add_header(0x8, 0xE);
    ptr->s0 = htons (callid);
    m_currentsize += 0x8;
    return 0;
}

int CL2TPControllMessage::add_serno_avp (unsigned int serno)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));
    add_header(0xA, 0xF);
    ptr->s0 = htons ((serno >> 16) & 0xFFFF);
    ptr->s1 = htons (serno & 0xFFFF);
    m_currentsize += 0xA;
    return 0;
}

int CL2TPControllMessage::add_bearer_avp (int bearer)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));
    add_header(0xA, 0x12);
    ptr->s0 = htons ((bearer >> 16) & 0xFFFF);
    ptr->s1 = htons (bearer & 0xFFFF);
    m_currentsize += 0xA;
    return 0;
}

int CL2TPControllMessage::add_frame_avp (int frame)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));
    add_header(0xA, 0x13);
    ptr->s0 = htons ((frame >> 16) & 0xFFFF);
    ptr->s1 = htons (frame & 0xFFFF);
    m_currentsize += 0xA;
    return 0;
}

int CL2TPControllMessage::add_txspeed_avp (int speed)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));
    add_header(0xA, 0x18);
    ptr->s0 = htons ((speed >> 16) & 0xFFFF);
    ptr->s1 = htons (speed & 0xFFFF);
    m_currentsize += 0xA;
    return 0;
}

int CL2TPControllMessage::add_rxspeed_avp (int speed)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));
    add_nonmandatory_header(0xA, 0x26);
    ptr->s0 = htons ((speed >> 16) & 0xFFFF);
    ptr->s1 = htons (speed & 0xFFFF);
    m_currentsize += 0xA;
    return 0;
}

int CL2TPControllMessage::add_physchan_avp (unsigned int physchan)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));
    add_nonmandatory_header(0xA, 0x19);
    ptr->s0 = htons ((physchan >> 16) & 0xFFFF);
    ptr->s1 = htons (physchan & 0xFFFF);
    m_currentsize += 0xA;
    return 0;
}

int CL2TPControllMessage::add_ppd_avp (_u16 ppd)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));
    add_header(0x8, 0x14);
    ptr->s0 = htons (ppd);
    m_currentsize += 0x8;
    return 0;
}

int CL2TPControllMessage::add_seqreqd_avp ()
{
    add_header(0x6, 0x27);
    m_currentsize += 0x6;
    return 0;
}

/* jz: options dor the outgoing call */

/* jz: Minimum BPS - 16 */
int CL2TPControllMessage::add_minbps_avp (int speed)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));
    add_header(0xA, 0x10);
    ptr->s0 = htons ((speed >> 16) & 0xFFFF);
    ptr->s1 = htons (speed & 0xFFFF);
    m_currentsize += 0xA;
    return 0;
}

/* jz: Maximum BPS - 17 */
int CL2TPControllMessage::add_maxbps_avp (int speed)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));
    add_header(0xA, 0x11);
    ptr->s0 = htons ((speed >> 16) & 0xFFFF);
    ptr->s1 = htons (speed & 0xFFFF);
    m_currentsize += 0xA;
    return 0;
}

/* jz: Dialed Number 21 */
int CL2TPControllMessage::add_number_avp (char *no)
{
    add_header((0x6 + strlen (no)), 0x15);
    strncpy ((char *) (m_buffer + m_currentsize + sizeof(struct avp_hdr)), no, strlen (no));
    m_currentsize += 0x6 + strlen (no);
    return 0;
}

int CL2TPControllMessage::add_init_lcp_req_avp (unsigned char *c,int len)
{
    add_header((0x6 + len), 26);
    memcpy((char *) (m_buffer + m_currentsize + sizeof(struct avp_hdr)), c, len);
    m_currentsize += 0x6 + len;
    return 0;

}

int CL2TPControllMessage::add_last_lcp_send_avp (unsigned char *c,int len)
{
    add_header((0x6 + len), 27);
    memcpy((char *) (m_buffer + m_currentsize + sizeof(struct avp_hdr)), c, len);
    m_currentsize += 0x6 + len;
    return 0;

}

int CL2TPControllMessage::add_last_lcp_recv_avp (unsigned char *c,int len)
{
    add_header((0x6 + len), 28);
    memcpy((char *) (m_buffer + m_currentsize + sizeof(struct avp_hdr)), c, len);
    m_currentsize += 0x6 + len;
    return 0;

}

int CL2TPControllMessage::add_proxy_auth_type_avp (_u16 type)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));

    add_header(0x8, 29);
    ptr->s0 = htons (type);
    m_currentsize += 0x8;
    return 0;

}

int CL2TPControllMessage::add_proxy_auth_name_avp (unsigned char *c,int len)
{
    add_header((0x6 + len), 30);
    memcpy((char *) (m_buffer + m_currentsize + sizeof(struct avp_hdr)), c, len);
    m_currentsize += 0x6 + len;
    return 0;

}

int CL2TPControllMessage::add_proxy_auth_challenge_avp (unsigned char *c,int len)
{
    add_header((0x6 + len), 31);
    memcpy((char *) (m_buffer + m_currentsize + sizeof(struct avp_hdr)), c, len);
    m_currentsize += 0x6 + len;
    return 0;

}

int CL2TPControllMessage::add_proxy_auth_id_avp (_u16 id)
{
    struct half_words *ptr = (struct half_words *) (m_buffer + m_currentsize + sizeof(struct avp_hdr));

    add_header(0x8, 32);
    ptr->s0 = htons (id);
    m_currentsize += 0x8;
    return 0;

}

int CL2TPControllMessage::add_proxy_auth_response_avp (unsigned char *c,int len)
{
    add_header((0x6 + len), 33);
    memcpy((char *) (m_buffer + m_currentsize + sizeof(struct avp_hdr)), c, len);
    m_currentsize += 0x6 + len;
    return 0;

}

//add control hdr
size_t CL2TPControllMessage::add_control_hdr (uint16_t tid,
        uint16_t cid,
        uint16_t ns,
        uint16_t nr)
{
    struct control_hdr *h;
    //m_buffer -= sizeof (struct control_hdr);
    m_currentsize += sizeof (struct control_hdr);
    h = (struct control_hdr *)m_start;
    h->ver = htons (TBIT | LBIT | FBIT | VER_L2TP);
    h->length = htons ((_u16)m_currentsize);
    h->tid = htons (tid);
    h->cid = htons (cid);
    h->Ns = htons (ns);
    h->Nr = htons (nr);

    return m_currentsize;
}
//Get message type
_u16 CL2TPControllMessage::get_message_type()
{
    return m_messatetype;
}

//Print information
void CL2TPControllMessage::Dump() const
{
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) m_ver=%d\n",m_ver));  
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) m_length=%d\n",m_length)); 
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) m_tid=%d\n",m_tid)); 
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) m_cid=%d\n",m_cid)); 
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) m_Ns=%d\n",m_Ns)); 
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) m_Nr=%d\n",m_Nr)); 
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) m_messatetype=%d\n",m_messatetype)); 
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) m_assigtunenid=%d\n",m_assigtunenid)); 
    ACE_DEBUG ((LM_DEBUG,"(%P|%t) m_assigcallid=%d\n",m_assigcallid));
}



CL2TPDataMessage::CL2TPDataMessage()
{
    Reset();
}

CL2TPDataMessage::CL2TPDataMessage(char *buffer, size_t buffersize)
{
    Reset();
    m_encodemode = 1;
    m_buffer = buffer;
    m_buffersize = buffersize;
}


CL2TPDataMessage::~CL2TPDataMessage()
{
}

//decode 
int CL2TPDataMessage::Decode(char *buffer, size_t buffersize, const ACE_INET_Addr &peeraddr)
{
    m_encodemode = false;
    m_peeraddr = peeraddr;

    m_buffer = buffer;
    m_buffersize = buffersize;
    /*
     * Check if this is a valid payload
     * or not.  Returns 0 on success.
     */

    int ehlen = MIN_PAYLOAD_HDR_LEN;
    
    struct payload_hdr *h = (struct payload_hdr *) (buffer);
    if (buffersize < MIN_PAYLOAD_HDR_LEN)
    {
        /* has to be at least MIN_PAYLOAD_HDR_LEN 
           no matter what.  we'll look more later */
        return -1;
    }

    uint16_t ver = ntohs(h->ver);
    if (PTBIT (ver))
    {
        Reset();
        return -1;
    }
    if (PVER (ver) != VER_L2TP)
    {
        ACE_DEBUG ((LM_DEBUG,"%s: Unknown version received\n",
             __FUNCTION__));
        Reset();

        return -1;
    }

#if 0    
    if ((buffersize < ehlen) && !PLBIT (ver))
    {
        ACE_DEBUG ((LM_DEBUG,"%s payload too small (%d < %d)\n",
             __FUNCTION__, buffersize, ehlen));

        return -1;
    }
#endif
    if ((buffersize != ntohs(h->length)) && PLBIT (ver))
    {
        ACE_DEBUG ((LM_DEBUG,"%s: size mismatch (%d != %d)\n",
             __FUNCTION__, buffersize, h->length));
        Reset();

        return -1;
    }
    
    if (PLBIT (ver))
    {
        ehlen += 2;         /* Should have length information */
        struct unaligned_u16 *raw = (struct unaligned_u16*)(buffer);
        m_length = ntohs (raw[1].s);
        m_blf = 1;
        if (PFBIT (ver))
        {
            ehlen += 4;         /* Should have Ns and Nr too */
            struct unaligned_u16 *raw = (struct unaligned_u16*)(buffer);
            m_tid=ntohs(raw[2].s);
            m_cid=ntohs(raw[3].s);
            m_Ns = ntohs (raw[4].s);
            m_Nr = ntohs (raw[5].s);
            m_bnsf=1;
        }
        else
        {
            m_tid=ntohs(raw[2].s);
            m_cid=ntohs(raw[3].s);
            m_Ns = 0;
            m_Nr = 0;
            m_bnsf=0;
        }
    }
    else
    {
        m_length = 0;
        m_blf = 0;
        if (PFBIT (ver))
        {
            ehlen += 4;         /* Should have Ns and Nr too */
            struct unaligned_u16 *raw = (struct unaligned_u16*)(buffer);
            m_tid=ntohs(raw[1].s);
            m_cid=ntohs(raw[2].s);
            m_Ns = ntohs (raw[3].s);
            m_Nr = ntohs (raw[4].s);
            m_bnsf=1;
        }
        else
        {
            m_Ns = 0;
            m_Nr = 0;
            m_bnsf=0;
        }
    }

    if (PSBIT (ver))
    {
        ehlen += 2;         /* Offset information */
    }

    if (((int)buffersize < ehlen) && !PLBIT (ver))
    {
        ACE_DEBUG ((LM_DEBUG,"%s payload too small (%d < %d)\n",
             __FUNCTION__, buffersize, ehlen));

        Reset();
        return -1;
    }

    m_payload = m_buffer+ehlen;
    m_payloadsize = buffersize-ehlen;

    return 0;
}

//add payload hdr
size_t CL2TPDataMessage::AddPayloadHdr(int fbit,
                                        int lbit,
                                        uint16_t tid, 
                                        uint16_t cid,
                                        uint16_t ns,
                                        uint16_t nr,
                                        const char *data, 
                                        size_t datasize)
{
    size_t hdrsize = 0;
    struct payload_hdr *p = NULL;
    hdrsize += sizeof (struct payload_hdr);
    /* Account for no offset */
    hdrsize -= 2;
    if (!fbit)
    {
        /* Forget about Ns and Nr fields then */
        hdrsize -= 4;
    }
    if (!lbit)
    {
        /* Forget about specifying the length */
        hdrsize -= 2;
    }

    if (m_buffersize <hdrsize+datasize)
    {
        return -1;
    }
    p = (struct payload_hdr *)m_buffer;
/*  p->ver = htons(c->lbit | c->rbit | c->fbit | c->ourfbit | VER_L2TP); */
    p->ver = htons (fbit | lbit | VER_L2TP);
    if (lbit)
    {
        p->length = htons ((_u16)m_buffersize);
    }
    else
    {
        p = (struct payload_hdr *) (((char *) p) - 2);
    }
    p->tid = htons (tid);
    p->cid = htons (cid);
    if (fbit)
    {
        p->Ns = htons (ns);
        p->Nr = htons (nr);
    }

    ::memcpy(m_buffer+hdrsize,data,datasize);

    return hdrsize+datasize;
}

//reset
void CL2TPDataMessage::Reset()
{
    m_ver = 0;
    m_length = 0;
    m_tid = 0;                   /* Tunnel ID */
    m_cid = 0;                   /* Call ID */
    m_Ns = 0;                    /* Next sent */
    m_Nr = 0;                    /* Next received */
    m_blf = 0;
    m_bnsf =0;
    m_buffer = NULL;
    m_buffersize=0;
    m_payload = NULL;
    m_payloadsize = 0;
    m_encodemode = -1;
}


