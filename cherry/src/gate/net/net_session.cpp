#include <net_session.h>

NetSession::NetSession(MSocket *p_sock, MNetEventLoopThread *p_event_loop_thread, const std::function<void (MError)> &error_cb)
    :connector_(p_sock, p_event_loop_thread ? &p_event_loop_thread->GetEventLoop() : nullptr, nullptr, nullptr, nullptr, error_cb, true, NET_READ_LEN, NET_WRITE_LEN)
    ,p_event_loop_thread_(p_event_loop_thread)
    ,recv_pkg_len_(0)
    ,recv_pkg_len_read_(false)
{
    connector_.SetReadCallback(std::bind(&NetSession::OnReadCallback, this));
    connector_.SetErrorCallback(error_cb);
    connector_.EnableReadWrite(true);
}

NetSession::~NetSession()
{
}

bool NetSession::SendNetPkg(const NetPackage &pkg)
{
    if (!p_connector_ || !p_event_loop_thread_)
    {
        return false;
    }
    std::lock_guard<std::mutex> lock(send_pkg_mtx_);
    send_pkg_list_.push_back(pkg);
    p_event_loop_thread_->AddCallback(std::bind(&NetSession::OnWriteCallback, this));
    if (send_pkg_list_.size() == 1)
    {
        p_event_loop_thread_->Interrupt();
    }
    return true;
}

void NetSession::Update()
{
    std::list<NetPackage> pkg_list;
    {
        std::lock_guard<std::mutex> lock(recv_pkg_mtx_);
        pkg_list.swap(recv_pkg_list_);
    }
    //handler pkg;
}

void NetSession::OnReadCallback()
{
    if (!recv_pkg_len_read_)
    {
        if (!connector_.ReadBuf(&recv_pkg_len_, sizeof(recv_pkg_len_)))
        {
            return;
        }
        recv_pkg_len_read_ = true;
    }
    if (recv_pkg_len_read_)
    {
        if (connector_.GetReadBufLen() < recv_pkg_len_)
        {
            return;
        }
    }
}

void NetSession::OnWriteCallback()
{
    connector_.SendNetPkg();
}
