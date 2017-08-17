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


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include "aceinclude.h"
#include "DHCPPacket.h"

unsigned char DHCP_MAGIC_COOKIE[4] = {0x63, 0x82, 0x53, 0x63};

//Caller need to free the memory used for the DHCP packet 
struct dhcp_packet *marshall(const char *buffer, int offset, int length)
{
    struct dhcp_packet *packet = NULL;
    
    //first check if the arguments are valid
    if (NULL == buffer)
    {
        ACE_DEBUG ((LM_ERROR, "marshall, argument buffer is NULL.\n"));
        
        if (NULL != packet)
        {
            free_packet(packet);
        }
        
        return NULL;
    }

    if (length < BOOTP_ABSOLUTE_MIN_LEN)
    {
        ACE_DEBUG ((LM_ERROR, "marshall, length too short. length=%d\n", length));
        
        if (NULL != packet)
        {
            free_packet(packet);
        }
        
        return NULL;
    }

    if (length > DHCP_MAX_MTU)
    {
        ACE_DEBUG ((LM_ERROR, "marshall, length too long. length = %d\n", length));
        
        if (NULL != packet)
        {
            free_packet(packet);
        }
        return NULL;
    }

    packet = (struct dhcp_packet*)malloc(sizeof(struct dhcp_packet));
    if (NULL == packet)
    {
        ACE_DEBUG ((LM_ERROR, "marshall, failed to malloc dhcp_packet.\n"));
        return NULL;
    }

    memset(packet, 0, sizeof(struct dhcp_packet));

    char* packet_begin = (char*)buffer + offset; 
    //parse static part of packet
    memcpy(&(packet->op), packet_begin, 1);
    memcpy(&(packet->htype), packet_begin + 1, 1);
    memcpy(&(packet->hlen), packet_begin + 2, 1);
    memcpy(&(packet->hops), packet_begin + offset + 3, 1);
    memcpy(packet->xid, packet_begin + 4, 4);
    memcpy(packet->secs, packet_begin + 8, 2);
    memcpy(packet->flags, packet_begin + 10, 2);
    memcpy(packet->ciaddr, packet_begin + 12, 4);
    memcpy(packet->yiaddr, packet_begin + 16, 4);
    memcpy(packet->siaddr, packet_begin + 20, 4);
    memcpy(packet->giaddr, packet_begin + 24, 4);
    memcpy(packet->chaddr, packet_begin + 28, 16);
    memcpy(packet->sname, packet_begin + 44, 64);
    memcpy(packet->file, packet_begin + 108, 128);

    //check DHCP magic cookie
    char magic[4];
    memcpy(magic, packet_begin + 236, 4);
    if (0 != memcmp(DHCP_MAGIC_COOKIE, magic, 4))
    {
        ACE_DEBUG ((LM_ERROR, "marshall, magic cookie is not DHCP.\n"));
        
        if (NULL != packet)
        {
            free_packet(packet);
        }
        
        return NULL;
    }

    //parse options
    int options_offset = 240; //236 + 4
    packet->options = NULL;
    struct dhcp_option *prev = NULL;
    while (1)
    { 
        if (options_offset > length - 1)
        {
            break;
        }

        //code
        char code = 0;
        memcpy(&code, packet_begin + options_offset, 1);
        options_offset++;

        if (DHO_PAD == code)
        {
            continue;
        }

        if (DHO_END == code)
        {
            break;
        }

        //length
        int len = 0;
        char len_buff = 0;
        memcpy(&len_buff, packet_begin + options_offset, 1);
        len = (int)len_buff;
        options_offset++;

        if (options_offset + len > length - 1)
        {
            break;
        }

        //value
        struct dhcp_option * option = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
        if (NULL == option)
        {
            ACE_DEBUG ((LM_ERROR, "marshall, failed to malloc option.\n"));
            
            if (NULL != packet)
            {
                free_packet(packet);
            }
            
            return NULL;
        }
        memset(option, 0, sizeof(struct dhcp_option));

        option->code = code;
        option->length = len_buff;
        option->value = (char*)malloc(len);
        if (NULL == option->value)
        {
            ACE_DEBUG ((LM_ERROR, "marshall, failed to malloc option->value. len = %d\n", len));
                
            if (NULL != packet)
            {
                free_packet(packet);
            }
            
            return NULL;
        }
        memcpy(option->value, buffer + options_offset, len);
        option->next = NULL;	
        options_offset += len;

        //Add the option into the packet
        if (NULL == packet->options)
        {
            packet->options = option;
        }
        
        if (NULL != prev)
        {
            prev->next = option;
        }
        prev = option;
    }

    if (options_offset < length - 1)
    {
        packet->padding = (char*)malloc(length - options_offset);
        if (NULL == packet->padding)
        {
            ACE_DEBUG ((LM_ERROR, 
                                "marshall, Allocate memory(%d bytes) for packet->padding failed! %s(%d).\n", 
                                length - options_offset,
                                strerror(errno), 
                                errno));
        }
        else
        {
            memcpy(packet->padding, buffer + options_offset, length - options_offset - 1);
        }
    }
    else
    {
        packet->padding = NULL;
    }

    return packet;
}

//Use this function to free dhcp packet
void free_packet(struct dhcp_packet *packet)
{
    if (NULL == packet)
    {
        return;
    }

    if (NULL != packet->padding)
    {
        free(packet->padding);
    }

    struct dhcp_option *option = packet->options;
    while (NULL != option)
    {
        if (NULL != option->value)
        {
            free(option->value);
        }
        struct dhcp_option *current = option; 
        option = current->next;

        free(current);
    }

    free(packet);

    return;
}

//Get packet length
int serialize(struct dhcp_packet *packet, char buffer[], int length)
{
    if (NULL == packet)
    {
        ACE_DEBUG ((LM_ERROR, "serialize, NULL packet.\n"));
        return 0;
    }

    //calculate the total size of the packet
    //static part
    int packet_len = BOOTP_ABSOLUTE_MIN_LEN;
    //magic cookie
    packet_len += sizeof(DHCP_MAGIC_COOKIE);
    //options
    struct dhcp_option *option = packet->options;
    while (NULL != option)
    {
        packet_len += 2;
        packet_len += (int)option->length;
        option = option->next;
    }
    //end option
    packet_len++;

    //calculate padding length
    int padding_len = 0;
    if (packet_len < BOOTP_ABSOLUTE_MIN_LEN + DHCP_VEND_SIZE)
    {
        padding_len = DHCP_VEND_SIZE + BOOTP_ABSOLUTE_MIN_LEN - packet_len;
        packet_len = DHCP_VEND_SIZE + BOOTP_ABSOLUTE_MIN_LEN;
    }

    if (packet_len > length)
    {
        ACE_DEBUG ((LM_ERROR, "serialize, packet_len > length. packet_len = %d, length = %d\n", packet_len, length));
        return 0;
    }

    memcpy(buffer, &(packet->op), 1);
    memcpy(buffer + 1, &(packet->htype), 1);
    memcpy(buffer + 2, &(packet->hlen), 1);
    memcpy(buffer + 3, &(packet->hops), 1);
    memcpy(buffer + 4, packet->xid, 4);
    memcpy(buffer + 8, packet->secs, 2);
    memcpy(buffer + 10, packet->flags, 2);
    memcpy(buffer + 12, packet->ciaddr, 4);
    memcpy(buffer + 16, packet->yiaddr, 4);
    memcpy(buffer + 20, packet->siaddr, 4);
    memcpy(buffer + 24, packet->giaddr, 4);
    memcpy(buffer + 28, packet->chaddr, 16);
    memcpy(buffer + 44, packet->sname, 64);
    memcpy(buffer + 108, packet->file, 128);

    memcpy(buffer + 236, DHCP_MAGIC_COOKIE, 4);

    int options_offset = 240;
    option = packet->options;
    while (NULL != option)
    {
        memcpy(buffer + options_offset, &(option->code), 1);
        options_offset++;
        memcpy(buffer + options_offset, &(option->length), 1);
        options_offset++;

        int len = (int)option->length;
        memcpy(buffer + options_offset, option->value, len);
        options_offset += len;

        option = option->next;
    }

    char dhcp_option_end = DHO_END;
    memcpy(buffer + options_offset, &dhcp_option_end, 1);
    options_offset++;

    if (padding_len > 0)
    {
        memset(buffer + options_offset, 0, padding_len);  
    }

    return packet_len; 
}

void copy_packet(struct dhcp_packet *src, struct dhcp_packet *dst)
{
    
}

//Check the status of the request packet
bool is_broadcast(struct dhcp_packet *request)
{
    if (request == NULL)
    {
        return false;
    }
    
    uint16_t flag = 0;
    ::memcpy(&flag, request->flags, 2);
    flag = ntohs(flag);
    if ((flag & 0x8000) == 0x8000)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//Get Request Ip
uint32_t get_request_ip(struct dhcp_packet *request)
{
    uint32_t ipaddr = 0;
    if (request->options == NULL)
    {
        return 0;
    }
    struct dhcp_option *option = request->options;
    while (option != NULL)
    {
        if ((option->code == DHO_DHCP_REQUESTED_ADDRESS) && (option->length == 4))
        {
            ::memcpy(&ipaddr,option->value,4);
            break;
        }
        option = option->next;
    }

    return ipaddr;
}

//Get Server Id
uint32_t get_server_id(struct dhcp_packet *request)
{
    uint32_t ipaddr = 0;
    if (request->options == NULL)
    {
        ACE_DEBUG ((LM_INFO, "get_server_id, request->options is NULL.\n"));
        return 0;
    }
    
    struct dhcp_option *option = request->options;
    while (option != NULL)
    {
        if ((option->code == DHO_DHCP_SERVER_IDENTIFIER) && (option->length == 4))
        {
            ::memcpy(&ipaddr,option->value,4);
            break;
        }
        option = option->next;
    }

    return ipaddr;  
}


