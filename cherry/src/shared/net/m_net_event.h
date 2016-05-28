#ifndef _M_NET_EVENT_H_
#define _M_NET_EVENT_H_

#include <net/m_net_common.h>
#include <functional>
#include <util/m_errno.h>

class MNetEventLoop;

class MNetEvent
{
public:
    explicit MNetEvent(int fd, MNetEventLoop *p_event_loop
        , const std::function<void ()> &read_cb, const std::function<void ()> &write_cb, const std::function<void (MError)> &error_cb);
    ~MNetEvent();
    MNetEvent(const MNetEvent &) = delete;
    MNetEvent& operator=(const MNetEvent &) = delete;
public:
    void SetFD(int fd);
    int  GetFD() const;
    void SetEventLoop(MNetEventLoop *p_event_loop);
    MNetEventLoop* GetEventLoop();
    void SetReadCallback(const std::function<void ()> &read_cb);
    std::function<void ()>& GetReadCallback();
    void SetWriteCallback(const std::function<void ()> &write_cb);
    std::function<void ()>& GetWriteCallback();
    void SetErrorCallback(const std::function<void (MError)> &error_cb);
    std::function<void (MError)>& GetErrorCallback();

    MError EnableEvents(int events);
    MError DisableEvents();
public:
    void OnReadCallback();
    void OnWriteCallback();
    void OnErrorCallback(MError err);
private:
    int fd_;
    MNetEventLoop *p_event_loop_;
    std::function<void ()> read_cb_;
    std::function<void ()> write_cb_;
    std::function<void (MError)> error_cb_;
    bool events_actived_;
};

#endif
