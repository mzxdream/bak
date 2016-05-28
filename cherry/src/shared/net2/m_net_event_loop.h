#ifndef _M_NET_EVENT_LOOP_H_
#define _M_NET_EVENT_LOOP_H_

#include <net/m_net_common.h>
#include <vector>
#include <util/m_errno.h>

class MNetEvent;

class MNetEventLoop
{
public:
    explicit MNetEventLoop(size_t single_process_events = 128);
    ~MNetEventLoop();
    MNetEventLoop(const MNetEventLoop &) = delete;
    MNetEventLoop& operator=(const MNetEventLoop &) = delete;
public:
    size_t GetEventCount() const;
    MError Create();
    MError Close();
    MError AddEvent(int fd, int events, MNetEvent *p_event);
    MError ModEvent(int fd, int events, MNetEvent *p_event);
    MError DelEvent(int fd);
    MError ProcessEvents();
    MError Interrupt();
private:
    int epoll_fd_;
    std::vector<epoll_event> event_list_;
    int interrupter_[2];
    size_t event_count_;
};

#endif
