/***********************************************************************
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved. 
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

