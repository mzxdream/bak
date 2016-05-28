#include <net_manager.h>

NetManager::NetManager()
{
}

NetManager::~NetManager()
{
    Close();
}

bool NetManager::Init(size_t thread_count, size_t session_count)
{
    for (size_t i = 0; i < thread_count; ++i)
    {
        MNetEventLoopThread *p_loop_thread = new MNetEventLoopThread();
        if (!p_loop_thread)
        {
            return false;
        }
        if (p_loop_thread->Init() != MError::No)
        {
            return false;
        }
        if (p_loop_thread->Start() != MError::No)
        {
            return false;
        }
        loop_threads_.push_back(p_loop_thread);
    }
    sessions_.resize(session_count);
    return true;
}

void NetManager::Close()
{
    for (const auto &it : loop_threads_)
    {
        if (it)
        {
            it->StopAndJoin();
        }
    }
    for (const auto &it : loop_threads_)
    {
        if (it)
        {
            it->Close();
            delete it;
        }
    }
}

MNetEventLoopThread* NetManager::GetMinEventLoopThread()
{
    if (loop_threads_.empty())
    {
        return nullptr;
    }
    MNetEventLoopThread *p_loop_thread = nullptr;
    size_t event_count = static_cast<size_t>(-1);
    for (auto &it : loop_threads_)
    {
        if (it && it->GetEventCount() < event_count)
        {
            p_loop_thread = it;
            event_count = it->GetEventCount();
        }
    }
    return p_loop_thread;
}

bool NetManager::AddListener(const std::string &ip, unsigned port)
{
    MNetEventLoopThread *p_loop_thread = GetMinEventLoopThread();
    if (!p_loop_thread)
    {
        return false;
    }
    MSocket *p_sock = new MSocket();
    if (!p_sock)
    {
        return false;
    }
    if (p_sock->CreateNonblockReuseAddrListener(ip, port) != MError::No)
    {
        delete p_sock;
        return false;
    }
    MNetListener *p_listener = new MNetListener(p_sock, &p_loop_thread->GetEventLoop(), nullptr, nullptr, true);
    if (!p_listener)
    {
        delete p_sock;
        return false;
    }
    p_listener->SetAcceptCallback(&NetManager::OnListenerAcceptCallback, this, p_listener, std::placeholders::_1);
    p_listener->SetErrorCallback(&NetManager::OnListenerErrorCallback, this, p_listener, std::placeholders::_1);
    listeners_.push_back(p_listener);
    p_loop_thread->AddCallback(std::bind(&NetManager::OnListenerEnableCallback, this, p_listener));
    if (p_loop_thread->Interrupt() != MError::No)
    {
        //print error
        return false;
    }
    return true;
}

bool NetManager::SendSessionMsg(int64_t id, char *p_buf, size_t len)
{
}

void NetManager::OnListenerAcceptCallback(MNetListener *p_listener, MSocket *p_sock)
{
    if (!p_listener || !p_sock)
    {
        return;
    }
    MNetEventLoopThread *p_loop_thread = GetMinEventLoopThread();
    if (!p_loop_thread)
    {
        delete p_sock;
        return;
    }
    NetSession *p_session = new NetSession(p_sock, p_loop_thread, nullptr, nullptr, 4096, 2048);
    if (!p_sock)
    {
        delete p_sock;
        return;
    }
    p_session->SetReadCallback(std::bind(&NetManager::OnSessionReadCallback, this, p_session, std::placeholders::_1, std::placeholders::_2));
    p_session->SetErrorCallback(std::bind(&NetManager::OnSessionInitErrorCallback, this, p_session, std::placeholders::_1));
    p_loop_thread->AddCallback(std::bind(&NetManager::OnSessionEnableCallback, this, p_session));
    if (p_loop_thread->Interrupt() != MError::No)
    {
        //print error
        return;
    }
}

void NetManager::OnListenerErrorCallback(MNetListener *p_listener, MError err)
{
    //print error
}

void NetManager::OnListenerEnableCallback(MNetListener *p_listener)
{
    if (!p_listener)
    {
        return;
    }
    if (p_listener->EnableAccept(true) != MError::No)
    {
        //print error
    }
}

void NetManager::OnSessionReadCallback(NetSession *p_session, char *p_buf, size_t len)
{
    if (!p_session || !p_buf)
    {
        return;
    }
    //TODO
    // p_session->GetID(), p_buf, len
}

void NetManager::OnSessionInitErrorCallback(NetSession *p_session, MError err)
{
    if (!p_session)
    {
        return;
    }
    delete p_session;
}

void NetManager::OnSessionErrorCallback(NetSession *p_session, MError err)
{
    if (!p_session)
    {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(session_mtx_);
        auto it = sessions_.find(p_session->GetID());
        if (it != sessions_.end())
        {
            sessions_.erase(it);
        }
    }
    int64_t id = p_session->GetID();
    delete p_session;
    /////TODO
}

void NetManager::OnSessionEnableCallback(NetSession *p_session)
{
    if (!p_session)
    {
        return;
    }
    MSocket *p_sock = p_session->GetSocket();
    if (!p_sock)
    {
        delete p_session;
        return;
    }
    p_session->SetErrorCallback(std::bind(&NetManager::OnSessionErrorCallback, this, p_session, std::placeholders::_1));
    if (p_session->EnableReadWrite(true) != MError::No)
    {
        delete p_session;
        return;
    }
    {
        std::lock_guard<std::mutex> lock(session_mtx_);
        if (++session_index_ >= 2^15)
        {
            session_index_ = 1;
        }
        int64_t session_id = session_index_ << 16 + p_sock->GetHandler();
        if (sessions_.find(session_id) != sessions_.end())
        {
            delete p_session;
            return;
        }
        p_session->SetID(session_id);
        sessions_[session_id] = p_session;
    }
}
