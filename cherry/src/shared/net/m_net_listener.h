#ifndef _M_NET_LISTENER_H_
#define _M_NET_LISTENER_H_

#include <net/m_net_event.h>

class MSocket;
class MNetEventLoop;

class MNetListener
{
public:
    explicit MNetListener(MSocket *p_sock, MNetEventLoop *p_event_loop
        , const std::function<void (MSocket*)> &accept_cb, const std::function<void (MError)> &error_cb
        , bool need_free_sock, size_t single_accept_count = 128);
    ~MNetListener();
    MNetListener(const MNetListener &) = delete;
    MNetListener& operator=(const MNetListener &) = delete;
public:
    void SetSocket(MSocket *p_sock);
    MSocket* GetSocket();
    MNetEvent& GetEvent();
    void SetEventLoop(MNetEventLoop *p_event_loop);
    MNetEventLoop* GetEventLoop();
    void SetAcceptCallback(const std::function<void (MSocket*)> &accept_cb);
    std::function<void (MSocket*)>& GetAcceptCallback();
    void SetErrorCallback(const std::function<void (MError)> &error_cb);
    std::function<void (MError)>& GetErrorCallback();
    void SetNeedFreeSock(bool need);
    bool GetNeedFreeSock() const;
    void SetSingleAcceptCount(size_t count);
    size_t GetSingleAcceptCount() const;

    MError EnableAccept(bool enable);
public:
    void OnAcceptCallback();
    void OnErrorCallback(MError err);
private:
    MSocket *p_sock_;
    MNetEvent event_;
    MNetEventLoop *p_event_loop_;
    std::function<void (MSocket*)> accept_cb_;
    std::function<void (MError)> error_cb_;
    bool need_free_sock_;
    size_t single_accept_count_;
};

#endif
