#include <net/net_manager.h>
#include <iostream>
#include <cstring>
#include <util/m_logger.h>

NetManager::NetManager()
{
}

NetManager::~NetManager()
{
    Close();
}

bool NetManager::Init(size_t work_count)
{
    work_list_.resize(work_count);
    for (auto &work : work_list_)
    {
        work = new MNetEventLoopThread();
        if (!work
            || work->Init() != MError::No
            || work->Start() != MError::No)
        {
            return false;
        }
    }
    return true;
}

void NetManager::Close()
{
    for (const auto &work : work_list_)
    {
        delete work;
    }
    for (const auto &listener : listener_list_)
    {
        delete listener;
    }
    for (const auto &session : session_list_)
    {
        if (session && session->p_connector)
        {
            delete session->p_connector;
        }
        delete session;
    }
}

bool NetManager::AddListener(const std::string &ip, unsigned short port)
{
    MSocket *p_sock = new MSocket();
    if (!p_sock)
    {
        return false;
    }
    if (p_sock->CreateNonblockReuseAddrListener(ip, port, 128) != MError::No)
    {
        delete p_sock;
        return false;
    }
    MNetEventLoopThread *p_net_thread = GetMinEventsThread();
    MNetListener *p_listener = new MNetListener(p_sock, &(p_net_thread->GetEventLoop()), nullptr, nullptr, true, 5);
    if (!p_listener)
    {
        return false;
    }
    listener_list_.push_back(p_listener);

    p_listener->SetAcceptCallback(std::bind(&NetManager::OnConnectCallback, this, p_listener, std::placeholders::_1));
    p_listener->SetErrorCallback(std::bind(&NetManager::OnListenerErrorCallback, this, listener_list_.size()-1, std::placeholders::_1));

    MError err = p_listener->EnableAccept(true);
    if (err != MError::No)
    {
        return false;
    }
    return true;
}

void OnCloseSessionCallback(NetSession *p_session)
{
    if (p_session)
    {
        if (p_session->p_connector)
        {
            delete p_session->p_connector;
        }
        delete p_session;
    }
}

void NetManager::CloseSession(NetSession *p_session)
{
    if (!p_session)
    {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(session_mutex_);
        auto it = session_list_.find(p_session);
        if (it != session_list_.end())
        {
            session_list_.erase(it);
        }
    }
    if (p_session->p_loop_thread)
    {
        p_session->p_loop_thread->AddCallback(std::bind(OnCloseSessionCallback, p_session));
    }
    else
    {
        delete p_session;
    }
}

void OnWriteSessionCallback(NetSession *p_session, char *p_buf, size_t len)
{
    uint16_t size = htons(static_cast<uint16_t>(len));
    p_session->p_connector->WriteBuf(static_cast<char*>(static_cast<void*>(&size)), sizeof(size));
    p_session->p_connector->WriteBuf(p_buf, len);
    delete p_buf;
}

void NetManager::WriteSession(NetSession *p_session, char *p_buf, size_t len)
{
    if (!p_session)
    {
        return;
    }
    p_session->p_loop_thread->AddCallback(std::bind(OnWriteSessionCallback, p_session, p_buf, len));
    p_session->p_loop_thread->Interrupt();
}

void NetManager::WriteAll(const char *p_buf, size_t len)
{
    std::lock_guard<std::mutex> lock(session_mutex_);
    for (auto &it : session_list_)
    {
        char *p_tmp = new char[len];
        memcpy(p_tmp, p_buf, len);
        WriteSession(it, p_tmp, len);
    }
}

void NetManager::OnReadCallback(NetSession *p_session)
{
    while (true)
    {
        if (!p_session->len_readed)
        {
            if (p_session->p_connector->ReadBuf(&p_session->len, sizeof(p_session->len)) == MError::No)
            {
                p_session->len = ntohs(p_session->len);
                p_session->len_readed = true;
            }
            else
            {
                return;
            }
        }
        std::string str;
        str.resize(p_session->len);
        if (p_session->p_connector->ReadBuf(&str[0], str.size()) == MError::No)
        {
            p_session->len_readed = false;
            std::cout << p_session->p_connector->GetSocket()->GetRemoteIP() << " "
            << p_session->p_connector->GetSocket()->GetRemotePort() << " "
                << str << std::endl;
        }
        else
        {
            return;
        }
    }
}

void NetManager::OnCloseCallback(NetSession *p_session, MError err)
{
    std::cout << p_session->p_connector->GetSocket()->GetRemoteIP() << " "
        << p_session->p_connector->GetSocket()->GetRemotePort() << " "
        << " has socket disconnect:" << static_cast<int>(err) << std::endl;
    std::lock_guard<std::mutex> lock(session_mutex_);
    auto it = session_list_.find(p_session);
    if (it != session_list_.end())
    {
        session_list_.erase(it);
    }
    delete p_session->p_connector;
    delete p_session;
}

void NetManager::OnConnectCallback(MNetListener *p_listener, MSocket *p_sock)
{
    if (!p_sock)
    {
        return;
    }
    MNetEventLoopThread *p_loop_thread = GetMinEventsThread();
    if (!p_loop_thread)
    {
        delete p_sock;
        return;
    }
    MNetConnector *p_connector = new MNetConnector(p_sock, p_listener, &(p_loop_thread->GetEventLoop()), nullptr, nullptr, nullptr, nullptr, true, 1024, 1024);
    if (!p_connector)
    {
        delete p_sock;
        return;
    }
    NetSession *p_session = new NetSession();
    p_session->p_connector = p_connector;
    p_session->p_loop_thread = p_loop_thread;
    p_session->len_readed = false;
    p_session->len = 0;

    std::lock_guard<std::mutex> lock(session_mutex_);
    session_list_.insert(p_session);
    p_connector->SetReadCallback(std::bind(&NetManager::OnReadCallback, this, p_session));
    p_connector->SetErrorCallback(std::bind(&NetManager::OnCloseCallback, this, p_session, std::placeholders::_1));
    p_connector->EnableReadWrite(true);
    std::cout << p_session->p_connector->GetSocket()->GetRemoteIP() << " "
        << p_session->p_connector->GetSocket()->GetRemotePort() << " "
        << " has socket connect" << std::endl;

}

void NetManager::OnListenerErrorCallback(size_t pos, MError err)
{
    if (listener_list_.size() >= pos)
    {
        return;
    }
    MNetListener *p_listener = listener_list_[pos];
    if (!p_listener)
    {
        return;
    }
    std::cout << "listener ip:" << p_listener->GetSocket()->GetBindIP()
        << " port:" << p_listener->GetSocket()->GetBindPort() << std::endl;
}

MNetEventLoopThread* NetManager::GetMinEventsThread()
{
    if (work_list_.empty())
    {
        return nullptr;
    }
    MNetEventLoopThread *p_loop_thread = work_list_[0];
    size_t count = work_list_[0]->GetEventCount();
    for (size_t i = 1; i < work_list_.size(); ++i)
    {
        if (count > work_list_[i]->GetEventCount())
        {
            count = work_list_[i]->GetEventCount();
            p_loop_thread = work_list_[i];
        }
    }
    return p_loop_thread;
}
