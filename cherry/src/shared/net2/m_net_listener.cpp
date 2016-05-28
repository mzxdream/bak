#include <net/m_net_listener.h>
#include <net/m_socket.h>
#include <net/m_net_event_loop.h>
#include <util/m_logger.h>

MNetListener::MNetListener(MSocket *p_sock, MNetEventLoop *p_event_loop
    , const std::function<void (MSocket*)> &accept_cb, const std::function<void (MError)> &error_cb
    , bool need_free_sock, size_t single_accept_count)
    :p_sock_(p_sock)
    ,event_(p_sock ? p_sock->GetHandler() : -1, p_event_loop, nullptr, nullptr, nullptr)
    ,p_event_loop_(p_event_loop)
    ,accept_cb_(accept_cb)
    ,error_cb_(error_cb)
    ,need_free_sock_(need_free_sock)
    ,single_accept_count_(single_accept_count)
{
}

MNetListener::~MNetListener()
{
    if (need_free_sock_ && p_sock_)
    {
        delete p_sock_;
    }
}

void MNetListener::SetSocket(MSocket *p_sock)
{
    p_sock_ = p_sock;
    event_.SetFD(p_sock_ ? p_sock_->GetHandler() : -1);
}

MSocket* MNetListener::GetSocket()
{
    return p_sock_;
}

MNetEvent& MNetListener::GetEvent()
{
    return event_;
}

void MNetListener::SetEventLoop(MNetEventLoop *p_event_loop)
{
    p_event_loop_ = p_event_loop;
    event_.SetEventLoop(p_event_loop_);
}

MNetEventLoop* MNetListener::GetEventLoop()
{
    return p_event_loop_;
}

void MNetListener::SetAcceptCallback(const std::function<void (MSocket*)> &accept_cb)
{
    accept_cb_ = accept_cb;
}

std::function<void (MSocket*)>& MNetListener::GetAcceptCallback()
{
    return accept_cb_;
}

void MNetListener::SetErrorCallback(const std::function<void (MError)> &error_cb)
{
    error_cb_ = error_cb;
}

std::function<void (MError)>& MNetListener::GetErrorCallback()
{
    return error_cb_;
}

void MNetListener::SetNeedFreeSock(bool need)
{
    need_free_sock_ = need;
}

bool MNetListener::GetNeedFreeSock() const
{
    return need_free_sock_;
}

void MNetListener::SetSingleAcceptCount(size_t count)
{
    single_accept_count_ = count;
}

size_t MNetListener::GetSingleAcceptCount() const
{
    return single_accept_count_;
}

MError MNetListener::EnableAccept(bool enable)
{
    if (enable)
    {
        event_.SetReadCallback(std::bind(&MNetListener::OnAcceptCallback, this));
        event_.SetErrorCallback(std::bind(&MNetListener::OnErrorCallback, this, std::placeholders::_1));
        return event_.EnableEvents(M_NET_EVENT_READ|M_NET_EVENT_LEVEL);
    }
    else
    {
        return event_.DisableEvents();
    }
}

void MNetListener::OnAcceptCallback()
{
    if (!accept_cb_)
    {
        return;
    }
    size_t count = single_accept_count_;
    while (count--)
    {
        MSocket *p_conn_sock = new MSocket();
        if (!p_conn_sock)
        {
            OnErrorCallback(MError::OutOfMemory);
            return;
        }
        MError err = p_sock_->Accept(*p_conn_sock);
        if (err != MError::No)
        {
            delete p_conn_sock;
            if (err != MError::Again
                && err != MError::InterruptedSysCall)
            {
                OnErrorCallback(err);
            }
            return;
        }
        accept_cb_(p_conn_sock);
    }
}

void MNetListener::OnErrorCallback(MError err)
{
    if (error_cb_)
    {
        error_cb_(err);
    }
}
