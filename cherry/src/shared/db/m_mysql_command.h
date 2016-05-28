#ifndef _M_MYSQL_COMMAND_H_
#define _M_MYSQL_COMMAND_H_

#include <db/m_db_command.h>
#include <util/m_convert.h>
#include <mysql.h>
#include <string>
#include <vector>
#include <cstring>
#include <util/m_type_define.h>
#include <util/m_errno.h>
#include <util/m_logger.h>

class MMysqlConnection;

class MMysqlCommand
    :public MDbCommand
{
public:
    explicit MMysqlCommand(MMysqlConnection &conn);
    virtual ~MMysqlCommand();
    MMysqlCommand(const MMysqlCommand &) = delete;
    MMysqlCommand& operator=(const MMysqlCommand &) = delete;
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
    template <typename T>
    MError AddParamNumeric(enum_field_types buffer_type, my_bool is_unsigned, const T *p_param)
    {
        MYSQL_BIND bind;
        memset(&bind, 0, sizeof(bind));
        bind.buffer_type = buffer_type;
        bind.buffer = const_cast<T*>(p_param);
        bind.buffer_length = sizeof(*p_param);
        bind.length = nullptr;
        bind.is_null = nullptr;
        bind.is_unsigned = is_unsigned;
        in_params_.push_back(bind);
        return MError::No;
    }
    template <typename T>
    MError GetParamNumeric(T *p_param)
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
        if (*(bind.is_null))
        {
            MLOG(MGetLibLogger(), MERR, "can't convert null to numeric");
            return MError::ConvertFailed;
        }
        switch (bind.buffer_type)
        {
        case MYSQL_TYPE_TINY:
            if (bind.is_unsigned)
            {
                *p_param = static_cast<T>(*static_cast<uint8_t*>(bind.buffer));
            }
            else
            {
                *p_param = static_cast<T>(*static_cast<int8_t*>(bind.buffer));
            }
            break;
        case MYSQL_TYPE_SHORT:
            if (bind.is_unsigned)
            {
                *p_param = static_cast<T>(*static_cast<uint16_t*>(bind.buffer));
            }
            else
            {
                *p_param = static_cast<T>(*static_cast<int16_t*>(bind.buffer));
            }
            break;
        case MYSQL_TYPE_LONG:
            if (bind.is_unsigned)
            {
                *p_param = static_cast<T>(*static_cast<uint32_t*>(bind.buffer));
            }
            else
            {
                *p_param = static_cast<T>(*static_cast<int32_t*>(bind.buffer));
            }
            break;
        case MYSQL_TYPE_LONGLONG:
            if (bind.is_unsigned)
            {
                *p_param = static_cast<T>(*static_cast<uint64_t*>(bind.buffer));
            }
            else
            {
                *p_param = static_cast<T>(*static_cast<int64_t*>(bind.buffer));
            }
            break;
        case MYSQL_TYPE_FLOAT:
            *p_param = static_cast<T>(*static_cast<float*>(bind.buffer));
            break;
        case MYSQL_TYPE_DOUBLE:
            *p_param = static_cast<T>(*static_cast<double*>(bind.buffer));
            break;
        default:
            MLOG(MGetLibLogger(), MERR, "can't convert type:", bind.buffer_type, " to numeric");
            return MError::ConvertFailed;
            break;
        }
        return MError::No;
    }
    MError BindResult();
private:
    MMysqlConnection &conn_;
    MYSQL_STMT *p_stmt_;
    std::vector<MYSQL_BIND> in_params_;
    std::vector<MYSQL_BIND> out_params_;
    std::vector<MBlob> out_datas_;
    std::vector<unsigned long> out_lengths_;
    std::vector<my_bool> out_is_nulls_;
    unsigned int cur_col_;
};

#endif
