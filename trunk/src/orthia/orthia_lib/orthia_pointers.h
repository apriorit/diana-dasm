#ifndef ORTHIA_POINTERS_H
#define ORTHIA_POINTERS_H

#include "windows.h"
#include "algorithm"

namespace orthia
{

class RefCountedBase
{
    long m_count;
public:
    RefCountedBase()
        :
            m_count(1)
    {
    }
    virtual ~RefCountedBase()
    {
    }
    virtual void OrthiaAddRef()
    {
        InterlockedIncrement(&m_count);
    }
    virtual void OrthiaRelease()
    {
        if (!InterlockedDecrement(&m_count))
        {
            delete this;
        }
    }
};

inline void Intrusive_AddRef(RefCountedBase * pObject)
{
    pObject->OrthiaAddRef();
}
inline void Intrusive_Release(RefCountedBase * pObject)
{
    pObject->OrthiaRelease();
}

#define ORTHIA_PREDECLARE(X)   X;  extern void Intrusive_AddRef(X * p);   extern void Intrusive_Release(X * p);

#define ORTHIA_DECLARE(X)   inline void Intrusive_AddRef(X * p) { p->OrthiaAddRef(); }  inline void Intrusive_Release(X * p) { p->OrthiaRelease(); }
template<class ObjectType>
class intrusive_ptr
{
    ObjectType * m_pObject;
    void AddRef()
    {
        if (m_pObject)
            Intrusive_AddRef(m_pObject);
    }
    void Release()
    {
        if (m_pObject)
           Intrusive_Release(m_pObject);
    }
public:
    intrusive_ptr()
        :
            m_pObject(0)
    {
    }
    explicit intrusive_ptr(ObjectType * pObject,
                           bool bAddRef = false)
        :
            m_pObject(pObject)
    {
        if (bAddRef)
        {
            AddRef();
        }
    }
    ~intrusive_ptr()
    {
        Release();
    }

    intrusive_ptr(const intrusive_ptr<ObjectType> & p)
        :
            m_pObject(p.m_pObject)
    {
        AddRef();
    }
    intrusive_ptr<ObjectType> & operator = (const intrusive_ptr<ObjectType> & p)
    {
        intrusive_ptr<ObjectType> temp(p);
        swap(temp);
        return *this;
    }

    template<class OtherType>
    intrusive_ptr(const intrusive_ptr<OtherType> & p)
        :
            m_pObject(p.m_pObject)
    {
        AddRef();
    }
    template<class OtherType>
    intrusive_ptr<ObjectType> & operator = (const intrusive_ptr<OtherType> & p)
    {
        intrusive_ptr<ObjectType> temp(p);
        swap(temp);
        return *this;
    }
    void swap(intrusive_ptr<ObjectType> & p)
    {
        std::swap(m_pObject, p.m_pObject);
    }
    ObjectType * operator -> ()
    {
        return m_pObject;
    }
    const ObjectType * operator -> () const 
    {
        return m_pObject;
    }
    ObjectType & operator * ()
    {
        return *m_pObject;
    }
    const ObjectType & operator *() const 
    {
        return *m_pObject;
    }
};

}
#endif