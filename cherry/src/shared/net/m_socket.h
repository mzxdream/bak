#ifndef _M_SOCKET_H_
#define _M_SOCKET_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <utility>
#include <net/m_net_common.h>
#include <util/m_errno.h>

enum class MSocketFamily
{
    IPV4 = AF_INET,
    IPV6 = AF_INET6,
};

enum class MSocketType
{
    TCP = SOCK_STREAM,
    UDP = SOCK_DGRAM,
};

enum class MSocketProtocol
{
    Default = 0,
    TCP = IPPROTO_TCP,
    UDP = IPPROTO_UDP,
};

class MSocket
{
public:
    explicit MSocket(int sock = -1);
    ~MSocket();
    MSocket(const MSocket &) = delete;
    MSocket& operator=(const MSocket &) = delete;
public:
    MError Attach(int sock);
    int Detach();
    MError Create(MSocketFamily family, MSocketType type, MSocketProtocol proto);
    MError Close();
    MError Bind(const std::string &ip, unsigned port);
    MError Listen(int backlog = 64);
    MError Accept(MSocket &sock);
    MError Connect(const std::string &ip, unsigned port);
    std::pair<int, MError> Send(const char *p_buf, int len);
    std::pair<int, MError> Recv(void *p_buf, int len);
    MError SetBlock(bool block);
    MError SetReUseAddr(bool re_use);
    int GetHandler() const;
    const std::string& GetBindIP() const;
    unsigned GetBindPort() const;
    const std::string& GetRemoteIP() const;
    unsigned GetRemotePort() const;
public:
    MError CreateNonblockReuseAddrListener(const std::string &ip, unsigned short port, int backlog = 64);
private:
    int sock_;
    std::string bind_ip_;
    unsigned short bind_port_;
    std::string remote_ip_;
    unsigned short remote_port_;
};

#endif
