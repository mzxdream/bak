#include <db/m_mysql_connection.h>
#include <db/m_mysql_command.h>
#include <cstring>

MMysqlCommand::MMysqlCommand(MMysqlConnection &conn)
    :conn_(conn)
    ,p_stmt_(nullptr)
    ,cur_col_(0)
{
}

MMysqlCommand::~MMysqlCommand()
{
    if (p_stmt_)
    {
        mysql_stmt_close(p_stmt_);
        p_stmt_ = nullptr;
    }
}

MError MMysqlCommand::DoPrepair(const std::string &command)
{
    if (p_stmt_)
    {
        mysql_stmt_close(p_stmt_);
        p_stmt_ = nullptr;
    }
    p_stmt_ = mysql_stmt_init(conn_.GetConnection());
    if (!p_stmt_)
    {
        MLOG(MGetLibLogger(), MERR, "init stmt failed errno:", mysql_errno(conn_.GetConnection()), " error:", mysql_error(conn_.GetConnection()));
        return MError::Unknown;
    }
    if (mysql_stmt_prepare(p_stmt_, command.c_str(), command.size()) != 0)
    {
        MLOG(MGetLibLogger(), MERR, "mysql prepare:", command, " failed errno:", mysql_stmt_errno(p_stmt_), " error:", mysql_stmt_error(p_stmt_));
        return MError::Unknown;
    }
    return MError::No;
}

MError MMysqlCommand::DoBeforeAddParams()
{
    in_params_.clear();
    return MError::No;
}

MError MMysqlCommand::DoGotoNextRecord()
{
    int ret = mysql_stmt_fetch(p_stmt_);
    if (ret == 0)
    {
        cur_col_ = 0;
        return MError::No;
    }
    else if (ret == MYSQL_NO_DATA)
    {
        MLOG(MGetLibLogger(), MINFO, "have no more data");
        return MError::NoData;
    }
    else
    {
        MLOG(MGetLibLogger(), MINFO, "next record errno:", mysql_stmt_errno(p_stmt_), " error:", mysql_stmt_error(p_stmt_));
        return MError::Unknown;
    }
}

MError MMysqlCommand::DoGotoNextResult()
{
    int ret = mysql_stmt_next_result(p_stmt_);
    if (ret == 0)
    {
        return BindResult();
    }
    else if (ret < 0)
    {
        MLOG(MGetLibLogger(), MWARN, "next result ret is ", ret);
        return MError::Unknown;
    }
    else
    {
        MLOG(MGetLibLogger(), MWARN, "next result errno:", mysql_stmt_errno(p_stmt_), " error:", mysql_stmt_error(p_stmt_));
        return MError::Unknown;
    }
}

std::pair<unsigned, MError> MMysqlCommand::DoExecuteNonQuery()
{
    int param_count = mysql_stmt_param_count(p_stmt_);
    if (static_cast<size_t>(param_count) != in_params_.size())
    {
        MLOG(MGetLibLogger(), MERR, "param count is not match need:", param_count, " actual is:", in_params_.size());
        return std::pair<unsigned, MError>(0, MError::NotMatch);
    }
    if (mysql_stmt_bind_param(p_stmt_, &in_params_[0]) != 0)
    {
        MLOG(MGetLibLogger(), MERR, "bind failed errno:", mysql_stmt_errno(p_stmt_), " error:", mysql_stmt_error(p_stmt_));
        return std::pair<unsigned, MError>(0, MError::Unknown);
    }
    if (mysql_stmt_execute(p_stmt_) != 0)
    {
        MLOG(MGetLibLogger(), MERR, "execute failed errorno:", mysql_stmt_errno(p_stmt_), " error:", mysql_stmt_error(p_stmt_));
        return std::pair<unsigned, MError>(0, MError::Unknown);
    }
    unsigned affect = mysql_stmt_affected_rows(p_stmt_);
    return std::make_pair(affect, MError::No);
}

MError MMysqlCommand::DoExecuteReader()
{
    int param_count = mysql_stmt_param_count(p_stmt_);
    if (static_cast<size_t>(param_count) != in_params_.size())
    {
        MLOG(MGetLibLogger(), MERR, "param count is not match need:", param_count, " actual is :", in_params_.size());
        return MError::NotMatch;
    }
    if (mysql_stmt_bind_param(p_stmt_, &in_params_[0]) != 0)
    {
        MLOG(MGetLibLogger(), MERR, "bind failed errno:", mysql_stmt_errno(p_stmt_), " error:", mysql_stmt_error(p_stmt_));
        return MError::Unknown;
    }
    my_bool update_max_length = 1;
    mysql_stmt_attr_set(p_stmt_, STMT_ATTR_UPDATE_MAX_LENGTH, static_cast<void*>(&update_max_length));
    if (mysql_stmt_execute(p_stmt_) != 0)
    {
        MLOG(MGetLibLogger(), MERR, "execute failed errno:", mysql_stmt_errno(p_stmt_), " error:", mysql_stmt_error(p_stmt_));
        return MError::Unknown;
    }
    return BindResult();
}

MError MMysqlCommand::DoAddParam(const int8_t *p_param)
{
    return AddParamNumeric(MYSQL_TYPE_TINY, 0, p_param);
}

MError MMysqlCommand::DoAddParam(const uint8_t *p_param)
{
    return AddParamNumeric(MYSQL_TYPE_TINY, 1, p_param);
}

MError MMysqlCommand::DoAddParam(const int16_t *p_param)
{
    return AddParamNumeric(MYSQL_TYPE_SHORT, 0, p_param);
}

MError MMysqlCommand::DoAddParam(const uint16_t *p_param)
{
    return AddParamNumeric(MYSQL_TYPE_SHORT, 1, p_param);
}

MError MMysqlCommand::DoAddParam(const int32_t *p_param)
{
    return AddParamNumeric(MYSQL_TYPE_LONG, 0, p_param);
}

MError MMysqlCommand::DoAddParam(const uint32_t *p_param)
{
    return AddParamNumeric(MYSQL_TYPE_LONG, 1, p_param);
}

MError MMysqlCommand::DoAddParam(const int64_t *p_param)
{
    return AddParamNumeric(MYSQL_TYPE_LONGLONG, 0, p_param);
}

MError MMysqlCommand::DoAddParam(const uint64_t *p_param)
{
    return AddParamNumeric(MYSQL_TYPE_LONGLONG, 1, p_param);
}

MError MMysqlCommand::DoAddParam(const float *p_param)
{
    return AddParamNumeric(MYSQL_TYPE_FLOAT, 0, p_param);
}

MError MMysqlCommand::DoAddParam(const double *p_param)
{
    return AddParamNumeric(MYSQL_TYPE_DOUBLE, 0, p_param);
}

MError MMysqlCommand::DoAddParam(const std::string *p_param)
{
    MYSQL_BIND bind;
    memset(&bind, 0, sizeof(bind));
    bind.buffer_type = MYSQL_TYPE_VAR_STRING;
    bind.buffer = const_cast<char*>(&((*p_param)[0]));
    bind.buffer_length = p_param->size();
    bind.length = nullptr;
    bind.is_null = nullptr;
    bind.is_unsigned = 0;
    in_params_.push_back(bind);
    return MError::No;
}

MError MMysqlCommand::DoAddParam(const MBlob *p_param)
{
    MYSQL_BIND bind;
    memset(&bind, 0, sizeof(bind));
    bind.buffer_type = MYSQL_TYPE_BLOB;
    bind.buffer = const_cast<char*>(p_param->GetData());
    bind.buffer_length = p_param->GetSize();
    bind.length = nullptr;
    bind.is_null = nullptr;
    bind.is_unsigned = 0;
    in_params_.push_back(bind);
    return MError::No;
}

MError MMysqlCommand::DoGetParam(int8_t *p_param)
{
    return GetParamNumeric(p_param);
}

MError MMysqlCommand::DoGetParam(uint8_t *p_param)
{
    return GetParamNumeric(p_param);
}

MError MMysqlCommand::DoGetParam(int16_t *p_param)
{
    return GetParamNumeric(p_param);
}

MError MMysqlCommand::DoGetParam(uint16_t *p_param)
{
    return GetParamNumeric(p_param);
}

MError MMysqlCommand::DoGetParam(int32_t *p_param)
{
    return GetParamNumeric(p_param);
}

MError MMysqlCommand::DoGetParam(uint32_t *p_param)
{
    return GetParamNumeric(p_param);
}

MError MMysqlCommand::DoGetParam(int64_t *p_param)
{
    return GetParamNumeric(p_param);
}

MError MMysqlCommand::DoGetParam(uint64_t *p_param)
{
    return GetParamNumeric(p_param);
}

MError MMysqlCommand::DoGetParam(float *p_param)
{
    return GetParamNumeric(p_param);
}

MError MMysqlCommand::DoGetParam(double *p_param)
{
    return GetParamNumeric(p_param);
}

MError MMysqlCommand::DoGetParam(std::string *p_param)
{
    if (cur_col_ >= out_params_.size())
    {
        MLOG(MGetLibLogger(), MERR, "record have no more data");
        return MError::NoData;
    }
    const MYSQL_BIND &bind = out_params_[cur_col_++];
    if (!p_param)
    {
        return MError::No;
    }
    if (*(bind.is_null)
            || bind.buffer_type == MYSQL_TYPE_NULL)
    {
        *p_param = "";
        return MError::No;
    }
    if (bind.buffer_type == MYSQL_TYPE_TINY_BLOB
        || bind.buffer_type == MYSQL_TYPE_MEDIUM_BLOB
        || bind.buffer_type == MYSQL_TYPE_LONG_BLOB
        || bind.buffer_type == MYSQL_TYPE_BLOB
        || bind.buffer_type == MYSQL_TYPE_STRING
        || bind.buffer_type == MYSQL_TYPE_VAR_STRING)
    {
        p_param->resize(*(bind.length));
        memcpy(&((*p_param)[0]), bind.buffer, p_param->size());
        return MError::No;
    }
    MLOG(MGetLibLogger(), MERR, "can't convert type:", bind.buffer_type, " to string");
    return MError::ConvertFailed;
}

MError MMysqlCommand::DoGetParam(MBlob *p_param)
{
    if (cur_col_ >= out_params_.size())
    {
        MLOG(MGetLibLogger(), MERR, "have no more data :", cur_col_+1);
        return MError::NoData;
    }
    const MYSQL_BIND &bind = out_params_[cur_col_++];
    if (!p_param)
    {
        return MError::No;
    }
    if (*(bind.is_null)
        || bind.buffer_type == MYSQL_TYPE_NULL)
    {
        p_param->Resize(0);
        return MError::No;
    }
    if (bind.buffer_type == MYSQL_TYPE_TINY_BLOB
        || bind.buffer_type == MYSQL_TYPE_MEDIUM_BLOB
        || bind.buffer_type == MYSQL_TYPE_LONG_BLOB
        || bind.buffer_type == MYSQL_TYPE_BLOB
        || bind.buffer_type == MYSQL_TYPE_STRING
        || bind.buffer_type == MYSQL_TYPE_VAR_STRING)
    {
        p_param->Resize(*(bind.length));
        memcpy(p_param->GetData(), bind.buffer, p_param->GetSize());
        return MError::No;
    }
    MLOG(MGetLibLogger(), MERR, "can't convert type:", bind.buffer_type, " to MBlob");
    return MError::ConvertFailed;
}

MError MMysqlCommand::BindResult()
{
    if (mysql_stmt_store_result(p_stmt_) != 0)
    {
        MLOG(MGetLibLogger(), MERR, "store result failed errno:", mysql_stmt_errno(p_stmt_), " error:", mysql_stmt_error(p_stmt_));
        return MError::Unknown;
    }
    MYSQL_RES *p_meta_res = mysql_stmt_result_metadata(p_stmt_);
    if (!p_meta_res)
    {
        MLOG(MGetLibLogger(), MERR, "have not meta res errno:", mysql_stmt_errno(p_stmt_), " error:", mysql_stmt_error(p_stmt_));
        return MError::Unknown;
    }
    MError err = MError::No;
    do
    {
        MYSQL_FIELD *p_field_list = mysql_fetch_fields(p_meta_res);
        if (!p_field_list)
        {
            MLOG(MGetLibLogger(), MERR, "mysql_fetch_fields is null unknown error");
            err = MError::Unknown;
            break;
        }
        unsigned long field_count = mysql_num_fields(p_meta_res);
        if (field_count == 0)
        {
            MLOG(MGetLibLogger(), MERR, "mysql field_count is 0");
            err = MError::Unknown;
            break;
        }
        out_params_.resize(field_count);
        out_datas_.resize(field_count);
        out_lengths_.resize(field_count);
        out_is_nulls_.resize(field_count);
        for (size_t i = 0; i < field_count; ++i)
        {
            MBlob &out_data = out_datas_[i];
            out_data.Resize(p_field_list[i].max_length);
            memset(&(out_params_[i]), 0, sizeof(out_params_[i]));
            out_params_[i].buffer_type = p_field_list[i].type;
            out_params_[i].buffer = out_data.GetData();
            out_params_[i].buffer_length = out_data.GetSize();
            out_params_[i].length = &(out_lengths_[i]);
            out_params_[i].is_null = &(out_is_nulls_[i]);
            out_params_[i].is_unsigned = (p_field_list[i].flags & UNSIGNED_FLAG);
            out_params_[i].error = nullptr;
        }
        if (mysql_stmt_bind_result(p_stmt_, &out_params_[0]) != 0)
        {
            MLOG(MGetLibLogger(), MERR, "bind result failed errno:", mysql_stmt_errno(p_stmt_), " error:", mysql_stmt_error(p_stmt_));
            err = MError::Unknown;
            break;
        }
        err = MError::No;
    } while (0);
    mysql_free_result(p_meta_res);
    return err;
}
