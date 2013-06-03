#ifndef ORTHIA_UTILS_H
#define ORTHIA_UTILS_H

#include "vector"
#include "stdexcept"
#include "sstream"
#include "algorithm"
#include "map"
#include "windows.h"

namespace orthia
{
typedef ULONG64 Address_type;

class CWin32Exception:public std::runtime_error
{
    ULONG m_errorCode;

    std::string ToErrText(const std::string & text, ULONG errorCode)
    {
        std::stringstream resStream;
        resStream<<text<<", code: "<<errorCode;
        return resStream.str();
    }
public:
    CWin32Exception(const std::string & text, ULONG errorCode)
        :
            std::runtime_error(ToErrText(text, errorCode)),
            m_errorCode(errorCode)
    {
    }


};

std::wstring ExpandVariable(const std::wstring & possibleVar);
std::wstring ToString(const std::string & str, UINT codePage = CP_ACP);
std::wstring ToString(Address_type address);

Address_type ToAddress(const std::wstring & sourceStr);
std::string ToAnsiString_Silent(const std::wstring & sourceStr);

template<class CharType>
struct CharTraits
{
};
template<>
struct CharTraits<char>
{
    static const char space = ' ';
};
template<>
struct CharTraits<wchar_t>
{
    static const wchar_t space = L' ';
};

template<class CharType>
std::basic_string<CharType> Trim(const std::basic_string<CharType> & str)
{
    typedef typename std::basic_string<CharType> String_type;
    typename String_type::const_iterator it = str.begin();
    typename String_type::const_iterator it_end = str.end();
    for(; it != it_end; ++it)
    {
        if (*it != CharTraits<CharType>::space)
            break;
    }
    if (it == it_end)
        return String_type(it, it_end);

    typename String_type::const_iterator it2 = it_end;
    --it2;
    for(; it2 != it; --it2)
    {
        if (*it2 != CharTraits<CharType>::space)
            break;
    }
    return String_type(it, ++it2);
}

template<class CharType>
std::basic_string<CharType> Trim(const CharType * str)
{
    std::basic_string<CharType> arg(str);
    return Trim(arg);
}

template<class CharType>
void Split(const std::basic_string<CharType> & sourceString, 
           std::vector<std::basic_string<CharType> > * pArgs, 
           CharType splitChar = CharTraits<CharType>::space)
{
    typedef typename std::basic_string<CharType> String_type;
    typename String_type::const_iterator it = sourceString.begin();
    typename String_type::const_iterator it_end = sourceString.end();
    for(; it != it_end; ++it)
    {
        
        for(; it != it_end; ++it)
        {
            if (*it != splitChar)
                break;
        }
        if (it == it_end)
            return;
        typename String_type::const_iterator wordStart = it++;
        for(; it != it_end; ++it)
        {
            if (*it == splitChar)
                break;
        }
        pArgs->push_back(String_type(wordStart, it));
    }
}


}
#endif 