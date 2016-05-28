#ifndef _NET_THREAD_H_
#define _NET_THREAD_H_

#include <net/net_session.h>
#include <net/m_net_event_loop.h>
#include <thread/m_thread.h>
#include <mutex>
#include <functional>
#include <list>

class NetThread
    :private MThread
{
public:
    NetThread(uint8_t id);
    virtual ~NetThread();
public:
    bool Init();
    void Close();
    bool Start();
    void StopAndJoin();
public:
    MNetEventLoop& GetEventLoop();
    size_t GetEventCount() const;
public:
    bool Send(uint64_t id, char *p_buf, size_t len);
    bool Close(uint64_t id);
private:
    virtual void DoRun() override;
private:
    uint8_t id_;
    MNetEventLoop event_loop_;
    std::list<std::function<void ()> > cb_list_;
    std::mutex cb_mutex_;
    std::map<uint64_t, NetSession*> sessions_;
};

#endif
