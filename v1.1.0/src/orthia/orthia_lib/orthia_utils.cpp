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
            return std::wstring();
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

std::string ToAnsiString_Silent(const std::wstring & str,
                                ULONG codePage)
{
    if(str.empty())
    {
        return std::string();
    }
    std::vector<char> tmp;
    tmp.resize(str.size() + 1);
    for(;;)
    {
        int size = WideCharToMultiByte(codePage,
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
        size = WideCharToMultiByte(codePage, 
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

static wchar_t g_hexChars[] = L"0123456789abcdef";

std::wstring ToHexString(const char * pArray, 
                         size_t size)
{
    std::wstring res;
    res.reserve(size*2);
    for(size_t i = 0; i < size; ++i)
    {
        unsigned char item = (unsigned char )(pArray[i]);
        res.push_back(g_hexChars[item >> 4]);
        res.push_back(g_hexChars[item &0xF]);
    }
    return res;
}

bool IsFileExist(const std::wstring & fullFileName)
{
    return ::GetFileAttributes(fullFileName.c_str()) != INVALID_FILE_ATTRIBUTES;
}
Address_type GetSizeOfFile(const std::wstring & fullFileName)
{
    HANDLE hFile = CreateFile(fullFileName.c_str(), 
                              GENERIC_READ, 
                              FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
                              0, 
                              OPEN_EXISTING, 
                              0, 
                              0);
    if (hFile == INVALID_HANDLE_VALUE)
        ORTHIA_THROW_WIN32("Can't open file: "<<orthia::ToAnsiString_Silent(fullFileName));

    CHandleGuard guard(hFile);

    LARGE_INTEGER size;
    size.QuadPart = 0;
    if (!::GetFileSizeEx(hFile, &size))
    {
        ORTHIA_THROW_WIN32("Can't get file size: "<<orthia::ToAnsiString_Silent(fullFileName));
    }
    return size.QuadPart;
}

void CreateAllDirectoriesForFile(const std::wstring & fullFileName)
{
    if (fullFileName.size() > 3)
    {
        // skip UNC paths and 
        if (wcsncmp(fullFileName.c_str(), L"\\\\", 2) == 0)
        {
            if (fullFileName[2] != L'?' &&
                fullFileName[2] != L'.') 
            {
                return;
            }
        }
    }
    for(std::wstring::const_iterator it = fullFileName.begin(), it_end = fullFileName.end();
        it != it_end;
        ++it)
    {
        if (*it == L'/' || *it == L'\\')
        {
            std::wstring path(fullFileName.begin(), it+1);
            CreateDirectory(path.c_str(), 0);
        }
    }
}


CDll::CDll(const std::wstring & name)
    :
        m_hLib(0)
{
    Reset(name);
}
CDll::~CDll()
{
    Reset(0);
}
HMODULE CDll::GetBase()
{
    return m_hLib;
}
void CDll::Reset(const std::wstring & name)
{
    Reset(name.empty()?0:name.c_str());
}

void CDll::Reset(const wchar_t * pName)
{
    if (m_hLib)
    {
        FreeLibrary(m_hLib);
        m_hLib = 0;
    }
    if (!pName)
        return;

    m_hLib = LoadLibraryW(pName);
    if (!m_hLib)
    {
        ORTHIA_THROW_WIN32("Can't load: "<<orthia::ToAnsiString_Silent(pName)<<", code: "<<orthia____code);
    }
}
FARPROC CDll::QueryFunctionRaw(const char * pFunctionName, 
                              bool bSilent)
{
    FARPROC pProc = GetProcAddress(m_hLib, pFunctionName);
    if (bSilent)
        return pProc;
    if (!pProc)
    {
        ORTHIA_THROW_WIN32("Can't find: "<<pFunctionName<<", code: "<<orthia____code);
    }
    return pProc;
}

}