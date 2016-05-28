#include <db/m_mysql_connection.h>
#include <db/m_mysql_command.h>
#include <util/m_convert.h>
#include <util/m_string.h>
#include <mysql.h>

static const std::string sc_mysql_sep = ";";
static const std::string sc_mysql_op = "=";
static const std::string sc_mysql_trim = " ";
static const std::string sc_mysql_ip = "ip";
static const std::string sc_mysql_port = "port";
static const std::string sc_mysql_timeout = "timeout";
static const std::string sc_mysql_user = "user";
static const std::string sc_mysql_pwd = "pwd";
static const std::string sc_mysql_db = "db";
static const std::string sc_mysql_charset = "charset";
static const std::string sc_mysql_client_flag = "clientflag";

MMysqlConnection::MMysqlConnection()
    :p_mysql_(nullptr)
{
}

MMysqlConnection::~MMysqlConnection()
{
    if (p_mysql_)
    {
        mysql_close(p_mysql_);
        p_mysql_ = nullptr;
    }
}

MYSQL* MMysqlConnection::GetConnection()
{
    return p_mysql_;
}

MDbConnParamStyleType MMysqlConnection::DoGetParamStyleType()
{
    return MDbConnParamStyleType::QuestionMark;
}

MDbConnThreadSafetyType MMysqlConnection::DoGetThreadSafetyType()
{
    return MDbConnThreadSafetyType::Connect;
}

MError MMysqlConnection::DoOpen(const std::string &conn_string)
{
    if (p_mysql_)
    {
        MLOG(MGetLibLogger(), MERR, "mysql is opened");
        return MError::ConnectOpened;
    }
    if ((p_mysql_ = mysql_init(nullptr)) == nullptr)
    {
        MLOG(MGetLibLogger(), MERR, "mysql_init failed");
        return MError::Unknown;
    }
    MError err = MError::No;
    do
    {
        unsigned int timeout = 0;
        if (MDbConnString::GetParamValue(conn_string, sc_mysql_timeout, timeout, sc_mysql_op, sc_mysql_sep, sc_mysql_trim))
        {
            if (mysql_options(p_mysql_, MYSQL_OPT_CONNECT_TIMEOUT, &timeout) != 0)
            {
                MLOG(MGetLibLogger(), MERR, "set timeout failed errno:", mysql_errno(p_mysql_), " error:", mysql_error(p_mysql_));
                err = MError::Unknown;
                break;
            }
        }

        char reconnect = 1;
        if (mysql_options(p_mysql_, MYSQL_OPT_RECONNECT, &reconnect) != 0)
        {
            MLOG(MGetLibLogger(), MERR, "set reconnect failed errno:", mysql_errno(p_mysql_), " error:", mysql_error(p_mysql_));
            err = MError::Unknown;
            break;
        }

        std::string ip;
        const char *p_ip = nullptr;
        if (MDbConnString::GetParamValue(conn_string, sc_mysql_ip, ip, sc_mysql_op, sc_mysql_sep, sc_mysql_trim))
        {
            p_ip = ip.c_str();
        }
        unsigned int port = 0;
        MDbConnString::GetParamValue(conn_string, sc_mysql_port, port, sc_mysql_op, sc_mysql_sep, sc_mysql_trim);
        std::string user;
        const char *p_user = nullptr;
        if (MDbConnString::GetParamValue(conn_string, sc_mysql_user, user, sc_mysql_op, sc_mysql_sep, sc_mysql_trim))
        {
            p_user = user.c_str();
        }
        std::string pwd;
        const char *p_pwd = nullptr;
        if (MDbConnString::GetParamValue(conn_string, sc_mysql_pwd, pwd, sc_mysql_op, sc_mysql_sep, sc_mysql_trim))
        {
            p_pwd = pwd.c_str();
        }
        std::string db;
        const char *p_db = nullptr;
        if (MDbConnString::GetParamValue(conn_string, sc_mysql_db, db, sc_mysql_op, sc_mysql_sep, sc_mysql_trim))
        {
            p_db = db.c_str();
        }
        unsigned int client_flag = 0;
        MDbConnString::GetParamValue(conn_string, sc_mysql_client_flag, client_flag, sc_mysql_op, sc_mysql_sep, sc_mysql_trim);

        if (mysql_real_connect(p_mysql_, p_ip, p_user, p_pwd, p_db, port, nullptr, client_flag) != p_mysql_)
        {
            MLOG(MGetLibLogger(), MERR, "connect failed errno:", mysql_errno(p_mysql_), " error:", mysql_error(p_mysql_));
            err = MError::ConnectFailed;
            break;
        }

        std::string charset;
        if (MDbConnString::GetParamValue(conn_string, sc_mysql_charset, charset, sc_mysql_op, sc_mysql_sep, sc_mysql_trim))
        {
            if (mysql_set_character_set(p_mysql_, charset.c_str()) != 0)
            {
                MLOG(MGetLibLogger(), MERR, "set charset failed errno:", mysql_errno(p_mysql_), " error:", mysql_error(p_mysql_));
                err = MError::Unknown;
                break;
           }
        }
        err = MError::No;
    } while (0);
    if (err != MError::No)
    {
        mysql_close(p_mysql_);
        p_mysql_ = nullptr;
    }
    return err;
}

MError MMysqlConnection::DoCheckConnect()
{
    if (mysql_query(p_mysql_, "select 1+1") != 0)
    {
        MLOG(MGetLibLogger(), MWARN, "select 1+1 failed errno:", mysql_errno(p_mysql_), " error:", mysql_error(p_mysql_));
        return MError::Disconnect;
    }
    MError err = MError::No;
#if 1 //test code, need set zero
    MYSQL_RES *p_res = mysql_store_result(p_mysql_);
    if (!p_res)
    {
        MLOG(MGetLibLogger(), MERR, "get res failed errno:", mysql_errno(p_mysql_), " error:", mysql_error(p_mysql_));
        return MError::Unknown;
    }
    do
    {
        MYSQL_ROW row = mysql_fetch_row(p_res);
        if (!row)
        {
            MLOG(MGetLibLogger(), MERR, "get row failed errno:", mysql_errno(p_mysql_), " error:", mysql_error(p_mysql_));
            err = MError::Unknown;
            break;
        }
        if (mysql_num_fields(p_res) != 1)
        {
            MLOG(MGetLibLogger(), MERR, "fields is not 1");
            err = MError::Unknown;
            break;
        }
        unsigned long *p_lengths = mysql_fetch_lengths(p_res);
        if (!p_lengths)
        {
            MLOG(MGetLibLogger(), MERR, "lengths is null");
            err = MError::Unknown;
            break;
        }
        int result = 0;
        if (!MConvertTo(std::string(row[0], p_lengths[0]), result)
            || result != 2)
        {
            MLOG(MGetLibLogger(), MERR, "result is wrong");
            err = MError::Unknown;
            break;
        }
        err = MError::No;
    } while(0);
    if (err != MError::No)
    {
       mysql_free_result(p_res);
       p_res = nullptr;
    }
#endif
    return err;
}

MError MMysqlConnection::DoCheckAndReconnect()
{
    if (mysql_ping(p_mysql_) != 0)
    {
        MLOG(MGetLibLogger(), MERR, "reconnect failed errno:", mysql_errno(p_mysql_), " error:", mysql_error(p_mysql_));
        return MError::Disconnect;
    }
    return MError::No;
}

void MMysqlConnection::DoClose()
{
    if (p_mysql_)
    {
        mysql_close(p_mysql_);
        p_mysql_ = nullptr;
    }
}

MError MMysqlConnection::DoSelectDb(const std::string &db)
{
    int ret = mysql_select_db(p_mysql_, db.c_str());
    if (ret != 0)
    {
        MLOG(MGetLibLogger(), MERR, "select db failed error:", ret);
        return MError::Unknown;
    }
    return MError::No;
}

MDbCommand* MMysqlConnection::DoCreateCommand()
{
    return new MMysqlCommand(*this);
}
