#ifndef _M_DB_CONNECTION_H_
#define _M_DB_CONNECTION_H_

#include <string>
#include <db/m_db_common.h>
#include <util/m_errno.h>

enum class MDbConnThreadSafetyType
{
    None = 0,
    Model = 1,
    Connect = 2,
    Command = 3,
};

enum class MDbConnParamStyleType
{
    CFormat = 0,
    QuestionMark = 1,
};

class MDbCommand;

class MDbConnection
{
public:
    MDbConnection() = default;
    virtual ~MDbConnection() = default;
    MDbConnection(const MDbConnection &) = delete;
    MDbConnection& operator=(const MDbConnection &) = delete;
public:
    MDbConnParamStyleType GetParamStyleType()
    {
        return DoGetParamStyleType();
    }
    MDbConnThreadSafetyType GetThreadSafetyType()
    {
        return DoGetThreadSafetyType();
    }
    MError Open(const std::string &conn_string)
    {
        return DoOpen(conn_string);
    }
    MError CheckConnect()
    {
        return DoCheckConnect();
    }
    MError CheckAndReConnect()
    {
        return DoCheckAndReconnect();
    }
    void Close()
    {
        return DoClose();
    }
    MError SelectDb(const std::string &db)
    {
        return DoSelectDb(db);
    }
    MDbCommand* CreateCommand()
    {
        return DoCreateCommand();
    }
private:
    virtual MDbConnParamStyleType DoGetParamStyleType() = 0;
    virtual MDbConnThreadSafetyType DoGetThreadSafetyType() = 0;
    virtual MError DoOpen(const std::string &conn_string) = 0;
    virtual MError DoCheckConnect() = 0;
    virtual MError DoCheckAndReconnect() = 0;
    virtual void DoClose() = 0;
    virtual MError DoSelectDb(const std::string &db) = 0;
    virtual MDbCommand* DoCreateCommand() = 0;
};

#endif
