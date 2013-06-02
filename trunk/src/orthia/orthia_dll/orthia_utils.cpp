#include "orthia_utils.h"
#pragma warning(disable:4996)
namespace orthia
{
std::wstring ExpandVariable(const std::wstring & possibleVar)
{
    std::vector<wchar_t> buf(1024);
    std::wstring prevResult;
    const int tryCount = 10;
    for(int i = 0; i < tryCount; ++i)
    {
        DWORD size = ExpandEnvironmentStringsW(possibleVar.c_str(), &buf.front(), (DWORD)buf.size());
        if (!size)
        {
            return possibleVar;
        }
        if (size > (DWORD)buf.size())
        {
            buf.resize(size*2);
            continue;
        }
        std::wstring result(&buf.front());
        if (result.find_first_of(L'%') == result.npos)
            return result;
        if (prevResult == result)
            return result;
        prevResult = result;
    }
    return prevResult;
}

Address_type ToAddress(const std::wstring & sourceStr)
{
    wchar_t * pEndStr = const_cast<wchar_t*>(sourceStr.c_str() + sourceStr.size());
    wchar_t * pResStr = pEndStr;
    Address_type res = _wcstoui64(sourceStr.c_str(), &pEndStr, 16);
    if (pEndStr != pEndStr)
    {
        throw std::runtime_error("Invalid address: " + ToAnsiString_Silent(sourceStr));
    }
    return res;
}
std::wstring ToString(Address_type address)
{
    wchar_t buf[64];
    _ui64tow(address, buf, 16);
    return buf;
}
std::wstring ToString(const std::string & str, UINT codePage)
{
    if(str.empty())
    {
        return std::wstring();
    }
    std::vector<wchar_t> tmp;
    tmp.resize(str.size() + 1);
    for(;;)
    {
        int size = MultiByteToWideChar(codePage,
                                       0,
                                       str.c_str(),
                                       (int)str.size(), 
                                       &tmp.front(), 
                                       (int)tmp.size());
        if(size)
        {
            return std::wstring(tmp.begin(), tmp.begin()+size);   
        }
        DWORD error = ::GetLastError();
        if(error != ERROR_INSUFFICIENT_BUFFER)
        {
            throw CWin32Exception("Can't convert: [" + str + "]", error);
        }
        size = MultiByteToWideChar(codePage, 
                                   0, 
                                   str.c_str(), 
                                   (int)str.size(), 
                                   0, 
                                   0);
        if(!size)
        {
            throw CWin32Exception("Can't convert: [" + str + "]", ::GetLastError());
        }
        tmp.resize(size*2);
    }
}

std::string ToAnsiString_Silent(const std::wstring & str)
{
    if(str.empty())
    {
        return std::string();
    }
    std::vector<char> tmp;
    tmp.resize(str.size() + 1);
    for(;;)
    {
        int size = WideCharToMultiByte(CP_ACP,
                                       0,
                                       str.c_str(),
                                       (int)str.size(), 
                                       &tmp.front(), 
                                       (int)tmp.size(),
                                       0,
                                       0);
        if(size)
        {
            return std::string(tmp.begin(), tmp.begin()+size);   
        }
        DWORD error = ::GetLastError();
        if(error != ERROR_INSUFFICIENT_BUFFER)
        {
            return std::string();
        }
        size = WideCharToMultiByte(CP_ACP, 
                                   0, 
                                   str.c_str(), 
                                   (int)str.size(), 
                                   0, 
                                   0,
                                   0,
                                   0);
        if(!size)
        {
            return std::string();
        }
        tmp.resize(size*2);
    }
}

}