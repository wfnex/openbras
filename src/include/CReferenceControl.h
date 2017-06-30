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

#ifndef CMREFERENCECONTROL_H
#define CMREFERENCECONTROL_H

#include "aceinclude.h"
#include <iostream>
class IReferenceControl
{
public:
    virtual uint32_t AddReference() = 0;
    virtual uint32_t ReleaseReference() = 0;

protected:
    virtual ~IReferenceControl() {}
};


class CReferenceControl 
{
public:
    CReferenceControl()
        :m_Atomic(0)
    {
    }

    virtual ~CReferenceControl()
    {
    }

    uint32_t AddReference()
    {
        mutex_.acquire ();
        uint32_t dwRef= ++m_Atomic;
        mutex_.release ();
        //ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(CReferenceControl::AddReference dwRef=%d\n"),dwRef));
        return dwRef;
    }

    uint32_t ReleaseReference()
    {
        mutex_.acquire ();
        uint32_t dwRef = --m_Atomic;
        mutex_.release ();
        //ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(CReferenceControl::AddReference dwRef=%d\n"),dwRef));
        if (dwRef == 0) 
        {
            ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(CReferenceControl::AddReference OnReferenceDestory dwRef=%d\n"),dwRef));
            OnReferenceDestory();
        }
        return dwRef;
    }

    uint32_t GetReference()
    {
        return m_Atomic;
    }

protected:
    virtual void OnReferenceDestory()
    {
        delete this;
    }
    
    ACE_Thread_Mutex mutex_;

    uint32_t m_Atomic;
};


template <class T> class CCmAutoPtr
{
public:
    CCmAutoPtr(T *aPtr = NULL) 
    : m_pRawPtr(aPtr)
    {
        if (m_pRawPtr)
            m_pRawPtr->AddReference();
    }

    CCmAutoPtr(const CCmAutoPtr& aAutoPtr) 
    : m_pRawPtr(aAutoPtr.m_pRawPtr)
    {
        if (m_pRawPtr)
        m_pRawPtr->AddReference();
    }

    ~CCmAutoPtr() 
    {
        if (m_pRawPtr)
            m_pRawPtr->ReleaseReference();
    }

    CCmAutoPtr& operator = (const CCmAutoPtr& aAutoPtr)
    {
        return (*this = aAutoPtr.m_pRawPtr);
    }

    bool operator< (const CCmAutoPtr& aAutoPtr) const
    {
        if ((m_pRawPtr == NULL)||(aAutoPtr.m_pRawPtr == NULL))
        {
            return false;
        }
        return *m_pRawPtr<*aAutoPtr.m_pRawPtr;
    }

    CCmAutoPtr& operator = (T* aPtr)
    {
        if (m_pRawPtr == aPtr)
            return *this;

        if (aPtr)
            aPtr->AddReference();
        if (m_pRawPtr)
            m_pRawPtr->ReleaseReference();
        m_pRawPtr = aPtr;
        return *this;
    }

    operator void* () const 
    {
        return m_pRawPtr;
    }

    T* operator -> () const 
    {
        return m_pRawPtr;
    }

    T* Get() const 
    {
        return m_pRawPtr;
    }

    T* ParaIn() const 
    {
        return m_pRawPtr;
    }

    T*& ParaOut() 
    {
        if (m_pRawPtr) {
            m_pRawPtr->ReleaseReference();
            m_pRawPtr = NULL;
        }
        return static_cast<T*&>(m_pRawPtr);
    }

    T*& ParaInOut() 
    {
        return static_cast<T*&>(m_pRawPtr);
    }

    T& operator * () const 
    {
        return *m_pRawPtr;
    }

private:
    T *m_pRawPtr;
};

#endif // !CMREFERENCECONTROL_H

