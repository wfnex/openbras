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


#ifndef CPPPOEPACKETHANDLE_H
#define CPPPOEPACKETHANDLE_H

#include "aceinclude.h"
#include "CReferenceControl.h"
#include "STDInclude.h"
#include "pppoe.h"
#include "BaseDefines.h"

class CPPPOEPacketHandler : public ACE_Event_Handler 
{
public:
    CPPPOEPacketHandler();
    virtual ~CPPPOEPacketHandler();
    
    SWORD32 OpenX (WORD16 type);
    SWORD32 SendPacket(const CHAR *pkt, SWORD32 size);
    SWORD32 BindInterface(CHAR const *ifname, WORD16 type);

    virtual SWORD32 ProcessPacket(const CHAR *packet, ssize_t size) = 0;

    // For class ACE_Event_Handler
    virtual int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE);
    virtual ACE_HANDLE get_handle (void) const;
    virtual int handle_close (ACE_HANDLE handle, ACE_Reactor_Mask close_mask);
    
private:
    ACE_HANDLE m_handler;
};

#endif // CPPPOEPACKETHANDLE_H

