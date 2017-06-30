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

/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
**********************************************************************/
#ifndef RADIUS_H
#define RADIUS_H
#include <stdint.h>

/**
   RADIUS data format

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Code      |  Identifier   |            Length             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                                                               |
   |                         Authenticator                         |
   |                                                               |
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Attributes ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-
 */

/** Lengths define in RFC 2865 Section 3 
 */
const uint16_t RadiusPacketHeaderSize = 20;
const uint16_t RadiusMinPacketSize = RadiusPacketHeaderSize;
const uint16_t RadiusMaxPacketSize = 4096;
const uint16_t RadiusAuthenticatorLength = 16;

struct RadiusHdr
{
    uint8_t  code;
    uint8_t  identifier;
    uint16_t length;
    uint8_t  authenticator[ RadiusAuthenticatorLength ];
};

/** Raw Radius message data buffer
 */
union RawMessage
{
    /* Packet buffer */
    uint8_t buffer[ RadiusMaxPacketSize ];
    /* TODO: Buffer size not big enough for Calculating authenticator */

    /* Message header */
    RadiusHdr msgHdr;
};


/** RADIUS Codes
 */
typedef uint8_t RadiusPacketType;

const uint8_t RP_ACCESS_REQUEST      =  1;
const uint8_t RP_ACCESS_ACCEPT       =  2;
const uint8_t RP_ACCESS_REJECT       =  3;
const uint8_t RP_ACCOUNTING_REQUEST  =  4;
const uint8_t RP_ACCOUNTING_RESPONSE =  5;
const uint8_t RP_ACCESS_CHALLENGE    = 11;


// Attribute format
//
//    0                   1                   2
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//   |     Type      |    Length     |  Value ...
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

const uint16_t RadiusMaxAttributeLength = 255;    // 256 - 2 + 1 for \0

/** RADIUS Attribute Types
    TODO: Add other attributes
 */
typedef uint8_t RadiusAttributeType;

const uint8_t RA_USER_NAME            =  1;
const uint8_t RA_USER_PASSWORD        =  2;
const uint8_t RA_CHAP_PASSWORD        =  3;
const uint8_t RA_NAS_IP_ADDRESS       =  4;
const uint8_t RA_NAS_PORT             =  5;
const uint8_t RA_SERVICE_TYPE         =  6;
const uint8_t RA_REPLY_MESSAGE        = 18;
const uint8_t RA_STATE                = 24;
const uint8_t RA_CLASS                = 25;
const uint8_t RA_VENDOR_SPECIFIC      = 26;
const uint8_t RA_SESSION_TIMEOUT      = 27;
const uint8_t RA_CALLED_STATION_ID    = 30;
const uint8_t RA_CALLING_STATION_ID   = 31;
const uint8_t RA_ACCT_STATUS_TYPE     = 40;
const uint8_t RA_ACCT_DELAY_TIME      = 41;
const uint8_t RA_ACCT_INPUT_OCTETS    = 42;
const uint8_t RA_ACCT_OUTPUT_OCTETS   = 43;
const uint8_t RA_ACCT_SESSION_ID      = 44;
const uint8_t RA_ACCT_AUTHENTIC       = 45;
const uint8_t RA_ACCT_SESSION_TIME    = 46;
const uint8_t RA_ACCT_INPUT_PACKETS   = 47;
const uint8_t RA_ACCT_OUTPUT_PACKETS  = 48;
const uint8_t RA_ACCT_TERMINATE_CAUSE = 49;
const uint8_t RA_NAS_PORT_TYPE        = 61;
const uint8_t RA_DIGEST_RESPONSE      = 206;
const uint8_t RA_DIGEST_ATTRIBUTES    = 207;
const uint8_t    RA_DIGEST_REALM      = 1;
const uint8_t    RA_DIGEST_NONCE      = 2;
const uint8_t    RA_DIGEST_METHOD     = 3;
const uint8_t    RA_DIGEST_URI        = 4;
const uint8_t    RA_DIGEST_ALGORITHM  = 6;
const uint8_t    RA_DIGEST_USER_NAME  = 10;

/** RA_ACCT_STATUS_TYPE Values
 */
typedef uint8_t RadiusAcctStatusType;

const uint32_t RAS_START   = 1;
const uint32_t RAS_STOP    = 2;
const uint32_t RAS_INTERIM = 3;
const uint32_t RAS_ON      = 7;
const uint32_t RAS_OFF     = 8;


/** RA_ACCT_TERMINATE_CAUSE Values (incomplete)
 */
typedef uint8_t RadiusAcctTerminateCauseType;

const uint32_t RATC_USER_REQUEST         =  1;
const uint32_t RATC_LOST_CARRIER         =  2;
const uint32_t RATC_LOST_SERVICE         =  3;
const uint32_t RATC_IDLE_TIMEOUT         =  4;
const uint32_t RATC_SESSION_TIMEOUT      =  5;
const uint32_t RATC_ADMIN_RESET          =  6;
const uint32_t RATC_ADMIN_REBOOT         =  7;
const uint32_t RATC_PORT_ERROR           =  8;
const uint32_t RATC_NAS_ERROR            =  9;
const uint32_t RATC_NAS_REQUEST          = 10;
const uint32_t RATC_NAS_REBOOT           = 11;
const uint32_t RATC_SERVICE_UNAVAILABLE  = 15;


/** RA_SERVICE_TYPE Values (incomplete)
 */
typedef uint8_t RadiusServiceTypeType;

const uint32_t RAST_LOGIN         =  1;


/** RA_NAS_PORT_TYPE Values (incomplete)
 */
typedef uint8_t RadiusNasPortTypeType;

const uint32_t RANPT_VIRTUAL         =  5;

#endif


