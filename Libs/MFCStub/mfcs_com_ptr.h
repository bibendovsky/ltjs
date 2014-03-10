#ifndef __MFCS_COM_PTR_H__
#define __MFCS_COM_PTR_H__


#include <cstddef>
#include <unknwn.h>


template<class T>
class CComPtr {
public:
    CComPtr() throw()
    {
        p = NULL;
    }

    CComPtr(
        T* p2) throw() :
            p(p2)
    {
        if (p != NULL)
            p->AddRef();
    }

    CComPtr(
        const CComPtr<T>& p2) throw() :
            p(p2)
    {
        if (p != NULL)
            p->AddRef();
    }

    ~CComPtr() throw()
    {
        if (p != NULL)
            p->Release();
    }

    operator T*() const throw()
    {
        return p;
    }

    T& operator*() const throw()
    {
        return *p;
    }

    T** operator&() throw()
    {
        return &p;
    }

    T* operator->() const throw()
    {
        return p;
    }

    T* operator=(
        T* p2) throw()
    {
        return assign(p, p2);
    }

    T* operator=(
        const CComPtr<T>& p2) throw()
    {
        return assign(p, p2);
    }

    bool operator!() const throw()
    {
        return p == NULL;
    }

    void Release() throw()
    {
        IUnknown* temp = p;

        if (temp != NULL) {
            p = NULL;
            temp->Release();
        }
    }

    HRESULT CoCreateInstance(
        LPCOLESTR szProgID,
        LPUNKNOWN pUnkOuter = NULL,
        DWORD dwClsContext = CLSCTX_ALL) throw()
    {
        CLSID clsid;

        HRESULT result = CLSIDFromProgID(szProgID, &clsid);

        if (SUCCEEDED(result)) {
            result = ::CoCreateInstance(
                clsid,
                pUnkOuter,
                dwClsContext,
                __uuidof(T),
                reinterpret_cast<LPVOID*>(&p));
        }

        return result;
    }

    HRESULT CoCreateInstance(
        REFCLSID rclsid,
        LPUNKNOWN pUnkOuter = NULL,
        DWORD dwClsContext = CLSCTX_ALL) throw()
    {
        return ::CoCreateInstance(
            rclsid,
            pUnkOuter,
            dwClsContext,
            __uuidof(T),
            reinterpret_cast<LPVOID*>(&p));
    }

    template<class C>
    HRESULT QueryInterface(
        C** p2) const throw()
    {
        return p->QueryInterface(__uuidof(C), reinterpret_cast<LPVOID*>(p2));
    }

    T* p;

private:
    T* assign(
        T* src,
        T*& dst)
    {
        if (src != NULL)
            src->AddRef();

        if (dst != NULL)
            dst->Release();

        dst = src;
        return src;
    }
}; // class CComPtr


#endif // __MFCS_COM_PTR_H__
