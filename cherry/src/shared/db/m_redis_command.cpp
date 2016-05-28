#include <db/m_redis_connection.h>
#include <db/m_redis_command.h>
#include <util/m_string.h>

MRedisCommand::MRedisCommand(MRedisConnection &conn)
    :conn_(conn)
    ,p_reply_(nullptr)
    ,pp_result_(nullptr)
    ,cur_row_(0)
    ,row_count_(0)
{
}

MRedisCommand::~MRedisCommand()
{
    if (p_reply_)
    {
        freeReplyObject(p_reply_);
        p_reply_ = nullptr;
    }
    pp_result_ = nullptr;
    cur_row_ = 0;
    row_count_ = 0;
}

MError MRedisCommand::DoPrepair(const std::string &command)
{
    command_ = command;
    in_params_.clear();
    in_param_lens_.clear();
    in_params_.push_back(&command_[0]);
    in_param_lens_.push_back(command_.size());
    return MError::No;
}

MError MRedisCommand::DoBeforeAddParams()
{
    in_params_.resize(1);
    in_param_lens_.resize(1);
    return MError::No;
}

MError MRedisCommand::DoGotoNextRecord()
{
    return MError::No;
}

MError MRedisCommand::DoGotoNextResult()
{
    return MError::No;
}

std::pair<unsigned, MError> MRedisCommand::DoExecuteNonQuery()
{
    if (p_reply_)
    {
        freeReplyObject(p_reply_);
        p_reply_ = nullptr;
    }
    pp_result_ = nullptr;
    cur_row_ = 0;
    row_count_ = 0;

    p_reply_ = static_cast<redisReply*>(redisCommandArgv(conn_.GetConnection(), in_params_.size(), &in_params_[0], &in_param_lens_[0]));

    if (!p_reply_)
    {
        MLOG(MGetLibLogger(), MERR, "Execute failed reply is null");
        return std::pair<unsigned, MError>(0, MError::Unknown);
    }
    if (p_reply_->type == REDIS_REPLY_ERROR)
    {
        MLOG(MGetLibLogger(), MERR, "Execute failed reply error:", p_reply_->str);
        return std::pair<unsigned, MError>(0, MError::Unknown);
    }
    else if (p_reply_->type == REDIS_REPLY_NIL)
    {
        return std::pair<unsigned, MError>(0, MError::No);
    }
    else if (p_reply_->type == REDIS_REPLY_STATUS)
    {
        if (MString::CompareNoCase(p_reply_->str, "OK") != 0)
        {
            MLOG(MGetLibLogger(), MERR, "status is failed error:", p_reply_->str);
            return std::pair<unsigned, MError>(0, MError::Unknown);
        }
    }
    else if (p_reply_->type == REDIS_REPLY_INTEGER
        || p_reply_->type == REDIS_REPLY_STRING)
    {
        pp_result_ = &p_reply_;
        row_count_ = 1;
    }
    else if (p_reply_->type == REDIS_REPLY_ARRAY)
    {
        pp_result_ = p_reply_->element;
        row_count_ = p_reply_->elements;
    }

    unsigned ret = 0;
    MError err = NextRecord(&ret);
    return std::make_pair(ret, err);
}

MError MRedisCommand::DoExecuteReader()
{
    if (p_reply_)
    {
        freeReplyObject(p_reply_);
        p_reply_ = nullptr;
    }
    pp_result_ = nullptr;
    cur_row_ = 0;
    row_count_ = 0;

    p_reply_ = static_cast<redisReply*>(redisCommandArgv(conn_.GetConnection(), in_params_.size(), &in_params_[0], &in_param_lens_[0]));

    if (!p_reply_)
    {
        MLOG(MGetLibLogger(), MERR, "Execute failed reply is null");
        return MError::Unknown;
    }
    if (p_reply_->type == REDIS_REPLY_ERROR)
    {
        MLOG(MGetLibLogger(), MERR, "Execute failed reply error:", p_reply_->str);
        return MError::Unknown;
    }
    else if (p_reply_->type == REDIS_REPLY_NIL)
    {
        return MError::No;
    }
    else if (p_reply_->type == REDIS_REPLY_STATUS)
    {
        if (MString::CompareNoCase(p_reply_->str, "OK") != 0)
        {
            MLOG(MGetLibLogger(), MERR, "status is failed error:", p_reply_->str);
            return MError::Unknown;
        }
    }
    else if (p_reply_->type == REDIS_REPLY_INTEGER
        || p_reply_->type == REDIS_REPLY_STRING)
    {
        pp_result_ = &p_reply_;
        row_count_ = 1;
    }
    else if (p_reply_->type == REDIS_REPLY_ARRAY)
    {
        pp_result_ = p_reply_->element;
        row_count_ = p_reply_->elements;
    }
    return MError::No;
}

MError MRedisCommand::DoAddParam(const int8_t *p_param)
{
    return AddParamNumeric(p_param);
}

MError MRedisCommand::DoAddParam(const uint8_t *p_param)
{
    return AddParamNumeric(p_param);
}

MError MRedisCommand::DoAddParam(const int16_t *p_param)
{
    return AddParamNumeric(p_param);
}

MError MRedisCommand::DoAddParam(const uint16_t *p_param)
{
    return AddParamNumeric(p_param);
}

MError MRedisCommand::DoAddParam(const int32_t *p_param)
{
    return AddParamNumeric(p_param);
}

MError MRedisCommand::DoAddParam(const uint32_t *p_param)
{
    return AddParamNumeric(p_param);
}

MError MRedisCommand::DoAddParam(const int64_t *p_param)
{
    return AddParamNumeric(p_param);
}

MError MRedisCommand::DoAddParam(const uint64_t *p_param)
{
    return AddParamNumeric(p_param);
}

MError MRedisCommand::DoAddParam(const float *p_param)
{
    return AddParamNumeric(p_param);
}

MError MRedisCommand::DoAddParam(const double *p_param)
{
    return AddParamNumeric(p_param);
}

MError MRedisCommand::DoAddParam(const std::string *p_param)
{
    in_params_.push_back(&((*p_param)[0]));
    in_param_lens_.push_back(p_param->size());
    return MError::No;
}

MError MRedisCommand::DoAddParam(const MBlob *p_param)
{
    in_params_.push_back(p_param->GetData());
    in_param_lens_.push_back(p_param->GetSize());
    return MError::No;
}

MError MRedisCommand::DoGetParam(int8_t *p_param)
{
    return GetParamNumeric(p_param);
}

MError MRedisCommand::DoGetParam(uint8_t *p_param)
{
    return GetParamNumeric(p_param);
}

MError MRedisCommand::DoGetParam(int16_t *p_param)
{
    return GetParamNumeric(p_param);
}

MError MRedisCommand::DoGetParam(uint16_t *p_param)
{
    return GetParamNumeric(p_param);
}

MError MRedisCommand::DoGetParam(int32_t *p_param)
{
    return GetParamNumeric(p_param);
}

MError MRedisCommand::DoGetParam(uint32_t *p_param)
{
    return GetParamNumeric(p_param);
}

MError MRedisCommand::DoGetParam(int64_t *p_param)
{
    return GetParamNumeric(p_param);
}

MError MRedisCommand::DoGetParam(uint64_t *p_param)
{
    return GetParamNumeric(p_param);
}

MError MRedisCommand::DoGetParam(float *p_param)
{
    return GetParamNumeric(p_param);
}

MError MRedisCommand::DoGetParam(double *p_param)
{
    return GetParamNumeric(p_param);
}

MError MRedisCommand::DoGetParam(std::string *p_param)
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
        if (!MConvertTo(p_reply->integer, *p_param))
        {
            MLOG(MGetLibLogger(), MERR, "convert int to string failed");
            return MError::ConvertFailed;
        }
    }
    else if (p_reply->type == REDIS_REPLY_STRING)
    {
        p_param->resize(p_reply->len);
        memcpy(&((*p_param)[0]), p_reply->str, p_param->size());
        return MError::No;
    }
    else
    {
        MLOG(MGetLibLogger(), MERR, "type:", p_reply->type, " can't convert to string");
        return MError::ConvertFailed;
    }
    return MError::No;
}

MError MRedisCommand::DoGetParam(MBlob *p_param)
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
        return MError::ConvertFailed;
    }
    else if (p_reply->type == REDIS_REPLY_STRING)
    {
        p_param->Resize(p_reply->len);
        memcpy(p_param->GetData(), p_reply->str, p_param->GetSize());
        return MError::No;
    }
    else
    {
        MLOG(MGetLibLogger(), MERR, "type:", p_reply->type, " can't convert to blob");
        return MError::ConvertFailed;
    }
}
