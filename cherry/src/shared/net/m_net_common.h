#ifndef _M_NET_COMMON_H_
#define _M_NET_COMMON_H_

#include <unistd.h>
#include <sys/epoll.h>
#include <cstddef>

#define M_NET_EVENT_READ (EPOLLIN|EPOLLRDHUP)
#define M_NET_EVENT_WRITE EPOLLOUT
#define M_NET_EVENT_LEVEL 0
#define M_NET_EVENT_EDGE EPOLLET

#endif
