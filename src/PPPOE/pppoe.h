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


#ifndef PPPOE_H
#define PPPOE_H

#include <stdio.h>		        /* For FILE */
#include <sys/types.h>		    /* For pid_t */
#include <linux/if_ether.h>     // IEEE 802.3 Ethernet constants // For ETH_DATA_LEN, struct ethhdr, ETH_ALEN
#include <string.h>             // For memset

/* cloned from dltest.h */
#define         MAXDLBUF        8192
#define         MAXDLADDR       1024
#define         MAXWAIT         15
#define         OFFADDR(s, n)   (unsigned char*)((char*)(s) + (int)(n))
#define         CASERET(s)      case s:  return ("s")


/* Ethernet frame types according to RFC 2516 */
#define ETH_PPPOE_DISCOVERY 0x8863
#define ETH_PPPOE_SESSION   0x8864


/* PPPoE codes */
#define CODE_PADI           0x09
#define CODE_PADO           0x07
#define CODE_PADR           0x19
#define CODE_PADS           0x65
#define CODE_PADT           0xA7

/* Extensions from draft-carrel-info-pppoe-ext-00 */
/* I do NOT like PADM or PADN, but they are here for completeness */
#define CODE_PADM           0xD3
#define CODE_PADN           0xD4

#define CODE_SESS           0x00

/* PPPoE Tags */
#define TAG_END_OF_LIST        0x0000
#define TAG_SERVICE_NAME       0x0101
#define TAG_AC_NAME            0x0102
#define TAG_HOST_UNIQ          0x0103
#define TAG_AC_COOKIE          0x0104
#define TAG_VENDOR_SPECIFIC    0x0105
#define TAG_RELAY_SESSION_ID   0x0110
#define TAG_PPP_MAX_PAYLOAD    0x0120
#define TAG_SERVICE_NAME_ERROR 0x0201
#define TAG_AC_SYSTEM_ERROR    0x0202
#define TAG_GENERIC_ERROR      0x0203

/* Extensions from draft-carrel-info-pppoe-ext-00 */
/* I do NOT like these tags one little bit */
#define TAG_HURL               0x111
#define TAG_MOTM               0x112
#define TAG_IP_ROUTE_ADD       0x121

/* Discovery phase states */
#define STATE_SENT_PADI     0
#define STATE_RECEIVED_PADO 1
#define STATE_SENT_PADR     2
#define STATE_SESSION       3
#define STATE_TERMINATED    4

/* How many PADI/PADS attempts? */
#define MAX_PADI_ATTEMPTS 3

/* Initial timeout for PADO/PADS */
#define PADI_TIMEOUT 5

/* States for scanning PPP frames */
#define STATE_WAITFOR_FRAME_ADDR 0
#define STATE_DROP_PROTO         1
#define STATE_BUILDING_PACKET    2

/* Special PPP frame characters */
#define FRAME_ESC    0x7D
#define FRAME_FLAG   0x7E
#define FRAME_ADDR   0xFF
#define FRAME_CTRL   0x03
#define FRAME_ENC    0x20

#define IPV4ALEN     4
#define SMALLBUF   256

/* Allow for 1500-byte PPPoE data which makes the
   Ethernet packet size bigger by 8 bytes */
/* About ETH_DATA_LEN:
#include <linux/if_ether.h>     // IEEE 802.3 Ethernet constants 
#define ETH_DATA_LEN    1500    // Max. octets in payload 
*/
#define ETH_JUMBO_LEN (ETH_DATA_LEN+8)

/* A PPPoE Packet, including Ethernet headers */
struct PPPoEPacket {
    struct ethhdr ethHdr;	/* Ethernet header */
    unsigned int ver:4;		/* PPPoE Version (must be 1) */
    unsigned int type:4;	/* PPPoE Type (must be 1) */
    unsigned int code:8;	/* PPPoE code */
    unsigned int session:16;	/* PPPoE session */
    unsigned int length:16;	/* Payload length */
    unsigned char payload[ETH_JUMBO_LEN]; /* A bit of room to spare */

    PPPoEPacket()
    {
        ::memset(this, 0, sizeof(struct PPPoEPacket));
    }
};

/* Header size of a PPPoE packet */
#define PPPOE_OVERHEAD 6  /* type, code, session, length */
#define HDR_SIZE (sizeof(struct ethhdr) + PPPOE_OVERHEAD)
#define MAX_PPPOE_PAYLOAD (ETH_JUMBO_LEN - PPPOE_OVERHEAD)
#define PPP_OVERHEAD 2
#define MAX_PPPOE_MTU (MAX_PPPOE_PAYLOAD - PPP_OVERHEAD)
#define TOTAL_OVERHEAD (PPPOE_OVERHEAD + PPP_OVERHEAD)

/* Normal PPPoE MTU without jumbo frames */
#define ETH_PPPOE_MTU (ETH_DATA_LEN - TOTAL_OVERHEAD)

// RFC2516: A value of 0xffff is reserved for future use and MUST NOT be used.
#define PPPOE_RESERVED_SESSION_ID 0xffff

/* PPPoE Tag */
struct PPPoETag
{
    unsigned int type:16;	/* tag type */
    unsigned int length:16;	/* Length of payload */
    unsigned char payload[ETH_JUMBO_LEN]; /* A LOT of room to spare */

    PPPoETag()
    {
        ::memset(this, 0, sizeof(*this));
    }
};

/* Header size of a PPPoE tag */
#define TAG_HDR_SIZE 4

/* Chunk to read from stdin */
#define READ_CHUNK 4096


#define PPPINITFCS16    0xffff  /* Initial FCS value */

#define PPPOE_MAX_ACNAME_LENGTH 200
#define PPPOE_MAX_SERVICE_NAME_LENGTH 200

#define CHECK_ROOM(cursor, start, len) \
do {\
    if (((cursor)-(start))+(len) > MAX_PPPOE_PAYLOAD) { \
        return; \
    } \
} while(0)

/* True if Ethernet address is broadcast or multicast */
#define NOT_UNICAST(e) ((e[0] & 0x01) != 0)
#define BROADCAST(e) ((e[0] & e[1] & e[2] & e[3] & e[4] & e[5]) == 0xFF)
#define NOT_BROADCAST(e) ((e[0] & e[1] & e[2] & e[3] & e[4] & e[5]) != 0xFF)

#endif

