/***********************************************************************
 * Copyright (C) 2013, Nanjing WFN Technology Co., Ltd 
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

