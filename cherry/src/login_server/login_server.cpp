#include <login_server.h>
#include <db/m_mysql_connection.h>
#include <db/m_db_command.h>
#include <util/m_logger.h>

LoginServer::LoginServer()
    :p_db_conn_(nullptr)
{
}

LoginServer::~LoginServer()
{
    if (p_db_conn_)
    {
        delete p_db_conn_;
        p_db_conn_ = nullptr;
    }
}

bool LoginServer::Init(const std::string &db_conn_string)
{
    db_conn_string_ = db_conn_string;
    p_db_conn_ = new MMysqlConnection();
    if (!p_db_conn_)
    {
        MLOG(MGetLibLogger(), MERR, "malloc mysql conn failed");
        return false;
    }
    if (p_db_conn_->Open(db_conn_string_) != MError::No)
    {
        MLOG(MGetLibLogger(), MERR, "open ", db_conn_string_, " failed");
        return false;
    }
    MDbCommand *p_db_command = p_db_conn_->CreateCommand();
    if (!p_db_command)
    {
        MLOG(MGetLibLogger(), MERR, "create command failed");
        return false;
    }
    if (p_db_command->Prepair("select `id`, `name`, `ip`, `port` from game_server") != MError::No)
    {
        MLOG(MGetLibLogger(), MERR, "prepair failed");
        return false;
    }
    if (p_db_command->ExecuteReader() != MError::No)
    {
        MLOG(MGetLibLogger(), MERR, "execute reader failed");
        return false;
    }
    GameServInfo info;
    while (p_db_command->NextRecord(&info.id, &info.name, &info.ip, &info.port) == MError::No)
    {
        game_serv_info_list_.push_back(info);
    }
    delete p_db_command;

    for (const auto &it : game_serv_info_list_)
    {
        std::cout << it.id << "," << it.name << "," << it.ip << "," << it.port << std::endl;
    }
    return true;
}

void LoginServer::Clear()
{
}

bool LoginServer::Update()
{
    return true;
}

bool LoginServer::LoadConf()
{
    return true;
}
