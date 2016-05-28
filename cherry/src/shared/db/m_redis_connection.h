#ifndef _M_REDIS_CONNECTION_H_
#define _M_REDIS_CONNECTION_H_

#include <db/m_db_connection.h>
#include <hiredis.h>
#include <string>
#include <util/m_errno.h>
#include <util/m_logger.h>

class MRedisConnection
    :public MDbConnection
{
public:
    MRedisConnection();
    virtual ~MRedisConnection();
public:
    redisContext* GetConnection();
private:
    MError AuthPwd(const std::string &pwd);
private:
    virtual MDbConnParamStyleType DoGetParamStyleType() override;
    virtual MDbConnThreadSafetyType DoGetThreadSafetyType() override;
    virtual MError DoOpen(const std::string &conn_string) override;
    virtual MError DoCheckConnect() override;
    virtual MError DoCheckAndReconnect() override;
    virtual void DoClose() override;
    virtual MError DoSelectDb(const std::string &db) override;
    virtual MDbCommand* DoCreateCommand() override;
private:
    redisContext *p_redis_;
    std::string conn_string_;
};

#endif
