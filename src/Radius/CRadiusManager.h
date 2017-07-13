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

#ifndef CRADIUSMANAGER_H
#define CRADIUSMANAGER_H
#include "aceinclude.h"
#include <string>
#include <unordered_map>
#include "CRadiusConfig.h"

class CRadiusScheme;
class CRadiusManager
{
public:
    CRadiusManager();
    ~CRadiusManager();
    int Init();
    int CreateRadiusScheme(std::string &name,CRadiusScheme *&radius);
    int DestroyRadiusScheme(std::string &name);
    int FindRadiusScheme(std::string &name,CRadiusScheme *&radius);
    CRadiusConfig &GetConfig()
    {
        return m_config;
    }
    int TestRadiusAccess();
    int TestRadiusAcct();
private:
    ACE_Thread_Mutex m_mutex;
    CRadiusConfig m_config;
    std::unordered_map<std::string, CRadiusScheme *> m_schemes;
};


#endif//CPORTALMANAGER_H

