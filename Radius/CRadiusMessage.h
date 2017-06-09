/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved. 
**********************************************************************/
#ifndef CRADIUSMESSAGE_H
#define CRADIUSMESSAGE_H

#include "aceinclude.h"
#include <stdint.h>

#include <string>
#include <list>

#include "CRadiusData.hxx"
#include "radius.h"
#include "CRadiusAttribute.hxx"


///
typedef std::list<CRadiusAttribute>::const_iterator RadiusAttrIter;


/** RADIUS Packet as defined in RFC 2865 Section 3.
    <pre>
    Data Format
    
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
   </pre>
 */
class CRadiusMessage
{
    public:
    
        /** Create a RADIUS Message with type, no Attriubte yet
            Useful for composing a request
         */
        CRadiusMessage( const RadiusPacketType code );
        
        /** Create a RADIUS Message with type and an Authenticator
            No Attriubte yet
            Useful for composing a response
         */
        CRadiusMessage( const RadiusPacketType code,
                       const uint8_t requestIdentifer,
                       const uint8_t *requestAuth );
        
        /** Create a RADIUS Message from data

            Use when a message is received

            Request Authenticator is verified automatically. However,
              RadiusMessage::verifyResponseAuthenticator(auth,secret)
              should be called later for Responses.

            If an exception is thrown, the message should be handled
            (discarded or rejected) according to the RFCs.
         */
        CRadiusMessage( const CRadiusData data, const char* secret );

        ///
        CRadiusMessage( const CRadiusMessage& rhs )
        {
            copyRhsToThis( rhs );
        }

        ///
        CRadiusMessage& operator=( const CRadiusMessage& rhs )
        {
            if( this != &rhs )
            {
                copyRhsToThis( rhs );
            }
            return *this;
        }

        ///
        void copyRhsToThis( const CRadiusMessage& rhs )
        {
            memcpy( &myRequestAuthenticator,
                    rhs.myRequestAuthenticator,
                    RadiusAuthenticatorLength );
            myAttributes = rhs.myAttributes;
            myData = rhs.myData;
        }

        ///
        RadiusPacketType type()
        {
            return myData.msgHdr.code;
        }

        /// Encode all Attributes in this list
        CRadiusData encodeAttributes() const;

        /// TODO: throw exception instead
        bool add( const CRadiusAttribute& attr );

        /// Get the number of attributes
        uint32_t attributeCount() const
        {
            return myAttributes.size();
        }

        /**
           Get the first instance of Attribute t from the list
           Use getAll( RadiusAttributeType ) to retrieve multiple
             attributes with the same type 
         */
        int get( const RadiusAttributeType t, const CRadiusAttribute& attr) const;

        /**
           Get the list of all attributes of type attrType
         */
        std::list<CRadiusAttribute > getAll( const RadiusAttributeType attrType ) const;

        /**
            For RADIUS requests only
            Must be called before Authenticator is calculated
         */
        void setIdentifier( const uint8_t& id )
        {
            myData.msgHdr.identifier = id;
        }

        ///
        uint8_t getIdentifier() const
        {
            return myData.msgHdr.identifier;
        }

        /** Calculate and set the Authenticator
            Must be called right before the packet is sent
         */
        void calcAuthenticator( const char* secret );

        /** Get the Authenticator
         */
        const uint8_t* getAuthenticator() const
        {
            return myRequestAuthenticator;
        }

        /** Verify a response with the original request's Authenticator
         */
        bool verifyResponseAuthenticator( const uint8_t *reqAuth,
                                          const char* secret );

        /** Get raw message data for sending
         */
        const RawMessage& data() const
        {
            return myData;
        }

        /// Get a human readable representation of all attributes
        std::string verbose() const;

        /// Raw message in hex for debugging
        std::string hexDump() const;

        /// Get a human readable representation of the message header
        std::string headerDump() const;

        /// Get a human readable representation of the message
        std::string attributesVerbose() const;

    private:

        /// Disable default constructor
        CRadiusMessage()
        {
        }

        /** Decode Attributes
            If an Attribute is received in an Access-Request but an
            exception is thrown, an Access-Reject SHOULD be transmitted.
            If an Attribute is received in an Access-Accept, Access-Reject
            or Access-Challenge packet and an exception is thrown, the
            packet must either be treated as an Access-Reject or else
            silently discarded.
         */
        void decodeAttributes( const char* secret );

        ///
        bool verifyAccountingRequestAuthenticator( const char* secret );

        /** Encode User-Password in Access-Request
              oldPassword length must be a multiple of 16 octets
         */
        void encodeUserPassword( uint8_t* newPassword,
                                 const char* secret,
                                 const CRadiusData& oldPassword );

        /** Decode User-Password in Access-Request
              oldPassword length must be a multiple of 16 octets
         */
        void decodeUserPassword( uint8_t* newPassword,
                                 const char* secret,
                                 const CRadiusData& oldPassword );

        /// Hide User-Password in Access-Request
        void hideUserPassword( const char* secret );

        /// Calculate and set myRequestAuthenticator for Access-Request packets
        void calcAccessRequestAuthenticator();

        /** Calculate and set the Authenticator for response (Access-Accept,
            Access-Reject, Access-Challenge or Accounting Response) packets
         */
        void calcResponseAuthenticator( const char* secret );

        /// Calculate and set the Authenticator for Accounting request packets
        void calcAccountingRequestAuthenticator( const char* secret );

        void calcMD5( uint8_t *digest,
                      const uint8_t *buf,
                      const u_int32_t bufLen );

        /// Request Authenticator
        uint8_t myRequestAuthenticator[ RadiusAuthenticatorLength ];

        // TODO: may need a flag to keep myAttrList and myData in sync

        /// Attribute list
        std::list<CRadiusAttribute > myAttributes;

        /// RADIUS packet (in network byte order)
        RawMessage myData;
};

// Local Variables:
// mode:c++
// c-file-style:"bsd"
// c-basic-offset:4
// c-file-offsets:((inclass . ++))
// indent-tabs-mode:nil
// End:

#endif
