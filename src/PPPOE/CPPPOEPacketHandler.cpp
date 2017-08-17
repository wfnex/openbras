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


#include "CPPPOEPacketHandler.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <regex.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <linux/filter.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <net/if.h>

CPPPOEPacketHandler::CPPPOEPacketHandler()
    : m_handler(ACE_INVALID_HANDLE)
{
}

CPPPOEPacketHandler::~CPPPOEPacketHandler()
{
    ACE_DEBUG ((LM_DEBUG, "CPPPOEPacketHandler::~CPPPOEPacketHandler\n"));
    
    ACE_DEBUG ((LM_DEBUG, "remove_handler\n"));
    ACE_Reactor::instance()->remove_handler (this, 
                                                                 ACE_Event_Handler::ALL_EVENTS_MASK |
                                                                 ACE_Event_Handler::DONT_CALL);

    if (m_handler != ACE_INVALID_HANDLE)
    {
        ACE_DEBUG ((LM_DEBUG, "close socket m_handler.\n"));
        close(m_handler);
        m_handler = ACE_INVALID_HANDLE;
    }
}

SWORD32 CPPPOEPacketHandler::OpenX (WORD16 type)
{
    SWORD32 optval = 1;
    
    if ((m_handler = socket(AF_PACKET, SOCK_RAW, htons(type))) < 0) 
    {
        /* Give a more helpful message for the common error case */
        if (errno == EPERM)
        {
            ACE_DEBUG ((LM_ERROR,"CPPPOEPacketHandler::OpenX, Cannot create raw socket -- PPPOE must be run as root.\n"));
        }

        ACE_DEBUG ((LM_ERROR,"CPPPOEPacketHandler::OpenX, socket error\n"));
        return -1;
    }

    if (setsockopt(m_handler, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) < 0) 
    {
        ACE_DEBUG ((LM_ERROR,"CPPPOEPacketHandler::OpenX, setsockopt(SO_BROADCAST) error, m_handler=%d\n",m_handler));
        close(m_handler);
        m_handler = ACE_INVALID_HANDLE;
        return -1;
    }

    ACE_DEBUG ((LM_DEBUG,"CPPPOEPacketHandler::OpenX, register_handler m_handler=%d\n",m_handler));

    // Register with the reactor for input.
    if (ACE_Reactor::instance()->register_handler (this, ACE_Event_Handler::READ_MASK) == -1)
    {
        close(m_handler);
        m_handler = ACE_INVALID_HANDLE;        
        ACE_ERROR_RETURN ((LM_ERROR,
                           ACE_TEXT ("(%P|%t) CPPPOEPacketHandler::OpenX: %p\n"),
                           ACE_TEXT ("register_handler for input")),
                          -1);
    }

    return 0; 
}

//Send Packet
SWORD32 CPPPOEPacketHandler::SendPacket(const CHAR *pkt, SWORD32 size)
{
    ACE_DEBUG ((LM_DEBUG, "CPPPOEPacketHandler::SendPacket, size=%d\n", size));
    
    if (::send(m_handler, pkt, size, 0) < 0 && (errno != ENOBUFS))
    {
        ACE_DEBUG ((LM_ERROR,"CPPPOEPacketHandler::SendPacket error\n"));
        return -1;
    }
    
    return 0;
}

//Input Handle
int CPPPOEPacketHandler::handle_input (ACE_HANDLE fd)
{
    static char buffer[2048] = {0};
    ssize_t size = 0;
    
    ACE_DEBUG ((LM_DEBUG,"CPPPOEPacketHandler::handle_input, m_handler=%d\n",m_handler));
    
    if ((size = ::recv(m_handler, buffer, sizeof(buffer), 0)) < 0) 
    {
        ACE_DEBUG ((LM_ERROR,"CPPPOEPacketHandler::handle_input, recv error\n"));
        return -1;
    }

    if (ProcessPacket(buffer,size) == -1)
    {
        ACE_DEBUG ((LM_ERROR,"PPPOEPacketHandler::handle_input, ProcessPacket error, size=%d\n",size));
    }

    return 0;  
}

//Get Handle
ACE_HANDLE CPPPOEPacketHandler::get_handle (void) const
{
    return m_handler;
}

//Close Handle
int CPPPOEPacketHandler::handle_close (ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
{
    ACE_DEBUG ((LM_ERROR,"CPPPOEPacketHandler::handle_close\n"));

    return 0;
}

//Bind Interface
SWORD32 CPPPOEPacketHandler::BindInterface(CHAR const *ifname, WORD16 type)
{
    struct sockaddr_ll sa;
    struct ifreq ifr;

    memset(&sa, 0, sizeof(sa));  

    ACE_DEBUG ((LM_DEBUG, "CPPPOEPacketHandler::BindInterface, ifname=%s, type=%#x\n", ifname, type));
    
    /* Get interface index */
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(type);

    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
    if (ioctl(get_handle(), SIOCGIFINDEX, &ifr) < 0) 
    {
        ACE_DEBUG ((LM_ERROR,"CPPPOEPacketHandler::BindInterface(), ioctl(SIOCFIGINDEX): Could not get interface index\n"));
        return -1;
    }
    
    sa.sll_ifindex = ifr.ifr_ifindex;

    /* We're only interested in packets on specified interface */
    if (bind(m_handler, (struct sockaddr *) &sa, sizeof(sa)) < 0) 
    {
        ACE_DEBUG ((LM_ERROR,"CPPPOEPacketHandler::BindInterface(), bind failed, m_handler=%d\n",m_handler));
        return -1;
    }

    return 0;
}
    

