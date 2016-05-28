#ifndef _M_REDIS_COMMAND_H_
#define _M_REDIS_COMMAND_H_

#include <util/m_convert.h>
#include <util/m_type_define.h>
#include <db/m_db_command.h>
#include <hiredis.h>
#include <string>
#include <vector>
#include <cstring>

class MRedisConnection;

class MRedisCommand
    :public MDbCommand
{
public:
    explicit MRedisCommand(MRedisConnection &conn);
    virtual ~MRedisCommand();
    MRedisCommand(const MRedisCommand &) = delete;
    MRedisCommand& operator=(const MRedisCommand &) = delete;
private:
    virtual MError DoPrepair(const std::string &command) override;
    virtual MError DoBeforeAddParams() override;
    virtual MError DoGotoNextRecord() override;
    virtual MError DoGotoNextResult() override;
    virtual std::pair<unsigned, MError> DoExecuteNonQuery() override;
    virtual MError DoExecuteReader() override;

    virtual MError DoAddParam(const int8_t *p_param) override;
    virtual MError DoAddParam(const uint8_t *p_param) override;
    virtual MError DoAddParam(const int16_t *p_param) override;
    virtual MError DoAddParam(const uint16_t *p_param) override;
    virtual MError DoAddParam(const int32_t *p_param) override;
    virtual MError DoAddParam(const uint32_t *p_param) override;
    virtual MError DoAddParam(const int64_t *p_param) override;
    virtual MError DoAddParam(const uint64_t *p_param) override;
    virtual MError DoAddParam(const float *p_param) override;
    virtual MError DoAddParam(const double *p_param) override;
    virtual MError DoAddParam(const std::string *p_param) override;
    virtual MError DoAddParam(const MBlob *p_param) override;

    virtual MError DoGetParam(int8_t *p_param) override;
    virtual MError DoGetParam(uint8_t *p_param) override;
    virtual MError DoGetParam(int16_t *p_param) override;
    virtual MError DoGetParam(uint16_t *p_param) override;
    virtual MError DoGetParam(int32_t *p_param) override;
    virtual MError DoGetParam(uint32_t *p_param) override;
    virtual MError DoGetParam(int64_t *p_param) override;
    virtual MError DoGetParam(uint64_t *p_param) override;
    virtual MError DoGetParam(float *p_param) override;
    virtual MError DoGetParam(double *p_param) override;
    virtual MError DoGetParam(std::string *p_param) override;
    virtual MError DoGetParam(MBlob *p_param) override;
private:
    template<typename T>
    MError AddParamNumeric(const T *p_param)
    {
        in_params_.push_back(static_cast<const char*>(static_cast<const void*>(p_param)));
        in_param_lens_.push_back(sizeof(*p_param));
        return MError::No;
    }
    template<typename T>
    MError GetParamNumeric(T *p_param)
    {
        if (!pp_result_)
        {
            MLOG(MGetLibLogger(), MERR, "pp_result_ is null");
            return MError::NoData;
        }
        if (cur_row_ >= row_count_)
        {
            MLOG(MGetLibLogger(), MERR, "cur row:", cur_row_, " is large than row count:", row_count_);
            return MError::NoData;
        }
        redisReply *p_reply = pp_result_[cur_row_++];
        if (!p_reply)
        {
            MLOG(MGetLibLogger(), MERR, "p_reply is null, cur row:", cur_row_);
            return MError::NoData;
        }
        if (!p_param)
        {
            return MError::No;
        }
        if (p_reply->type == REDIS_REPLY_INTEGER)
        {
            *p_param = static_cast<T>(p_reply->integer);
        }
        else if (p_reply->type == REDIS_REPLY_STRING)
        {
            std::string str(p_reply->str, p_reply->len);
            if (!MConvertTo(str, *p_param))
            {
                MLOG(MGetLibLogger(), MERR, "convert str to numeric failed");
                return MError::ConvertFailed;
            }
        }
        else
        {
            MLOG(MGetLibLogger(), MERR, "type:", p_reply->type, " can't convert to numeric");
            return MError::ConvertFailed;
        }
        return MError::No;
    }
private:
    MRedisConnection &conn_;
    redisReply *p_reply_;
    redisReply **pp_result_;
    std::string command_;
    std::vector<const char*> in_params_;
    std::vector<size_t> in_param_lens_;
    size_t cur_row_;
    size_t row_count_;
};

#endif
