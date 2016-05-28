#ifndef _M_SOCKET_OPTS_H_
#define _M_SOCKET_OPTS_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <utility>
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

enum class MSocketProto
{
    Default = 0,
    TCP = IPPROTO_TCP,
    UDP = IPPROTO_UDP,
};

class MSocketOpts
{
public:
    static std::pair<int, MError> Create(MSocketFamily family, MSocketType type, MSocketProto proto);
    static MError Destroy(int sock);
    static MError Bind(int sock, const std::string &ip, unsigned port);
    static MError Listen(int sock, int backlog = 64);
    static MError Accept(int sock, int &accept_sock, std::string &ip, unsigned &port);
    static MError Accept(int sock, int &accept_sock);
    static MError Connect(int sock, const std::string &ip, unsigned port);
    static std::pair<int, MError> Send(int sock, const char *p_buf, int len);
    static std::pair<int, MError> Recv(int sock, void *p_buf, int len);
    static MError SetNonBlock(int sock, bool enable);
    static MError SetReUseAddr(int sock, bool enable);
    static MError SetTCPNoDelay(int sock, bool enable);
};

#endif
