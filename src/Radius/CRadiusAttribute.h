/***********************************************************************
 * Copyright (C) 2014, Nanjing WFNEX Technology Co., Ltd 
**********************************************************************/
#ifndef CRADIUSATTRIBUTE_H
#define CRADIUSATTRIBUTE_H
#include "aceinclude.h"
#include <stdint.h>

#include "CRadiusData.hxx"
#include "radius.h"

/** RADIUS Attribute as defined in RFC 2865 Section 5.
    <pre>
    Attribute Format
    
    0                   1                   2
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
   |     Type      |    Length     |  Value ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
   
   
    Vendor-Specific Attribute format
    
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |  Length       |            Vendor-Id
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        Vendor-Id (cont)           |  String...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
   </pre>
 */
class CRadiusAttribute
{
public:

    /// Create a RADIUS Attribute
    CRadiusAttribute( const RadiusAttributeType type,
                     const uint8_t* value,
                     const uint8_t length,
                     const uint32_t vendorId = 0);
    
    ///

    /// Create a RADIUS Attribute
    CRadiusAttribute( const RadiusAttributeType type,
                     const CRadiusData& value,
                     const uint32_t vendorId = 0 );
    
    ///

    CRadiusAttribute( const CRadiusAttribute& rhs )
    {
        copyRhsToThis( rhs );
    }

    ///
    CRadiusAttribute& operator=( const CRadiusAttribute& rhs )
    {
        if( this != &rhs )
        {
            copyRhsToThis( rhs );
        }
        return *this;
    }

    ///
    void copyRhsToThis( const CRadiusAttribute& rhs )
    {
        myType = rhs.myType;
        myValue = rhs.myValue;
        myVendorId = rhs.myVendorId;
    }

    /// Get Attribute type
    RadiusAttributeType type() const
    {
        return myType;
    }

    /// Check if this is a Vendor Specific Attribute (VSA)
    bool isVsa() const
    {
        return myType == RA_VENDOR_SPECIFIC;
    }

    /// Get the vendor ID, 0 also indicates this is not a VSA
    uint32_t vendorId() const
    {
        return myVendorId;
    }

    /// Get the total length of this Attribute
    uint16_t length() const
    {
        // 2 octets for Type and Length
        return myValue.length() + 2 + ((myVendorId == 0) ? 0 : 4);
    }

    /// Encode the Attribute
    // TODO: return a RawMessage instead to enhance performance?
    CRadiusData encode() const;

    /// Get the Attribute value field
    const CRadiusData& value() const
    {
        return myValue;
    }

    /// Get a human readable representation of this attribute
    std::string verbose() const;
    std::string itos(uint32_t a);
private:


    /// RADIUS Attribute Type
    RadiusAttributeType  myType;

    /** Attribute Value
        Multiple subattributes may be encoded within a single
        Vendor-Specific attribute. They should further be decoded
        by a vendor-specific application.
     */
    CRadiusData           myValue;

    /// VSA Vendor-Id, 0 for non-VSA Attributes
    uint32_t            myVendorId;
};

#endif//CRADIUSATTRIBUTE_H

