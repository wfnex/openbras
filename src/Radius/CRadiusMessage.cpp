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
#include <time.h>
#include "CRadiusMessage.h"


/** Create a RADIUS Message with type, no Attriubte yet
    Useful for composing a request
 */
CRadiusMessage::CRadiusMessage( const RadiusPacketType code )
{
    ACE_DEBUG ((LM_DEBUG,"Create packet with code %d\n", code));

    // These are the requests that we handle now
    assert( code == RP_ACCOUNTING_REQUEST ||
            code == RP_ACCESS_REQUEST );

    ::memset( &myData, 0, sizeof( struct RadiusHdr ) );
    myData.msgHdr.code = code;
    myData.msgHdr.length = htons( sizeof( struct RadiusHdr ) );
    ::memset( &myRequestAuthenticator, 0, RadiusAuthenticatorLength );

    // Attributes are added later
    // Identifier is set and Authenticator is calculated when message is sent
}
 

/** Create a RADIUS Message with type, identifier and an Authenticator
    No Attriubte yet
    Use for composing a response
 */
CRadiusMessage::CRadiusMessage( const RadiusPacketType code,
                              const u_int8_t requestId,
                              const u_int8_t *requestAuth )
{
    ACE_DEBUG ((LM_DEBUG,"Create packet with code %d\n", code));

    assert( requestAuth != 0 );

    // These are the responses that we handle now
    assert( code == RP_ACCOUNTING_RESPONSE ||
            code == RP_ACCESS_REJECT ||
            code == RP_ACCESS_CHALLENGE ||
            code == RP_ACCESS_ACCEPT );

    ::memset( &myData, 0, sizeof( struct RadiusHdr ) );
    myData.msgHdr.code = code;
    myData.msgHdr.identifier = requestId;
    myData.msgHdr.length = htons( sizeof( struct RadiusHdr ) );

    // save original request Authenticator for later use
    ::memcpy( &myRequestAuthenticator, requestAuth, RadiusAuthenticatorLength );

    // Attributes are added later
    // Authenticator is calculated when message is sent
}
 
/** Create a RADIUS Message from data
    Use when a message is received
 */
CRadiusMessage::CRadiusMessage( const CRadiusData rawMsg, const char* secret)
{
    ACE_DEBUG ((LM_DEBUG,"Create packet with raw data\n"));

    uint16_t msgLen = rawMsg.length();

    if( msgLen < RadiusMinPacketSize || msgLen > RadiusMaxPacketSize )
    {
        return;
    }

    // Save raw packet
    memcpy( myData.buffer, rawMsg.data(), msgLen );

    if( msgLen != ntohs(myData.msgHdr.length) )
    {
        return;
    }

    decodeAttributes( secret );


    if( myData.msgHdr.code == RP_ACCOUNTING_REQUEST )
    {
        // Verify Accounting Request Authenticator
        if( verifyAccountingRequestAuthenticator( secret ) == false )
        {
            return;
        }
    }
    // Access Request Authenticator is not verified
    // Response Authenticator should be verified with the original
    //   request Authenticator later
}


/** Decode Attributes
    If an Attribute is received in an Access-Request but an
    exception is thrown, an Access-Reject SHOULD be transmitted.
    If an Attribute is received in an Access-Accept, Access-Reject
    or Access-Challenge packet and an exception is thrown, the
    packet must either be treated as an Access-Reject or else
    silently discarded.

    TODO: Enforce attribute quantity guide Section 5.44 of RFC 2865
 */
int CRadiusMessage::decodeAttributes( const char* secret )
{
    uint16_t totalLen = ntohs( myData.msgHdr.length );

    ACE_DEBUG ((LM_DEBUG,
           "Received code=%d id=%d length=%d\n",
           myData.msgHdr.code, myData.msgHdr.identifier, totalLen));
    
    // Move to the beginning of Attributes
    uint32_t parsed = sizeof( RadiusHdr );
    const uint8_t *msgPtr = myData.buffer + parsed;

    const uint8_t            *attrPtr;

    RadiusAttributeType attrType;
    uint8_t            attrLen;

    uint32_t vendorId;    // For VSA

    // Build Attribute list
    while( parsed < totalLen )
    {
        attrPtr  = msgPtr;
        attrType = *attrPtr++;
        attrLen  = *attrPtr++;
        msgPtr += attrLen;
        parsed += attrLen;

        if( (attrLen < 2) || (parsed > totalLen) )
        {
            return -1;
        }

        if( attrType == RA_VENDOR_SPECIFIC )    // VSA
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
            if( attrLen > 8 )
            {
                // Extract Vendor-Id
                vendorId = ntohl( *(reinterpret_cast< const u_int32_t * >(attrPtr)) );
                attrPtr += 4;                // Move pointer to Vendor type
                // Exclude Type, Length and Vendor-Id
                CRadiusData rdat( attrPtr, attrLen - 6 );
                CRadiusAttribute attr( attrType, rdat, vendorId );
                myAttributes.push_back( attr );
                // Multiple subattributes may be encoded within a single
                // Vendor-Specific attribute. They should further be decoded
                // by a vendor-specific application.
            }
            else
            {
                return -1;
            }
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
            if( attrType == RA_USER_PASSWORD )
            {
                if( myData.msgHdr.code != RP_ACCESS_REQUEST )
                {
                    return -1;
                }
                if( ((attrLen-2)%16) != 0 )
                {
                    return -1;
                }
                // Decode password
                CRadiusData hiddenPassword( attrPtr, attrLen - 2 );
                uint8_t decodedPassword[ 256 ];
                decodeUserPassword( decodedPassword, secret, hiddenPassword );
                // Store decoded password in attribute list
                CRadiusData password( decodedPassword, attrLen - 2 );
                CRadiusAttribute attr( attrType, password );
                myAttributes.push_back( attr );
            }
            else
            {
                // Exclude Type and Length
                CRadiusData rdat( attrPtr, attrLen - 2 );
                CRadiusAttribute attr( attrType, rdat );
                myAttributes.push_back( attr );
            }
        }
    }

    return 0;
}

//verify Acct Request Authenticator
bool
CRadiusMessage::verifyAccountingRequestAuthenticator( const char* secret )
{
    ACE_DEBUG ((LM_DEBUG,"Verify Accounting Request Authenticator\n"));

    assert( myData.msgHdr.code == RP_ACCOUNTING_REQUEST );

    // Copy of Authenticator
    uint8_t auth[ RadiusAuthenticatorLength ];
    ::memcpy( auth, myData.msgHdr.authenticator, RadiusAuthenticatorLength );

    // Re-calculate
    calcAccountingRequestAuthenticator( secret );

    // Compare
    return( 0 == ::memcmp( auth,
                         myData.msgHdr.authenticator,
                         RadiusAuthenticatorLength ) );
}

//verify Response Authenticator
bool
CRadiusMessage::verifyResponseAuthenticator( const u_int8_t *reqAuth,
                                            const char* secret )
{
    ACE_DEBUG ((LM_DEBUG,"Verify Response Authenticator"));

    assert( myData.msgHdr.code == RP_ACCESS_ACCEPT ||
            myData.msgHdr.code == RP_ACCESS_REJECT ||
            myData.msgHdr.code == RP_ACCOUNTING_RESPONSE );

    // Copy Request Authenticator
    memcpy( myRequestAuthenticator, reqAuth, RadiusAuthenticatorLength );

    // Copy Response Authenticator
    uint8_t respAuth[ RadiusAuthenticatorLength ];
    memcpy( respAuth, myData.msgHdr.authenticator, RadiusAuthenticatorLength );

    // Re-calculate
    calcResponseAuthenticator( secret );

    return( 0 == ::memcmp( respAuth,
                         myData.msgHdr.authenticator,
                         RadiusAuthenticatorLength ) );
}

//Add Radius Attribute
bool
CRadiusMessage::add( const CRadiusAttribute& attr )
{
    ACE_DEBUG ((LM_DEBUG,"Add Attribute of type %d\n", attr.type()));

    uint16_t attrLen = attr.length();
    uint16_t currLen = ntohs( myData.msgHdr.length );
    if( (currLen + attrLen) > RadiusMaxPacketSize )
    {
        ACE_DEBUG ((LM_ERROR,"Cannot add %d octets, current length is %d\n",
                        attrLen, myData.msgHdr.length));
        return false;
    }

    memcpy( myData.buffer + currLen,
            attr.encode().data() ,
            attrLen );
    myData.msgHdr.length = htons( currLen + attrLen );

    myAttributes.push_back( attr );

    return true;
}

/** Encode User-Password in Access-Request according to RFC 2865 Section 5.2

      Call the shared secret S and the pseudo-random 128-bit Request
      Authenticator RA.  Break the password into 16-octet chunks p1, p2,
      etc.  with the last one padded at the end with nulls to a 16-octet
      boundary.  Call the ciphertext blocks c(1), c(2), etc.  We'll need
      intermediate values b1, b2, etc.

         b1 = MD5(S + RA)       c(1) = p1 xor b1
         b2 = MD5(S + c(1))     c(2) = p2 xor b2
                .                       .
                .                       .
                .                       .
         bi = MD5(S + c(i-1))   c(i) = pi xor bi

      The String will contain c(1)+c(2)+...+c(i) where + denotes
      concatenation.
 */
void
CRadiusMessage::encodeUserPassword( uint8_t* newPassword,
                                   const char* secret,
                                   const CRadiusData& oldPassword )
{
    ACE_DEBUG ((LM_DEBUG,"Encode User-Password\n"));

    int secretLen = strlen(secret);
    assert( secretLen < 256 );

    int hashLen = strlen(secret) + RadiusAuthenticatorLength;
    uint8_t hashInput[ 256 + RadiusAuthenticatorLength ];

    // Secret + Request Authenticator
    ::memcpy( hashInput, secret, secretLen );
    ::memcpy( hashInput + secretLen,
            myData.msgHdr.authenticator,
            RadiusAuthenticatorLength );

    assert( (oldPassword.length() % 16) == 0 );

    const uint8_t* p = oldPassword.data();
    uint8_t b[ 16 ];

    unsigned int j;
    int i;
    for( i = 0; i < oldPassword.length(); i += sizeof(b) )
    {
        if( i )
        {
            memcpy( hashInput + secretLen,
                    newPassword + (i - sizeof(b)),
                    sizeof(b) );
        }
        calcMD5( b, hashInput, hashLen );
        for( j = 0; j < sizeof(b); ++j )
        {
            newPassword[i+j] = *(p+i+j) ^ b[j];
        }
    }
}

/** Decode User-Password in Access-Request

      Reversal of encodeUserPassword above
 */
void
CRadiusMessage::decodeUserPassword( uint8_t* newPassword,
                                   const char* secret,
                                   const CRadiusData& oldPassword )
{
    ACE_DEBUG ((LM_DEBUG,"Decode User-Password\n"));

    int secretLen = strlen(secret);
    assert( secretLen < 256 );

    int hashLen = strlen(secret) + RadiusAuthenticatorLength;
    uint8_t hashInput[ 256 + RadiusAuthenticatorLength ];

    memcpy( hashInput, secret, secretLen );

    assert( (oldPassword.length() % 16) == 0 );

    const uint8_t* p = oldPassword.data();
    uint8_t b[ 16 ];

    unsigned int j;
    int i;
    for( i = oldPassword.length() - sizeof(b); i >= 0; i -= sizeof(b) )
    {
        if( i )
        {
            memcpy( hashInput + secretLen,
                    p + (i - sizeof(b)),
                    sizeof(b) );
        }
        else
        {
            memcpy( hashInput + secretLen,
                    myData.msgHdr.authenticator,
                    RadiusAuthenticatorLength );
        }
        calcMD5( b, hashInput, hashLen );
        for( j = 0; j < sizeof(b); ++j )
        {
            newPassword[i+j] = *(p+i+j) ^ b[j];
        }
    }
}

/** Hide User-Password in Access-Request
 */
void
CRadiusMessage::hideUserPassword( const char* secret )
{
    ACE_DEBUG ((LM_DEBUG,"Hide User-Password"));

    assert( myData.msgHdr.code == RP_ACCESS_REQUEST );

    CRadiusData password;
    RadiusAttrIter itr = getAll(RA_USER_PASSWORD).begin(); 
    password = itr->value();

    
    uint8_t hiddenPassword[ 256 ];
    encodeUserPassword( hiddenPassword, secret, password );

    // Replace password in raw message with hidden password
    int offset = RadiusPacketHeaderSize;
    for ( RadiusAttrIter itr = myAttributes.begin();
          itr != myAttributes.end();
          itr++ )
    {   
        if( itr->type() == RA_USER_PASSWORD )
        {
            memcpy( myData.buffer + offset + 2,
                    hiddenPassword,
                    password.length() );
        }
        else
        {
            offset += itr->length();
        }
    }
}

/** Calculate and set the Authenticator
    Must be called right before the packet is sent
 */
void
CRadiusMessage::calcAuthenticator( const char* secret )
{
    switch( myData.msgHdr.code )
    {
        case RP_ACCESS_REQUEST:
        {
            calcAccessRequestAuthenticator();
            hideUserPassword( secret );
            break;
        }
        case RP_ACCESS_ACCEPT:
        case RP_ACCESS_REJECT:
        case RP_ACCOUNTING_RESPONSE:
        case RP_ACCESS_CHALLENGE:
        {
            calcResponseAuthenticator( secret );
            break;
        }
        case RP_ACCOUNTING_REQUEST:
        {
            calcAccountingRequestAuthenticator( secret );
            break;
        }
        default:
        {
            ACE_DEBUG ((LM_ERROR,"Cannot calculate Authenticator for code %d\n",
                            myData.msgHdr.code));
            assert( 0 );
        }
    }
}

/** Calculate and set the myRequestAuthenticator for Access-Request packets
 */
void
CRadiusMessage::calcAccessRequestAuthenticator()
{
    // Generate a global and temporal unique Authenticator
    // Request Authenticator = MD5( time + ip + id )
    uint8_t hashInput[ 256 ];
    unsigned int t = time( NULL );
    memcpy( hashInput, &t, sizeof(t) );
    int hashLen = sizeof(t);

    RadiusAttrIter itr = getAll(RA_NAS_IP_ADDRESS).begin();
    memcpy( hashInput+hashLen,
            itr->value().data(),
            itr->value().length() );
    hashLen += itr->value().length();
    memcpy( hashInput+hashLen,
            &myData.msgHdr.identifier,
            sizeof(myData.msgHdr.identifier) );
    hashLen += sizeof(myData.msgHdr.identifier);
    
    // Calculate
    calcMD5( myRequestAuthenticator, hashInput, hashLen );
    
    // Set Authenticator in Access Request raw message
    memcpy( myData.msgHdr.authenticator,
            myRequestAuthenticator,
            RadiusAuthenticatorLength );

}

/** Calculate and set the Authenticator for Response (Access-Accept,
    Access-Reject, Access-Challenge or Accounting Response) packets
 */
void
CRadiusMessage::calcResponseAuthenticator( const char* secret )
{
    // See RFC 2865 Section 3 and RFC 2866 Section 3
    // MD5( Code + ID + Length + myRequestAuthenticator + Attributes + Secret )

    uint16_t msgLen = ntohs( myData.msgHdr.length );

    // TODO: make buffer bigger to accommodate secret
    assert( (msgLen + strlen(secret)) <= RadiusMaxPacketSize );

    // Load original request's Authenticator
    memcpy( myData.msgHdr.authenticator,
            myRequestAuthenticator,
            RadiusAuthenticatorLength );

    // Load secret behind message
    memcpy( myData.buffer + msgLen, secret, strlen(secret) );

    uint8_t auth[ RadiusAuthenticatorLength ];

    // Calculate
    calcMD5( auth, myData.buffer, msgLen + strlen(secret) );

    // Set Authenticator in response packet
    memcpy( myData.msgHdr.authenticator,
            auth,
            RadiusAuthenticatorLength );
}

/** Calculate and set the Authenticator for Accounting request packets
 */
void
CRadiusMessage::calcAccountingRequestAuthenticator( const char* secret )
{
    // See RFC 2866 Section 3
    // MD5( Code + ID + Length + 16 zero octets + Attributes + Secret )

    ACE_DEBUG ((LM_DEBUG,"Calculate Accounting Request Authenticator\n"));

    uint32_t msgLen = ntohs( myData.msgHdr.length );

    assert( (msgLen + strlen(secret)) <= RadiusMaxPacketSize );

    ::memset( myData.msgHdr.authenticator, 0, RadiusAuthenticatorLength );

    // Load secret behind message
    ::memcpy( myData.buffer + msgLen, secret, strlen(secret) );

    // Calculate
    calcMD5( myRequestAuthenticator, myData.buffer, msgLen + strlen(secret) );

    // Set Authenticator in request packet
    memcpy( myData.msgHdr.authenticator,
            myRequestAuthenticator,
            RadiusAuthenticatorLength );
}

void
CRadiusMessage::calcMD5( uint8_t *digest,
                        const uint8_t *buf,
                        const uint32_t bufLen )
{
    ACE_DEBUG ((LM_DEBUG,"Calculate MD5\n"));

    MD5_CTX context;
    MD5_Init( &context );
    MD5_Update( &context, buf, bufLen );
    MD5_Final( digest, &context );
}

//Analytic attribute
CRadiusData
CRadiusMessage::encodeAttributes() const
{
    CRadiusData msg;

    for( RadiusAttrIter itr = myAttributes.begin();
         itr != myAttributes.end();
         itr++ )
    {
        msg += (*itr).encode();
    }
    return msg;
}

//Get CRadiusAttribute instance
int CRadiusMessage::get( const RadiusAttributeType t, CRadiusAttribute& attr) const
{
    for ( RadiusAttrIter itr = myAttributes.begin();
          itr != myAttributes.end();
          itr++ )
    {
        if( itr->type() == t )
        {
            // Found one
            attr= *itr;
            return 0;
        }
    }

    return -1;
}

//Get All RadiusAttribute Instance
std::list<CRadiusAttribute >
CRadiusMessage::getAll( const RadiusAttributeType t ) const
{
    std::list<CRadiusAttribute> attrList;

    for ( RadiusAttrIter itr = myAttributes.begin();
          itr != myAttributes.end();
          itr++ )
    {
        if( itr->type() == t )
        {
            attrList.push_back( *itr );
        }
    }
    return attrList;
}


// Get a human readable representation of the message
std::string
CRadiusMessage::verbose()
{
    return std::string( hexDump() + headerDump() + attributesVerbose() );
}

// Raw message in hex for debugging
std::string
CRadiusMessage::hexDump()
{
    std::string s;        // Complete output (all lines)
    std::string a;        // "Ascii" portion of a line
    uint8_t temp;   // One octet
    uint8_t hi;     // High hex digit of an octet
    uint8_t low;    // Low hex digit of an octet
    char buf[4];     // Output buffer for one octet
    char cntBuf[6];  // Output buffer for counter at the beginning of a line
    int msgLen = ntohs(myData.msgHdr.length);

    int i;
    for( i = 0; i < msgLen; i++ )
    {
        if( 0 == (i % 16) ) // 16 in a line
        {
            snprintf( cntBuf, 6, "%04x ", i );
            s += a + "\n" + cntBuf;
            a = "   ";
        }
        temp = myData.buffer[i];
        hi  = (temp & 0xf0) / 16;
        low = (temp & 0xf);
        snprintf( buf, 4, " %x%x", hi, low );
        if( temp < 0x20 || temp > 0x7E)
        {
            a += ".";
        }
        else
        {
            a += reinterpret_cast<u_char>(temp);
        }
        s += buf;
    }
    // Last line only:
    // Pad with ' ' to make the "ascii" interpreted part line up with
    //   the previous line
    int j = i % 16;
    std::string b;    // Blanks
    if( j )
    {
        b = std::string( (16-j)*3, ' ' );
    }
    s += b + a + "\n";
    return s;
}

// Get a human readable representation of the message header
std::string
CRadiusMessage::headerDump()
{
    std::string codeStr;
    std::list<CRadiusAttribute>::iterator itr = myAttributes.begin(); 
    switch( myData.msgHdr.code )
    {
        case RP_ACCESS_REQUEST:
        {
            codeStr = "Access-Request (1)";
            break;
        }
        case RP_ACCESS_ACCEPT:
        {
            codeStr = "Access-Accept (2)";
            break;
        }
        case RP_ACCESS_REJECT:
        {
            codeStr = "Access-Reject (3)";
            break;
        }
        case RP_ACCOUNTING_REQUEST:
        {
            codeStr = "Accounting-Request (4)";
            break;
        }
        case RP_ACCOUNTING_RESPONSE:
        {
            codeStr = "Accounting-Response (5)";
            break;
        }
        case RP_ACCESS_CHALLENGE:
        {
            codeStr = "Access-Challenge (11)";
            break;
        }
        default:
        {
            codeStr = "Unknown (" + itr->itos(myData.msgHdr.code) + ")";
        }
    }
    return std::string( "\n  1 Code   = " + codeStr +
                        "\n  1 ID     = " + itr->itos( myData.msgHdr.identifier) +
                        "\n  2 Length = " + itr->itos( ntohs(myData.msgHdr.length) ) +
                        "\n 16 Authenticator\n" );
}

//Verbose attributes
std::string CRadiusMessage::attributesVerbose()
{
    std::string s = "\nAttributes:\n";

    for ( RadiusAttrIter itr = myAttributes.begin();
          itr != myAttributes.end();
          itr++ )
    {
        s += itr->verbose();
    }
    return s;
          
}
