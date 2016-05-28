#ifndef _M_NET_EVENT_LOOP_THREAD_H_
#define _M_NET_EVENT_LOOP_THREAD_H_

#include <net/m_net_event_loop.h>
#include <thread/m_thread.h>
#include <mutex>
#include <functional>
#include <list>

class MNetEventLoopThread
    :private MThread
{
public:
    MNetEventLoopThread(size_t single_process_events = 128);
    virtual ~MNetEventLoopThread();
    MNetEventLoopThread(const MNetEventLoopThread &) = delete;
    MNetEventLoopThread& operator=(const MNetEventLoopThread &) = delete;
public:
    MError Init();
    MError Close();
    MError Start();
    MError Stop();
    MError StopAndJoin();
    MNetEventLoop& GetEventLoop();
    size_t GetEventCount() const;
    void AddCallback(const std::function<void ()> &cb);
    MError Interrupt();
private:
    virtual void _Run() override;
private:
    MNetEventLoop event_loop_;
    std::list<std::function<void ()> > cb_list_;
    std::mutex cb_mutex_;
};

#endif
