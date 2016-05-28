#include <net/m_net_event_loop_thread.h>

MNetEventLoopThread::MNetEventLoopThread(size_t single_process_events)
    :event_loop_(single_process_events)
{
}

MNetEventLoopThread::~MNetEventLoopThread()
{
    StopAndJoin();
    Close();
}

MError MNetEventLoopThread::Init()
{
    return event_loop_.Create();
}

MError MNetEventLoopThread::Close()
{
    return event_loop_.Close();
}

MError MNetEventLoopThread::Start()
{
    return MThread::Start();
}

MError MNetEventLoopThread::Stop()
{
    MThread::Stop();
    return event_loop_.Interrupt();
}

MError MNetEventLoopThread::StopAndJoin()
{
    MError err = Stop();
    if (err != MError::No)
    {
        return err;
    }
    return MThread::Join();
}

MNetEventLoop& MNetEventLoopThread::GetEventLoop()
{
    return event_loop_;
}

size_t MNetEventLoopThread::GetEventCount() const
{
    return event_loop_.GetEventCount();
}

void MNetEventLoopThread::AddCallback(const std::function<void ()> &cb)
{
    std::lock_guard<std::mutex> lock(cb_mutex_);
    cb_list_.push_back(cb);
}

MError MNetEventLoopThread::Interrupt()
{
    return event_loop_.Interrupt();
}

void MNetEventLoopThread::_Run()
{
    event_loop_.ProcessEvents();
    std::list<std::function<void ()> > cb_list;
    {
        std::lock_guard<std::mutex> lock(cb_mutex_);
        cb_list.swap(cb_list_);
    }
    for (auto &cb : cb_list)
    {
        if (cb)
        {
            cb();
        }
    }
}

