#include <db/m_redis_connection.h>
#include <db/m_redis_command.h>
#include <util/m_convert.h>
#include <util/m_string.h>
#include <sys/time.h>
#include <string>

static const std::string sc_redis_sep = ";";
static const std::string sc_redis_op = "=";
static const std::string sc_redis_trim = " ";
static const std::string sc_redis_ip = "ip";
static const std::string sc_redis_port = "port";
static const std::string sc_redis_timeout = "timeout";
static const std::string sc_redis_pwd = "pwd";
static const std::string sc_redis_db = "db";

MRedisConnection::MRedisConnection()
    :p_redis_(nullptr)
{
}

MRedisConnection::~MRedisConnection()
{
    if (p_redis_)
    {
        redisFree(p_redis_);
        p_redis_ = nullptr;
    }
}

redisContext* MRedisConnection::GetConnection()
{
    return p_redis_;
}

MError MRedisConnection::AuthPwd(const std::string &pwd)
{
    redisReply *p_reply = static_cast<redisReply*>(redisCommand(p_redis_, "AUTH %s", pwd.c_str()));
    if (!p_reply)
    {
        MLOG(MGetLibLogger(), MERR, "auth pwd failed reply is null");
        return MError::Unknown;
    }
    MError err = MError::No;
    if (p_reply->type == REDIS_REPLY_ERROR
        || (p_reply->type == REDIS_REPLY_STATUS && MString::CompareNoCase(p_reply->str, "OK") != 0))
    {
        MLOG(MGetLibLogger(), MERR, "auth pwd failed error:", p_reply->str);
    }
    freeReplyObject(p_reply);
    p_reply = nullptr;
    return err;
}

MDbConnParamStyleType MRedisConnection::DoGetParamStyleType()
{
    return MDbConnParamStyleType::CFormat;
}

MDbConnThreadSafetyType MRedisConnection::DoGetThreadSafetyType()
{
    return MDbConnThreadSafetyType::Connect;
}

MError MRedisConnection::DoOpen(const std::string &conn_string)
{
    if (p_redis_)
    {
        MLOG(MGetLibLogger(), MERR, "redis is opened");
        return MError::ConnectOpened;
    }

    std::string ip;
    if (!MDbConnString::GetParamValue(conn_string, sc_redis_ip, ip, sc_redis_op, sc_redis_sep, sc_redis_trim))
    {
        MLOG(MGetLibLogger(), MERR, "can't get ip info");
        return MError::Unknown;
    }

    int port = 0;
    if (!MDbConnString::GetParamValue(conn_string, sc_redis_port, port, sc_redis_op, sc_redis_sep, sc_redis_trim))
    {
        MLOG(MGetLibLogger(), MERR, "can't get port info");
        return MError::Unknown;
    }

    time_t timeout = 0;
    if (!MDbConnString::GetParamValue(conn_string, sc_redis_timeout, timeout, sc_redis_op, sc_redis_sep, sc_redis_trim))
    {
        p_redis_ = redisConnect(ip.c_str(), port);
    }
    else
    {
        struct timeval tm;
        tm.tv_sec = timeout;
        tm.tv_usec = 0;
        p_redis_ = redisConnectWithTimeout(ip.c_str(), port, tm);
    }

    if (!p_redis_)
    {
        MLOG(MGetLibLogger(), MERR, "redis connect is null");
        return MError::ConnectFailed;
    }
    MError err = MError::No;
    do
    {
        if (p_redis_->err != REDIS_OK)
        {
            MLOG(MGetLibLogger(), MERR, "connect error:", p_redis_->errstr);
            err = MError::Unknown;
            break;
        }

        std::string pwd;
        if (MDbConnString::GetParamValue(conn_string, sc_redis_pwd, pwd, sc_redis_op, sc_redis_sep, sc_redis_trim))
        {
            err = AuthPwd(pwd);
            if (err != MError::No)
            {
                break;
            }
        }

        std::string db;
        if (MDbConnString::GetParamValue(conn_string, sc_redis_db, db, sc_redis_op, sc_redis_sep, sc_redis_trim))
        {
            err = SelectDb(db);
            if (err != MError::No)
            {
                break;
            }
        }
        conn_string_ = conn_string;
        err = MError::No;
    } while (0);
    if (err != MError::No)
    {
        redisFree(p_redis_);
        p_redis_ = nullptr;
    }
    return err;
}

MError MRedisConnection::DoCheckConnect()
{
    redisReply *p_reply = static_cast<redisReply*>(redisCommand(p_redis_, "PING"));
    if (!p_reply)
    {
        MLOG(MGetLibLogger(), MERR, "ping failed reply is null");
        return MError::Unknown;
    }
    MError err = MError::No;
    if (p_reply->type == REDIS_REPLY_ERROR
        || (p_reply->type == REDIS_REPLY_STATUS && MString::CompareNoCase(p_reply->str, "PONG") != 0))
    {
        MLOG(MGetLibLogger(), MERR, "ping failed error:", p_reply->str);
        err = MError::Unknown;
    }
    freeReplyObject(p_reply);
    p_reply = nullptr;
    return err;
}

MError MRedisConnection::DoCheckAndReconnect()
{
    MError err = CheckConnect();
    if (err != MError::No)
    {
        Close();
        return Open(conn_string_);
    }
    return err;
}

void MRedisConnection::DoClose()
{
    if (p_redis_)
    {
        redisFree(p_redis_);
        p_redis_ = nullptr;
    }
}

MError MRedisConnection::DoSelectDb(const std::string &db)
{
    redisReply *p_reply = static_cast<redisReply*>(redisCommand(p_redis_, "SELECT %s", db.c_str()));
    if (!p_reply)
    {
        MLOG(MGetLibLogger(), MERR, "select db failed reply is null");
        return MError::Unknown;
    }
    MError err = MError::No;
    if (p_reply->type == REDIS_REPLY_ERROR
        || (p_reply->type == REDIS_REPLY_STATUS && MString::CompareNoCase(p_reply->str, "OK") != 0))
    {
        MLOG(MGetLibLogger(), MERR, "select db failed error:", p_reply->str);
        err = MError::Unknown;
    }
    freeReplyObject(p_reply);
    p_reply = nullptr;
    return err;
}

MDbCommand* MRedisConnection::DoCreateCommand()
{
    return new MRedisCommand(*this);
}
