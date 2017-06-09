/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved. 
**********************************************************************/
#ifndef CRADIUSDATA_H
#define CRADIUSDATA_H

#include <cassert>
#include "radius.h"
#include "aceinclude.h"
#include <stdint.h>



/** Raw RADIUS data
     This is a simple but limited implementation with a static size
 */
class CRadiusData
{
public:
    CRadiusData( ) : myLength( 0 )
    {
        ::memset( &myData, 0, RadiusMaxPacketSize);
    }

    CRadiusData( const void *val, uint16_t len ) :
        myLength( len > RadiusMaxPacketSize ? RadiusMaxPacketSize : len )
    {
        assert( val );
        ::memcpy( &myData, val, myLength );
    }
    
    CRadiusData( const CRadiusData& rhs )
    {
        copyRhsToThis( rhs );
    }
    CRadiusData& operator=( const CRadiusData& rhs )
    {
        if( this != &rhs )
        {
            copyRhsToThis( rhs );
        }
        return *this;
    }
    void copyRhsToThis( const CRadiusData& rhs )
    {
        myLength = rhs.myLength;
        memcpy( &myData, rhs.myData, myLength);
    }

    CRadiusData& operator+=( const CRadiusData& rhs )
    {
        assert( this != &rhs );
        append( &rhs.myData, rhs.myLength );
        return *this;
    }
    int append( const void *val, const int len )
    {
        assert( val );

        if( (myLength + len) > RadiusMaxPacketSize )
        {
            return -1;
        }
        memcpy( myData + myLength, val, len );
        myLength += len;
        return 0;
    }
    
    ///
    ~CRadiusData() {}
    
    ///
    const uint8_t* data() const { return myData; }
    
    ///
    uint16_t length() const { return myLength; }
    
private:
    uint16_t   myLength;
    uint8_t    myData[RadiusMaxPacketSize];
};

#endif


