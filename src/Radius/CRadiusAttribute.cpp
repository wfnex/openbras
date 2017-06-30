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

#include <netinet/in.h>
#include <cstdio>
#include <cstring>
#include "CRadiusAttribute.h"

std::string itos(uint32_t a)
{
    char buffer[1024]={0};
    snprintf(buffer, sizeof(buffer), "%d",a);
    return std::string(buffer);
}


CRadiusAttribute::CRadiusAttribute( const RadiusAttributeType type,
                                  const uint8_t* value,
                                  const uint8_t length,
                                  const uint32_t vendorId ) :
    myType( type ),
    myVendorId( vendorId )
{
    assert( ( type != RA_VENDOR_SPECIFIC && vendorId == 0 ) ||
            ( type == RA_VENDOR_SPECIFIC && vendorId != 0 ) );

    if( type == RA_USER_PASSWORD && (length % 16 ) != 0 )
    {
        // Expand password to a multiple of 16 octets and fill it up with '\0'
        uint8_t value16[ RadiusMaxAttributeLength ];
        ::memcpy( value16, value, length );
        ::memset( value16+length, '\0', 16 - (length % 16) );
        myValue = CRadiusData( value16, (length+16)&0xFFF0 );
    }
    else
    {
        myValue = CRadiusData( value, length );
    }
}


CRadiusAttribute::CRadiusAttribute( const RadiusAttributeType type,
                                  const CRadiusData& value,
                                  const uint32_t vendorId ) :
    myType( type ),
    myValue( value ),
    myVendorId( vendorId )
{
    assert( ( type != RA_VENDOR_SPECIFIC && vendorId == 0 ) ||
            ( type == RA_VENDOR_SPECIFIC && vendorId != 0 ) );

    if( type == RA_USER_PASSWORD )
    {
        if( value.length() % 16 )
        {
            // Expand password to a multiple of 16 octets and fill it up with '\0'
            uint8_t value16[ RadiusMaxAttributeLength ];
            uint16_t length = value.length();
            ::memcpy( value16, value.data(), length );
            ::memset( value16+length, '\0', 16 - (length % 16) );
            ACE_DEBUG ((LM_DEBUG,"User-Password length %d -> %d",
                                    length,
                                    (length+16)&0xFFF0));
            myValue = CRadiusData( value16, (length+16)&0xFFF0 );
        }
    }
}

CRadiusData
CRadiusAttribute::encode() const
{
    CRadiusData msgDat;

    msgDat.append( &myType, 1 );

    if( myType == RA_VENDOR_SPECIFIC )
    {
        //   Vendor-Specific Attribute format
        //
        //    0                   1                   2                   3
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //   |     Type      |  Length       |            Vendor-Id
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //        Vendor-Id (cont)           |  String...
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

        // 6 octets for Type, Length and Vendor-Id
        uint8_t  msgLen  = myValue.length() + 6;
        msgDat.append( &msgLen, 1 );

        uint32_t vendId  = htonl( myVendorId );
        msgDat.append( &vendId, 4 );

        msgDat += myValue;
    }
    else
    {
        //   Attribute format
        //
        //    0                   1                   2
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
        //   |     Type      |    Length     |  Value ...
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

        // 2 octets for Type and Length
        uint8_t msgLen = myValue.length() + 2;
        msgDat.append( &msgLen, 1 );

        msgDat += myValue;
    }
    return msgDat;
}

std::string
CRadiusAttribute::verbose() const
{
    char attrLenStr[4];
    snprintf( attrLenStr, 4, "%3d", myValue.length() + 2 );

    char tempBuff[ RadiusMaxAttributeLength ];

    std::string attrStr;

    switch( myType )
    {
        case RA_USER_NAME:
        {
        //    0                   1                   2
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
        //   |     Type      |    Length     |  String ...
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

            memcpy( tempBuff, myValue.data(), myValue.length() );
            tempBuff[ myValue.length() ] = '\0';
            attrStr = "  User-Name (" + ::itos( myType ) + ") = \"" +
                      tempBuff + "\"\n";
            break;
        }
        case RA_USER_PASSWORD:
        {
        //    0                   1                   2
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
        //   |     Type      |    Length     |  String ...
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

            memcpy( tempBuff, myValue.data(), myValue.length() );
            tempBuff[ myValue.length() ] = '\0';
            attrStr = "  User-Password (" + ::itos( myType ) + ") = \"" +
                      tempBuff + "\"\n";
            break;
        }
        case RA_CHAP_PASSWORD:
        {
        //    0                   1                   2
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
        //   |     Type      |    Length     |  CHAP Ident   |  String ...
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

            // TODO: show CHAP Ident, but not password
            attrStr = "  CHAP-Password (" + ::itos( myType ) + ")\n";
            break;
        }
        case RA_NAS_IP_ADDRESS:
        {
        //    0                   1                   2                   3
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //   |     Type      |    Length     |            Address
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //            Address (cont)         |
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            snprintf( tempBuff, 16, "%d.%d.%d.%d",
                                     myValue.data()[0],
                                     myValue.data()[1],
                                     myValue.data()[2],
                                     myValue.data()[3] );
            attrStr = "  NAS-IP-Address (" + ::itos( myType ) + ") = " +
                      tempBuff + "\n";
            break;
        }
        case RA_NAS_PORT:
        {
        //    0                   1                   2                   3
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //   |     Type      |    Length     |             Value
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //              Value (cont)         |
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            u_int32_t port = ntohl( *(reinterpret_cast< const u_int32_t * >(myValue.data())) );
            attrStr = "  NAS-Port (" + ::itos( myType ) + ") = " +
                      ::itos( port ) + "\n";
            break;
        }
        case RA_SERVICE_TYPE:
        {
        //    0                   1                   2                   3
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //   |     Type      |    Length     |             Value
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //              Value (cont)         |
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            u_int32_t serviceType = ntohl( *(reinterpret_cast< const u_int32_t * >(myValue.data())) );
            attrStr = "  Service-Type (" + ::itos( myType ) + ") = " +
                      ::itos( serviceType ) + "\n";
            // TODO: more info
            break;
        }
        case RA_REPLY_MESSAGE:
        {
        //    0                   1                   2
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
        //   |     Type      |    Length     |  Text ...
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

            memcpy( tempBuff, myValue.data(), myValue.length() );
            tempBuff[ myValue.length() ] = '\0';
            attrStr = "  Reply-Message (" + ::itos( myType ) + ") = \"" +
                      tempBuff + "\"\n";
            break;
        }
        case RA_STATE:
        {
        //    0                   1                   2
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
        //   |     Type      |    Length     |  String ...
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

            attrStr = "  State (" + ::itos( myType ) + ") \n";
            // TODO: more info
            break;
        }
        case RA_CLASS:
        {
        //    0                   1                   2
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
        //   |     Type      |    Length     |  String ...
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

            attrStr = "  Class (" + ::itos( myType ) + ") \n";
            // TODO: more info
            break;
        }
        case RA_VENDOR_SPECIFIC:
        {
        //    0                   1                   2                   3
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //   |     Type      |  Length       |            Vendor-Id
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //        Vendor-Id (cont)           |  String...
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

            if (myValue.length() < 3 || myVendorId != 9 ||
	      myValue.data()[0] != 1 || myValue.data()[1] > myValue.length())
	    {
                // TODO: more info
		attrStr = "  Vendor-Specific (" + ::itos( myType ) + ") \n";
		break;
	    }
	    u_int8_t avpLength = myValue.data()[1] - 2;
	    memcpy( tempBuff, myValue.data() + 2, avpLength);
	    tempBuff[avpLength] = '\0';
	    attrStr = "  Cisco AVPair (" + ::itos( myValue.data()[0] ) +
		      ") = \"" + tempBuff + "\"\"\n";
            break;
        }
        case RA_SESSION_TIMEOUT:
        {
        //    0                   1                   2                   3
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //   |     Type      |    Length     |             Value
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //              Value (cont)         |
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            u_int32_t to = ntohl( *(reinterpret_cast< const u_int32_t * >(myValue.data())) );
            attrStr = "  Session-Timeout (" + ::itos( myType ) + ") = " +
                      ::itos( to ) + "\n";
            break;
        }
        case RA_CALLED_STATION_ID:
        {
        //    0                   1                   2
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
        //   |     Type      |    Length     |  String ...
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

            memcpy( tempBuff, myValue.data(), myValue.length() );
            tempBuff[ myValue.length() ] = '\0';
            attrStr = "  Called-Station-Id (" + ::itos( myType ) + ") = \"" +
                      tempBuff + "\"\n";
            break;
        }
        case RA_CALLING_STATION_ID:
        {
        //    0                   1                   2
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
        //   |     Type      |    Length     |  String ...
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

            memcpy( tempBuff, myValue.data(), myValue.length() );
            tempBuff[ myValue.length() ] = '\0';
            attrStr = "  Calling-Station-Id (" + ::itos( myType ) + ") = \"" +
                      tempBuff + "\"\n";
            break;
        }
        case RA_ACCT_STATUS_TYPE:
        {
        //    0                   1                   2                   3
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //   |     Type      |    Length     |             Value
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //              Value (cont)         |
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            std::string acctTypeStr;
            u_int32_t acctType = ntohl( *(reinterpret_cast< const u_int32_t * >(myValue.data())) );
            switch( acctType )
            {
                case RAS_START:
                {
                    acctTypeStr = "Start (" + ::itos(acctType) + ")";
                    break;
                }
                case RAS_STOP:
                {
                    acctTypeStr = "Stop (" + ::itos(acctType) + ")";
                    break;
                }
                case RAS_INTERIM:
                {
                    acctTypeStr = "Interim-Update (" + ::itos(acctType) + ")";
                    break;
                }
                case RAS_ON:
                {
                    acctTypeStr = "Accounting-On (" + ::itos(acctType) + ")";
                    break;
                }
                case RAS_OFF:
                {
                    acctTypeStr = "Accounting-Off (" + ::itos(acctType) + ")";
                    break;
                }
                default:
                {
                    acctTypeStr = "Unknown (" + ::itos(acctType) + ")";
                }
            }
            attrStr = "  Acct-Status-Type (" + ::itos( myType ) + ") = " +
                      acctTypeStr + "\n";
            break;
        }
        case RA_ACCT_DELAY_TIME:
        {
        //    0                   1                   2                   3
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //   |     Type      |    Length     |             Value
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //              Value (cont)         |
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            u_int32_t duration = ntohl( *(reinterpret_cast< const u_int32_t * >(myValue.data())) );
            attrStr = "  Acct-Delay-Time (" + ::itos( myType ) + ") = " +
                      ::itos( duration ) + "\n";
            break;
        }
        case RA_ACCT_INPUT_OCTETS:
        {
        //    0                   1                   2                   3
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //   |     Type      |    Length     |             Value
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //              Value (cont)         |
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            attrStr = "  Acct-Input-Octets (" + ::itos( myType ) + ")\n";
            // TODO: more info
            break;
        }
        case RA_ACCT_OUTPUT_OCTETS:
        {
        //    0                   1                   2                   3
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //   |     Type      |    Length     |             Value
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //              Value (cont)         |
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            attrStr = "  Acct-Output-Octets (" + ::itos( myType ) + ")\n";
            // TODO: more info
            break;
        }
        case RA_ACCT_SESSION_ID:
        {
        //    0                   1                   2
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //   |     Type      |    Length     |  Text ...
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            memcpy( tempBuff, myValue.data(), myValue.length() );
            tempBuff[ myValue.length() ] = '\0';
            attrStr = "  Acct-Session-Id (" + ::itos( myType ) + ") = \"" +
                      tempBuff + "\"\n";
            break;
        }
        case RA_ACCT_AUTHENTIC:
        {
        //    0                   1                   2                   3
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //   |     Type      |    Length     |             Value
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //              Value (cont)         |
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            attrStr = "  Acct-Authentic (" + ::itos( myType ) + ")\n";
            // TODO: more info
            break;
        }
        case RA_ACCT_SESSION_TIME:
        {
        //    0                   1                   2                   3
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //   |     Type      |    Length     |             Value
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //              Value (cont)         |
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            u_int32_t duration = ntohl( *(reinterpret_cast< const u_int32_t * >(myValue.data())) );
            attrStr = "  Acct-Session-Time (" + ::itos( myType ) + ") = " +
                      ::itos( duration ) + "\n";
            break;
        }
        case RA_ACCT_INPUT_PACKETS:
        {
        //    0                   1                   2                   3
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //   |     Type      |    Length     |             Value
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //              Value (cont)         |
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            attrStr = "  Acct-Input-Packets (" + ::itos( myType ) + ")\n";
            // TODO: more info
            break;
        }
        case RA_ACCT_OUTPUT_PACKETS:
        {
        //    0                   1                   2                   3
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //   |     Type      |    Length     |             Value
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //              Value (cont)         |
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            attrStr = "  Acct-Output-Packets (" + ::itos( myType ) + ")\n";
            // TODO: more info
            break;
        }
        case RA_ACCT_TERMINATE_CAUSE:
        {
        //    0                   1                   2                   3
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //   |     Type      |    Length     |             Value
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //              Value (cont)         |
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            std::string causeStr;
            uint32_t cause = ntohl( *(reinterpret_cast< const u_int32_t * >(myValue.data())) );
            switch( cause )
            {
                case RATC_USER_REQUEST:
                {
                    causeStr = "User Request (" + ::itos(cause) + ")";
                    break;
                }
                case RATC_LOST_CARRIER:
                {
                    causeStr = "Lost Carrier (" + ::itos(cause) + ")";
                    break;
                }
                case RATC_LOST_SERVICE:
                {
                    causeStr = "Lost Service (" + ::itos(cause) + ")";
                    break;
                }
                case RATC_IDLE_TIMEOUT:
                {
                    causeStr = "Idle Timeout (" + ::itos(cause) + ")";
                    break;
                }
                case RATC_SESSION_TIMEOUT:
                {
                    causeStr = "Session Timeout (" + ::itos(cause) + ")";
                    break;
                }
                case RATC_ADMIN_RESET:
                {
                    causeStr = "Admin Reset (" + ::itos(cause) + ")";
                    break;
                }
                case RATC_ADMIN_REBOOT:
                {
                    causeStr = "Admin Reboot (" + ::itos(cause) + ")";
                    break;
                }
                case RATC_PORT_ERROR:
                {
                    causeStr = "Port Error (" + ::itos(cause) + ")";
                    break;
                }
                case RATC_NAS_ERROR:
                {
                    causeStr = "NAS Error (" + ::itos(cause) + ")";
                    break;
                }
                case RATC_NAS_REQUEST:
                {
                    causeStr = "NAS Request (" + ::itos(cause) + ")";
                    break;
                }
                case RATC_NAS_REBOOT:
                {
                    causeStr = "NAS Reboot (" + ::itos(cause) + ")";
                    break;
                }
                case RATC_SERVICE_UNAVAILABLE:
                {
                    causeStr = "Service Unavailable (" + ::itos(cause) + ")";
                    break;
                }
                default:
                {
                    causeStr = "Unknown (" + ::itos(cause) + ")";
                }
            }
            attrStr = "  Acct-Terminate-Cause (" + ::itos( myType ) + ") = " +
                      causeStr + "\n";
            break;
        }
        case RA_NAS_PORT_TYPE:
        {
        //    0                   1                   2                   3
        //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //   |     Type      |    Length     |             Value
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //              Value (cont)         |
        //   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            uint32_t portType = ntohl( *(reinterpret_cast< const u_int32_t * >(myValue.data())) );
            attrStr = "  NAS-Port-Type (" + ::itos( myType ) + ") = " +
                      ::itos( portType ) + "\n";
            // TODO: more info
            break;
        }
	case RA_DIGEST_RESPONSE:
	{
	//    0                   1                   2
	//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
	//   |     Type      |    Length     |  String ...
	//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

	    memcpy( tempBuff, myValue.data(), myValue.length() );
	    tempBuff[ myValue.length() ] = '\0';
	    attrStr = "  Digest-Response (" + ::itos( myType ) + ") = \"" +
		      tempBuff + "\"\n";
	    break;
	}
	case RA_DIGEST_ATTRIBUTES:
	{
	//    0                   1                   2                   3
	//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//   |     Type      |    Length     |    Sub-Type   |  Sub-Lengh    |
	//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//   |   String ...
	//   +-+-+-+-+-+-+-+-

        uint8_t subType = *(reinterpret_cast< const uint8_t * >(myValue.data()));
        uint8_t subLength = *(reinterpret_cast< const uint8_t * >(myValue.data() + 1));
        std::string subAttrStr;

        switch (subType)
        {
            case RA_DIGEST_REALM:
                subAttrStr = "Realm";
                break;
            case RA_DIGEST_NONCE:
                subAttrStr = "Nonce";
                break;
            case RA_DIGEST_METHOD:
                subAttrStr = "Method";
                break;
            case RA_DIGEST_URI:
                subAttrStr = "URI";
                break;
            case RA_DIGEST_ALGORITHM:
                subAttrStr = "Algorithm";
                break;
            case RA_DIGEST_USER_NAME:
                subAttrStr = "User-Name";
                break;
            default:
                subAttrStr = "Unknown";
                break;
            }

            memcpy( tempBuff, myValue.data() + 2, subLength - 2);
            tempBuff[ subLength - 2] = '\0';
            attrStr = "  Digest-Attributes (" + ::itos( myType ) + ") = \"" +
              subAttrStr + " (" + ::itos( subType ) + ") = \"" +
              tempBuff + "\"\"\n";
            break;
        }
        default:
        {
            attrStr = "  Unknown (" + ::itos( myType ) + ")\n";
        }
    }
    return std::string( attrLenStr + attrStr );
}


