#ifndef _M_DB_COMMAND_H_
#define _M_DB_COMMAND_H_

#include <string>
#include <util/m_type_define.h>
#include <db/m_db_common.h>
#include <utility>
#include <util/m_errno.h>

class MDbCommand
{
public:
    MDbCommand() = default;
    virtual ~MDbCommand() = default;
    MDbCommand(const MDbCommand &) = delete;
    MDbCommand& operator=(const MDbCommand &) = delete;
public:
    MError Prepair(const std::string &command)
    {
        return DoPrepair(command);
    }
    template <typename ...Args>
    std::pair<unsigned, MError> ExecuteNonQuery(const Args& ...args)
    {
        MError err = DoBeforeAddParams();
        if (err == MError::No)
        {
            err = AddParams(args...);
            if (err == MError::No)
            {
                return DoExecuteNonQuery();
            }
        }
        return std::pair<unsigned, MError>(0, err);
    }
    template <typename ...Args>
    MError ExecuteReader(const Args& ...args)
    {
        MError err = DoBeforeAddParams();
        if (err == MError::No)
        {
            err = AddParams(args...);
            if (err == MError::No)
            {
                return DoExecuteReader();
            }
        }
        return err;
    }
    template <typename ...Args>
    MError NextRecord(Args* ...args)
    {
        MError err = DoGotoNextRecord();
        if (err == MError::No)
        {
            return GetParams(args...);
        }
        return err;
    }
    MError NextResult()
    {
        return DoGotoNextResult();
    }
private:
    virtual MError DoPrepair(const std::string &command) = 0;
    virtual MError DoBeforeAddParams() = 0;
    virtual MError DoGotoNextRecord() = 0;
    virtual MError DoGotoNextResult() = 0;
    virtual std::pair<unsigned, MError> DoExecuteNonQuery() = 0;
    virtual MError DoExecuteReader() = 0;

    virtual MError DoAddParam(const int8_t *p_param) = 0;
    virtual MError DoAddParam(const uint8_t *p_param) = 0;
    virtual MError DoAddParam(const int16_t *p_param) = 0;
    virtual MError DoAddParam(const uint16_t *p_param) = 0;
    virtual MError DoAddParam(const int32_t *p_param) = 0;
    virtual MError DoAddParam(const uint32_t *p_param) = 0;
    virtual MError DoAddParam(const int64_t *p_param) = 0;
    virtual MError DoAddParam(const uint64_t *p_param) = 0;
    virtual MError DoAddParam(const float *p_param) = 0;
    virtual MError DoAddParam(const double *p_param) = 0;
    virtual MError DoAddParam(const std::string *p_param) = 0;
    virtual MError DoAddParam(const MBlob *p_param) = 0;

    virtual MError DoGetParam(int8_t *p_param) = 0;
    virtual MError DoGetParam(uint8_t *p_param) = 0;
    virtual MError DoGetParam(int16_t *p_param) = 0;
    virtual MError DoGetParam(uint16_t *p_param) = 0;
    virtual MError DoGetParam(int32_t *p_param) = 0;
    virtual MError DoGetParam(uint32_t *p_param) = 0;
    virtual MError DoGetParam(int64_t *p_param) = 0;
    virtual MError DoGetParam(uint64_t *p_param) = 0;
    virtual MError DoGetParam(float *p_param) = 0;
    virtual MError DoGetParam(double *p_param) = 0;
    virtual MError DoGetParam(std::string *p_param) = 0;
    virtual MError DoGetParam(MBlob *p_param) = 0;
private:
    MError AddParams()
    {
        return MError::No;
    }
    template <typename T, typename ...Args>
    MError AddParams(const T &arg, const Args& ...args)
    {
        MError err = DoAddParam(&arg);
        if (err == MError::No)
        {
            return AddParams(args...);
        }
        return err;
    }
    MError GetParams()
    {
        return MError::No;
    }
    template <typename T, typename ...Args>
    MError GetParams(T *arg, Args* ...args)
    {
        MError err = DoGetParam(arg);
        if (err == MError::No)
        {
            return GetParams(args...);
        }
        return err;
    }
};

#endif
