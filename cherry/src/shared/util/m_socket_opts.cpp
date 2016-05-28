#include <net/m_socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

std::pair<int, MError> MSocketOpts::Create(MSocketFamily family, MSocketType type, MSocketProto proto)
{
    MError err = MError::No;
    int sock = socket(static_cast<int>(family), static_cast<int>(type), static_cast<int>(proto));
    if (sock == -1)
    {
        err = MError::Unknown;
    }
    return std::make_pair(sock, err);
}

MError MSocketOpts::Destroy(int sock)
{
    if (sock < 0)
    {
        return MError::No;
    }
    if (close(sock_) == -1)
    {
        return MError::Unknown;
    }
    return MError::No;
}

MError MSocketOpts::Bind(int sock, const std::string &ip, unsigned port)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if (ip.empty())
    {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
    }
    addr.sin_port = htons(port);
    if (bind(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1)
    {
        return MError::Unknown;
    }
    return MError::No;
}

MError MSocketOpts::Listen(int sock, int backlog)
{
    if (listen(sock, backlog) == -1)
    {
        return MError::Unknown;
    }
    return MError::No;
}

MError MSocketOpts::Accept(int sock, int &accept_sock, std::string &ip, unsigned &port)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    accept_sock = accept(sock, reinterpret_cast<struct sockaddr*>(&addr), &len);
    if (accept_sock == -1)
    {
        if (errno == EINTR)
        {
            return MError::InterruptedSysCall;
        }
        else if (errno == EAGAIN)
        {
            return MError::Again;
        }
        return MError::Unknown;
    }
    ip.resize(64);
    inet_ntop(AF_INET, &addr.sin_addr.s_addr, &ip[0], ip.size());
    port = ntohs(addr.sin_port);
    return MError::No;
}

MError MSocketOpts::Accept(int sock, int &accept_sock)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    accept_sock = accept(sock, reinterpret_cast<struct sockaddr*>(&addr), &len);
    if (accept_sock == -1)
    {
        if (errno == EINTR)
        {
            return MError::InterruptedSysCall;
        }
        else if (errno == EAGAIN)
        {
            return MError::Again;
        }
        return MError::Unknown;
    }
    return MError::No;
}

MError MSocketOpts::Connect(int sock, const std::string &ip, unsigned port)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if (ip.empty())
    {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
    }
    addr.sin_port = htons(port);
    if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1)
    {
        if (errno == EINPROGRESS)
        {
            return MError::InProgress;
        }
        return MError::Unknown;
    }
    return MError::No;
}

std::pair<int, MError> MSocketOpts::Send(int sock, const char *p_buf, int len)
{
    if (len <= 0)
    {
        return std::make_pair(0, MError::No);
    }
    int send_len = send(sock, p_buf, len, 0);
    if (send_len == -1)
    {
        if (errno == EINTR)
        {
            return std::make_pair(0, MError::InterruptedSysCall);
        }
        else if (errno == EAGAIN)
        {
            return std::make_pair(0, MError::Again);
        }
        return std::make_pair(0, MError::Unknown);
    }
    return std::make_pair(send_len, MError::No);
}

std::pair<int, MError> MSocketOpts::Recv(int sock, void *p_buf, int len)
{
    int recv_len = recv(sock, p_buf, len, 0);
    if (recv_len == -1)
    {
        if (errno == EINTR)
        {
            return std::make_pair(0, MError::InterruptedSysCall);
        }
        else if (errno == EAGAIN)
        {
            return std::make_pair(0, MError::Again);
        }
        return std::make_pair(0, MError::Unknown);
    }
    return std::make_pair(recv_len, MError::No);
}

MError MSocketOpts::SetNonBlock(int sock, bool enable)
{
    int flag = fcntl(sock, F_GETFL, 0);
    if (flag == -1)
    {
        return MError::Unknown;
    }
    if (!enable)
    {
        flag &= ~O_NONBLOCK;
    }
    else
    {
        flag |= O_NONBLOCK;
    }
    flag = fcntl(sock, F_SETFL, flag);
    if (flag == -1)
    {
        return MError::Unknown;
    }
    return MError::No;
}

MError MSocketOpts::SetReUseAddr(int sock, bool enable)
{
    int re_use = enable ? 1 : 0;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&re_use), sizeof(re_use)) < 0)
    {
        return MError::Unknown;
    }
    return MError::No;
}

MError MSocketOpts::SetTCPNoDelay(int sock, bool enable)
{
    int no_delay = enable ? 1 : 0;
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&no_delay), sizeof(no_delay)) < 0)
    {
        return MError::Unknown;
    }
    return MError::No;
}
