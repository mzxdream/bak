#ifndef _M_STRING_H_
#define _M_STRING_H_

#include <string>
#include <strings.h>
#include <sstream>

template<typename O>
void _MConcatBase(O &os)
{
}

template<typename O, typename T, typename ...Args>
void _MConcatBase(O &os, const T &val, const Args& ...args)
{
    os << val;
    _MConcatBase(os, args...);
}

template<typename ...Args>
std::string MConcat(const Args& ...args)
{
    std::stringstream ss;
    _MConcatBase(ss, args...);
    return ss.str();
}

class MString
{
public:
    static int CompareNoCase(const std::string &s1, const std::string &s2)
    {
        return strcasecmp(s1.c_str(), s2.c_str());
    }
    static std::string TrimLeft(const std::string &str, const std::string &trim)
    {
        size_t pos = str.find_first_not_of(trim);
        if (pos != std::string::npos)
        {
            return str.substr(pos);
        }
        return "";
    }
    static std::string TrimRight(const std::string &str, const std::string &trim)
    {
        size_t pos = str.find_last_not_of(trim);
        if (pos != std::string::npos)
        {
            return str.substr(0, pos+1);
        }
        return "";
    }
    static std::string Trim(const std::string &str, const std::string &trim)
    {
        return TrimRight(TrimLeft(str, trim), trim);
    }
};

#endif
