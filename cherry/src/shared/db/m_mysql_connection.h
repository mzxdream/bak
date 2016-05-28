#ifndef _M_MYSQL_CONNECTION_H_
#define _M_MYSQL_CONNECTION_H_

#include <db/m_db_connection.h>
#include <string>
#include <util/m_errno.h>
#include <util/m_logger.h>
#include <mysql.h>

class MMysqlConnection
    :public MDbConnection
{
public:
    MMysqlConnection();
    virtual ~MMysqlConnection();
public:
    MYSQL* GetConnection();
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
    MYSQL *p_mysql_;
};

#endif
