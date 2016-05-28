#include <util/m_singleton.h>
#include <string>

class ServerConf
    :public MSingleton<ServerConf>
{
protected:
    ServerConf();
    ~ServerConf();
public:
    bool Load(const std::string &conf);
private:
    std::string bind_ip;
    unsigned short bind_port;
    std::string db_ip;
    std::string db_port;
};
