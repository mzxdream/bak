#ifndef _M_DB_COMMON_H_
#define _M_DB_COMMON_H_

#include <util/m_string.h>
#include <util/m_convert.h>
#include <util/m_logger.h>

class MDbConnString
{
public:
    template<typename T>
    static bool GetParamValue(const std::string &str, const std::string &param, T &val, const std::string &op="=", const std::string &sep=";", const std::string &trim=" ")
    {
        std::string val_str;
        if (!GetParamValue(str, param, val_str, op, sep, trim))
        {
            //MLOG(MGetLibLogger(), MERR, "GetParamValue failed");
            return false;
        }
        if (!MConvertTo(val_str, val))
        {
            MLOG(MGetLibLogger(), MERR, "convert ", val_str, " failed");
            return false;
        }
        return true;
    }
    static bool GetParamValue(const std::string &str, const std::string &param, std::string &val, const std::string &op="=", const std::string &sep=";", const std::string &trim=" ")
    {
        size_t pos_start = str.find(param);
        if (pos_start == std::string::npos)
        {
            //MLOG(MGetLibLogger(), MERR, "can't find \"", param, "\" from \"", str, "\"");
            return false;
        }
        pos_start += param.size();
        pos_start = str.find(op, pos_start);
        if (pos_start == std::string::npos)
        {
            MLOG(MGetLibLogger(), MERR, "can't find \"", op, "\" from \"", str, "\"");
            return false;
        }
        pos_start += op.size();

        size_t pos_end = str.find(sep, pos_start);
        if (pos_end == std::string::npos)
        {
            val = MString::Trim(str.substr(pos_start), trim);
        }
        else
        {
            val = MString::Trim(str.substr(pos_start, pos_end-pos_start), trim);
        }
        return true;
    }
};

#endif
