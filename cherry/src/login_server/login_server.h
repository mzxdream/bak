#ifndef _LOGIN_SERVER_H_
#define _LOGIN_SERVER_H_

#include <util/m_singleton.h>
#include <mutex>
#include <db/m_db_connection.h>
#include <list>
#include <string>
#include <util/m_type_define.h>

struct GameServInfo
{
    uint32_t id;
    std::string name;
    std::string ip;
    uint32_t port;
};

class LoginServer
    :public MSingleton<LoginServer>
{
public:
    LoginServer();
    ~LoginServer();
public:
    bool Init(const std::string &db_conn_string);
    void Clear();
    bool Update();
private:
    bool LoadConf();
private:
    MDbConnection *p_db_conn_;
    std::string db_conn_string_;
    std::list<GameServInfo> game_serv_info_list_;
};

#endif
