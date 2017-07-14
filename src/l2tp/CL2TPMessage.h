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


#ifndef CL2TPMESSAGE_H
#define CL2TPMESSAGE_H
#include "IL2TPInterface.h"
#include <stdint.h>
#include <unordered_map>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/utsname.h>


class CL2TPMessage 
{
public:
    CL2TPMessage(){}
    virtual ~CL2TPMessage(){}
    virtual int Decode(char *buffer, size_t buffersize, const ACE_INET_Addr &peeraddr) = 0;
};

class CL2TPControllMessage: public CL2TPMessage
{
public:
    CL2TPControllMessage();
    CL2TPControllMessage(char *buffer, size_t buffersize);
    virtual ~CL2TPControllMessage();
    bool IsZLB() const;
    virtual int Decode(char *buffer, size_t buffersize,const ACE_INET_Addr &peeraddr);
    int HandleAVPs(char *buffer, size_t buffersize);
    int message_type_avp (void *data,int datalen);
    int result_code_avp (void *data,int datalen);
    int protocol_version_avp (void *data,int datalen);
    int framing_caps_avp (void *data,int datalen);
    int bearer_caps_avp (void *data,int datalen);
    int firmware_rev_avp (void *data,int datalen);
    int hostname_avp (void *data,int datalen);
    int vendor_avp (void *data,int datalen);
    int assigned_tunnel_avp (void *data,int datalen);
    int receive_window_size_avp (void *data,int datalen);
    int challenge_avp (void *data,int datalen);
    int chalresp_avp (void *data,int datalen);
    int assigned_call_avp (void *data,int datalen);
    int call_serno_avp (void *data,int datalen);
    int bearer_type_avp (void *data,int datalen);
    int frame_type_avp (void *data,int datalen);
    int packet_delay_avp (void *data,int datalen);
    int dialed_number_avp (void *data,int datalen);
    int dialing_number_avp (void *data,int datalen);
    int sub_address_avp (void *data,int datalen);
    int tx_speed_avp (void *data,int datalen);
    int call_physchan_avp (void *data,int datalen);
    int ignore_avp (void *data,int datalen);
    int rand_vector_avp (void *data,int datalen);
    int rx_speed_avp (void *data,int datalen);
    int seq_reqd_avp (void *data,int datalen);
    int init_lcp_req_avp (void *data,int datalen);
    int last_lcp_send_avp (void *data,int datalen);
    int last_lcp_recv_avp (void *data,int datalen);
    int proxy_auth_type_avp (void *data,int datalen);
    int proxy_auth_name_avp (void *data,int datalen);
    int proxy_auth_challenge_avp (void *data,int datalen);
    int proxy_auth_id_avp (void *data,int datalen);
    int proxy_auth_response_avp (void *data,int datalen);

public:
    void add_nonmandatory_header(_u16 length, _u16 type);
    void add_header(_u16 length, _u16 type);

    size_t add_control_hdr (uint16_t tid,
        uint16_t cid,
        uint16_t ns,
        uint16_t nr);
    
    int add_challenge_avp (unsigned char *, int);
    int add_avp_rws (_u16);
    int add_tunnelid_avp (_u16);
    int add_vendor_avp (const char *);
    int add_hostname_avp (const char *);
    int add_firmware_avp (_u16 version);
    int add_bearer_caps_avp (_u16 caps);
    int add_frame_caps_avp (_u16 caps);
    int add_protocol_avp (_u16 protocol);
    int add_message_type_avp (_u16 type);
    int add_result_code_avp (_u16, _u16, char *, int);
    int add_bearer_avp (int);
    int add_frame_avp (int);
    int add_rxspeed_avp (int);
    int add_txspeed_avp (int);
    int add_serno_avp (unsigned int);
    int add_callid_avp (_u16);
    int add_ppd_avp ( _u16);
    int add_seqreqd_avp ();
    int add_chalresp_avp (unsigned char *, int);
    int add_randvect_avp (unsigned char *, int);
    int add_minbps_avp (int speed);      /* jz: needed for outgoing call */
    int add_maxbps_avp (int speed);      /* jz: needed for outgoing call */
    int add_number_avp (char *no);       /* jz: needed for outgoing call */
    int add_physchan_avp (unsigned int physchan);

    int add_init_lcp_req_avp (unsigned char *c,int len);
    int add_last_lcp_send_avp (unsigned char *c,int len);
    int add_last_lcp_recv_avp (unsigned char *c,int len);
    int add_proxy_auth_type_avp (_u16 type);
    int add_proxy_auth_name_avp (unsigned char *c,int len);
    int add_proxy_auth_challenge_avp (unsigned char *c,int len);
    int add_proxy_auth_id_avp (_u16 type);
    int add_proxy_auth_response_avp (unsigned char *c,int len);

    
    _u16 get_message_type();
public:
    
    inline _u16 GetVersion() const {return m_ver;}
    inline _u16 GetLength() const {return m_length;}
    inline _u16 GetTID() const {return m_tid;}
    inline _u16 GetCID() const {return m_cid;}
    inline _u16 GetNS() const {return m_Ns;}
    inline _u16 GetNR() const {return m_Nr;}
    inline char* GetBuffer() const {return m_buffer;}
    inline size_t GetBufferSize() const {return m_buffersize;}
    inline int GetMode() const {return m_encodemode;}

    inline _u16 GetMessageType() const {return m_messatetype;}
    inline _u16 GetAssignedTID() const {return m_assigtunenid;}
    inline _u16 GetAssignedCID() const {return m_assigcallid;}  
    inline ACE_INET_Addr GetPeerAddr() const {return m_peeraddr;}
    void Reset();

    void Dump() const;
public:    
//private:
    _u16 m_ver;                   /* Version and more */
    _u16 m_length;                /* Length field */
    _u16 m_tid;                   /* Tunnel ID */
    _u16 m_cid;                   /* Call ID */
    _u16 m_Ns;                    /* Next sent */
    _u16 m_Nr;                    /* Next received */
    _u16 m_messatetype;
    _u16 m_assigtunenid;
    _u16 m_assigcallid;
    char *m_start;
    char *m_buffer;
    size_t m_buffersize;
    size_t m_currentsize;
    int m_encodemode;
    ACE_INET_Addr m_peeraddr;
    bool m_bzlb;
    L2tpSessionProxy m_proxy;
};


class CL2TPDataMessage : public CL2TPMessage
{
public:
    CL2TPDataMessage();
    CL2TPDataMessage(char *buffer, size_t buffersize);

    virtual ~CL2TPDataMessage();
    virtual int Decode(char *buffer, size_t buffersize, const ACE_INET_Addr &peeraddr);
    size_t AddPayloadHdr(int fbit,
                            int lbit,
                            uint16_t tid, 
                            uint16_t cid,
                            uint16_t ns,
                            uint16_t nr,
                            const char *data, 
                            size_t datasize);

    void Reset();

    inline _u16 GetVersion() const {return m_ver;}
    inline _u16 GetLength() const {return m_length;}
    inline _u16 GetTID() const {return m_tid;}
    inline _u16 GetCID() const {return m_cid;}
    inline int GetNS() const {return m_Ns;}
    inline int GetNR() const {return m_Nr;}
    inline _u16 GetBLF() const {return m_blf;}
    inline _u16 GetBNSF() const {return m_bnsf;}
    inline char *GetBuffer() const {return m_buffer;}
    inline size_t GetBufferSize() const {return m_buffersize;}
    inline char* GetPayload() const {return m_payload;}
    inline size_t GetPayloadSize() const {return m_payloadsize;}
    inline int GetMode() const {return m_encodemode;}
    inline ACE_INET_Addr GetPeerAddr() const {return m_peeraddr;}

public:
    _u16 m_ver;                   /* Version and more */
    _u16 m_length;                /* Length field */
    _u16 m_tid;                   /* Tunnel ID */
    _u16 m_cid;                   /* Call ID */
    _u16 m_Ns;                    /* Next sent */
    _u16 m_Nr;                    /* Next received */
    int m_blf;
    int m_bnsf;
    char *m_buffer;
    size_t m_buffersize;
    char *m_payload;
    size_t m_payloadsize;
    int m_encodemode;
    ACE_INET_Addr m_peeraddr;
};

#endif//CL2TPMESSAGE_H

