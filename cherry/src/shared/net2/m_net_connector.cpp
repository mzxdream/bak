#include <net/m_net_connector.h>
#include <net/m_socket.h>
#include <net/m_net_event_loop.h>
#include <util/m_logger.h>

MNetConnector::MNetConnector(MSocket *p_sock, MNetEventLoop *p_event_loop
        , const std::function<void ()> &connect_cb, const std::function<void ()> &read_cb, const std::function<void ()> &write_complete_cb, const std::function<void (MError)> &error_cb
        , bool need_free_sock, size_t read_len, size_t write_len)
    :p_sock_(p_sock)
    ,event_(p_sock ? p_sock->GetHandler() : -1, p_event_loop, nullptr, nullptr, nullptr)
    ,connect_cb_(connect_cb)
    ,read_cb_(read_cb)
    ,write_complete_cb_(write_complete_cb)
    ,error_cb_(error_cb)
    ,need_free_sock_(need_free_sock)
    ,read_buffer_(read_len)
    ,write_buffer_(write_len)
    ,write_ready_(true)
{
}

MNetConnector::~MNetConnector()
{
    if (need_free_sock_ && p_sock_)
    {
        delete p_sock_;
    }
}

void MNetConnector::SetSocket(MSocket *p_sock)
{
    p_sock_ = p_sock;
    event_.SetFD(p_sock_ ? p_sock_->GetHandler() : -1);
}

MSocket* MNetConnector::GetSocket()
{
    return p_sock_;
}

MNetEvent& MNetConnector::GetEvent()
{
    return event_;
}

void MNetConnector::SetEventLoop(MNetEventLoop *p_event_loop)
{
    event_.SetEventLoop(p_event_loop);
}

MNetEventLoop* MNetConnector::GetEventLoop()
{
    return event_.GetEventLoop();
}

void MNetConnector::SetConnectCallback(const std::function<void ()> &connect_cb)
{
    connect_cb_ = connect_cb;
}

void MNetConnector::SetReadCallback(const std::function<void ()> &read_cb)
{
    read_cb_ = read_cb;
}

std::function<void ()>& MNetConnector::GetReadCallback()
{
    return read_cb_;
}

void MNetConnector::SetWriteCompleteCallback(const std::function<void ()> &write_complete_cb)
{
    write_complete_cb_ = write_complete_cb;
}

std::function<void ()>& MNetConnector::GetWriteCompleteCallback()
{
    return write_complete_cb_;
}

void MNetConnector::SetErrorCallback(const std::function<void (MError)> &error_cb)
{
    error_cb_ = error_cb;
}

std::function<void (MError)>& MNetConnector::GetErrorCallback()
{
    return error_cb_;
}

void MNetConnector::SetNeedFreeSock(bool need)
{
    need_free_sock_ = need;
}

bool MNetConnector::GetNeedFreeSock() const
{
    return need_free_sock_;
}

MError MNetConnector::EnableReadWrite(bool enable)
{
    if (enable)
    {
        event_.SetReadCallback(std::bind(&MNetConnector::OnReadCallback, this));
        event_.SetWriteCallback(std::bind(&MNetConnector::OnWriteCallback, this));
        event_.SetErrorCallback(std::bind(&MNetConnector::OnErrorCallback, this, std::placeholders::_1));
        return event_.EnableEvents(M_NET_EVENT_READ|M_NET_EVENT_LEVEL);
    }
    else
    {
        return event_.DisableEvents();
    }
}

MError MNetConnector::Connect(const std::string &ip, unsigned port)
{
    event_.SetReadCallback(connect_cb_);
    event_.SetErrorCallback(std::bind(&MNetConnector::OnErrorCallback, this, std::placeholders::_1));
    MError err = event_.EnableEvents(M_NET_EVENT_READ|M_NET_EVENT_LEVEL);
    if (err != MError::No)
    {
        return err;
    }
    err = p_sock_->Connect(ip, port);
    if (err != MError::No
        && err != MError::InProgress)
    {
        return MError::Unknown;
    }
    return MError::No;
}

MError MNetConnector::ReadBuf(void *p_buf, size_t len)
{
    if (!read_buffer_.Peek(p_buf, len))
    {
        return MError::Underflow;
    }
    return MError::No;
}

size_t MNetConnector::GetReadBufLen() const
{
    return read_buffer_.GetLen();
}

MError MNetConnector::WriteBuf(const char *p_buf, size_t len)
{
    if (write_ready_)
    {
        std::pair<int, MError> ret = p_sock_->Send(p_buf, len);
        if (ret.second == MError::No)
        {
            if (static_cast<size_t>(ret.first) < len)
            {
                if (!write_buffer_.Append(p_buf + ret.first, len - ret.first))
                {
                    return MError::Overflow;
                }
                write_ready_ = false;
                return event_.EnableEvents(M_NET_EVENT_READ|M_NET_EVENT_WRITE|M_NET_EVENT_LEVEL);
            }
            if (write_complete_cb_)
            {
                write_complete_cb_();
            }
            return MError::No;
        }
        else if (ret.second == MError::InterruptedSysCall
            || ret.second == MError::Again)
        {
            write_ready_ = false;
            return event_.EnableEvents(M_NET_EVENT_READ|M_NET_EVENT_WRITE|M_NET_EVENT_LEVEL);
        }
        else
        {
            return MError::Unknown;
        }
    }
    else
    {
        return write_buffer_.Append(p_buf, len) ? MError::No : MError::Overflow;
    }
    return MError::No;
}

size_t MNetConnector::GetWriteBufLen() const
{
    return write_buffer_.GetLen();
}

void MNetConnector::OnReadCallback()
{
    std::pair<char*, size_t> buf;
    std::pair<int, MError> ret;
    while (true)
    {
        buf = read_buffer_.GetNextCapacity();
        if (!buf.first || buf.second == 0)
        {
            if (read_cb_)
            {
                read_cb_();
            }
            return;
        }
        ret = p_sock_->Recv(buf.first, static_cast<int>(buf.second));
        if (ret.second == MError::No)
        {
            if (ret.first == 0)
            {
                event_.DisableEvents();
                OnErrorCallback(MError::Disconnect);
                return;
            }
            else
            {
                if (!read_buffer_.AddEndLen(ret.first))
                {
                    OnErrorCallback(MError::Unknown);
                    return;
                }
                if (static_cast<size_t>(ret.first) < buf.second)
                {
                    if (read_cb_)
                    {
                        read_cb_();
                    }
                    return;
                }
            }
        }
        else if (ret.second == MError::InterruptedSysCall
            || ret.second == MError::Again)
        {
            if (read_cb_)
            {
                read_cb_();
            }
            return;
        }
        else
        {
            OnErrorCallback(ret.second);
            return;
        }
    }
}

void MNetConnector::OnWriteCallback()
{
    std::pair<const char*, size_t> buf;
    std::pair<int, MError> ret;
    while (true)
    {
        buf = read_buffer_.GetNextData();
        if (!buf.first || buf.second == 0)
        {
            MError err = event_.EnableEvents(M_NET_EVENT_READ|M_NET_EVENT_LEVEL);
            if (err != MError::No)
            {
                OnErrorCallback(err);
                return;
            }
            write_ready_ = true;
            if (write_complete_cb_)
            {
                write_complete_cb_();
            }
            return;
        }
        ret = p_sock_->Send(buf.first, static_cast<int>(buf.second));
        if (ret.second == MError::No)
        {
            if (!write_buffer_.AddStartLen(ret.first))
            {
                OnErrorCallback(MError::Unknown);
                return;
            }
            if (static_cast<size_t>(ret.first) < buf.second)
            {
                return;
            }
        }
        else if (ret.second == MError::InterruptedSysCall
            || ret.second == MError::Again)
        {
            return;
        }
        else
        {
            OnErrorCallback(ret.second);
            return;
        }
    }
}

void MNetConnector::OnErrorCallback(MError err)
{
    if (error_cb_)
    {
        error_cb_(err);
    }
}
