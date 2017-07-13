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


