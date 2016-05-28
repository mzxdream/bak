#include <event/m_event_loop.h>
#include <fcntl.h>
#include <util/m_logger.h>
#include <util/m_time.h>
#include <unistd.h>

MEventLoop::MEventLoop()
    :epoll_fd_(-1)
    ,cur_time_(0)
    ,interrupter_{-1, -1}
{
}

MEventLoop::~MEventLoop()
{
    Clear();
}

MError MEventLoop::AddInterrupt()
{
    if (pipe(interrupter_) != 0)
    {
        MLOG(MGetLibLogger(), MERR, "create pipe failed, errno:", errno);
        return MError::Unknown;
    }
    fcntl(interrupter_[0], F_SETFL, O_NONBLOCK);
    fcntl(interrupter_[1], F_SETFL, O_NONBLOCK);
    epoll_event ee;
    ee.events = EPOLLIN | EPOLLERR | EPOLLET;
    ee.data.ptr = interrupter_;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, interrupter_[0], &ee) == -1)
    {
        MLOG(MGetLibLogger(), MERR, "epoll ctl failed, errno:", errno);
        return MError::Unknown;
    }
    int tmp = 1;
    write(interrupter_[1], &tmp, sizeof(tmp));
    return MError::No;
}

MError MEventLoop::Init()
{
    if ((epoll_fd_ = epoll_create1(EPOLL_CLOEXEC)) == -1)
    {
        MLOG(MGetLibLogger(), MERR, "create epoll failed, errno:", errno);
        return MError::Unknown;
    }
    MError err = AddInterrupt();
    if (err != MError::No)
    {
        return err;
    }
    UpdateTime();
    io_events_.resize(1024);
    return MError::No;
}

void MEventLoop::Clear()
{
    if (epoll_fd_ >= 0)
    {
        close(epoll_fd_);
        epoll_fd_ = -1;
    }
    if (interrupter_[1] != interrupter_[0]
        && interrupter_[1] >= 0)
    {
        close(interrupter_[1]);
        interrupter_[1] = -1;
    }
    if (interrupter_[0] >= 0)
    {
        close(interrupter_[0]);
        interrupter_[0] = -1;
    }
}

int64_t MEventLoop::GetTime() const
{
    return cur_time_;
}

void MEventLoop::UpdateTime()
{
    cur_time_ = MTime::GetTime();
}

MError MEventLoop::AddIOEvent(unsigned events, MIOEventBase *p_event)
{
    if (!p_event)
    {
        MLOG(MGetLibLogger(), MERR, "event is Invalid");
        return MError::Invalid;
    }
    events |= p_event->GetEvents();
    if (events & (MIOEVENT_IN | MIOEVENT_OUT | MIOEVENT_RDHUP))
    {
        MLOG(MGetLibLogger(), MERR, "events is not in out or rdhup");
        return MError::Invalid;
    }
    int op = EPOLL_CTL_ADD;
    if (p_event->IsActived())
    {
        op = EPOLL_CTL_MOD;
    }
    epoll_event ee;
    ee.events = events;
    ee.data.ptr = p_event;
    if (epoll_ctl(epoll_fd_, op, p_event->GetFD(), &ee) == -1)
    {
        MLOG(MGetLibLogger(), MERR, "epoll add failed errno:", errno);
        return MError::Unknown;
    }
    p_event->SetActived(true);
    p_event->SetEvents(events);
    return MError::No;
}

MError MEventLoop::DelIOEvent(unsigned events, MIOEventBase *p_event)
{
    if (!p_event)
    {
        MLOG(MGetLibLogger(), MERR, "event is Invalid");
        return MError::Invalid;
    }
    events = p_event->GetEvents() & ~events;
    if (!p_event->IsActived())
    {
        p_event->SetEvents(events);
        return MError::No;
    }
    int op = EPOLL_CTL_DEL;
    if (events & (MIOEVENT_IN | MIOEVENT_OUT | MIOEVENT_RDHUP))
    {
        op = EPOLL_CTL_MOD;
    }
    epoll_event ee;
    ee.events = events;
    ee.data.ptr = p_event;
    if (epoll_ctl(epoll_fd_, op, p_event->GetFD(), &ee) == -1)
    {
        MLOG(MGetLibLogger(), MERR, "epoll del failed errno:", errno);
        return MError::Unknown;
    }
    if (op == EPOLL_CTL_DEL)
    {
        p_event->SetActived(false);
    }
    p_event->SetEvents(events);
    return MError::No;
}

MError MEventLoop::AddTimerEvent(int64_t start_time, MTimerEventBase *p_event)
{
    if (!p_event)
    {
        MLOG(MGetLibLogger(), MERR, "event is Invalid");
        return MError::Invalid;
    }
    if (p_event->IsActived())
    {
        return MError::No;
    }
    auto ret = timer_events_.insert(std::make_pair(start_time, p_event));
    if (!ret.second)
    {
        MLOG(MGetLibLogger(), MERR, "insert timer event failed");
        return MError::Unknown;
    }
    p_event->SetActived(true);
    p_event->SetLocation(ret.first);
    return MError::No;
}

MError MEventLoop::DelTimerEvent(MTimerEventBase *p_event)
{
    if (!p_event)
    {
        MLOG(MGetLibLogger(), MERR, "event is Invalid");
        return MError::Invalid;
    }
    if (!p_event->IsActived())
    {
        return MError::No;
    }
    timer_events_.erase(p_event->GetLocation());
    p_event->SetActived(false);
    return MError::No;
}

MError MEventLoop::AddBeforeEvent(MBeforeEventBase *p_event)
{
    if (!p_event)
    {
        MLOG(MGetLibLogger(), MERR, "event is Invalid");
        return MError::Invalid;
    }
    if (p_event->IsActived())
    {
        return MError::No;
    }
    before_events_.push_back(p_event);
    p_event->SetActived(true);
    auto iter = before_events_.end();
    p_event->SetLocation(--iter);
    return MError::No;
}

MError MEventLoop::DelBeforeEvent(MBeforeEventBase *p_event)
{
    if (!p_event)
    {
        MLOG(MGetLibLogger(), MERR, "event is Invalid");
        return MError::Invalid;
    }
    if (!p_event->IsActived())
    {
        return MError::No;
    }
    before_events_.erase(p_event->GetLocation());
    p_event->SetActived(false);
    return MError::No;
}

MError MEventLoop::AddAfterEvent(MAfterEventBase *p_event)
{
    if (!p_event)
    {
        MLOG(MGetLibLogger(), MERR, "event is Invalid");
        return MError::Invalid;
    }
    if (p_event->IsActived())
    {
        return MError::No;
    }
    after_events_.push_back(p_event);
    p_event->SetActived(true);
    auto iter = after_events_.end();
    p_event->SetLocation(--iter);
    return MError::No;
}

MError MEventLoop::DelAfterEvent(MAfterEventBase *p_event)
{
    if (!p_event)
    {
        MLOG(MGetLibLogger(), MERR, "event is Invalid");
        return MError::Invalid;
    }
    if (!p_event->IsActived())
    {
        return MError::No;
    }
    after_events_.erase(p_event->GetLocation());
    p_event->SetActived(false);
    return MError::No;
}

MError MEventLoop::Interrupt()
{
    epoll_event ee;
    ee.events = EPOLLIN | EPOLLERR | EPOLLET;
    ee.data.ptr = interrupter_[0];
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, interrupter_[0], &ee) == -1)
    {
        MLOG(MGetLibLogger(), MERR, "epoll ctl failed errno:", errno);
        return MError::Unknown;
    }
    return MError::No;
}

MError MEventLoop::DispatchEvent()
{
    return MError::No;
}

MError MEventLoop::DispatchEventOnce(int timeout)
{
    MError err = DispatchBeforeEvent();
    if (err != MError::No)
    {
        MLOG(MGetLibLogger(), MERR, "DispatchBeforeEvent failed");
        return err;
    }
    if (timeout < 0)
    {
        if (timer_events_.empty())
        {
            err = DispatchIOEvent(true, 0);
        }
        else
        {
            err = DispatchIOEvent(false, timer_events_.begin()->first);
        }
    }
    else
    {
        if (timer_events_.empty())
        {
            err = DispatchIOEvent(false, cur_time_ + timeout);
        }
        else
        {
            err = DispatchIOEvent(false, std::min(timer_events_.begin()->first, cur_time_ + timeout));
        }
    }
    if (err != MError::No)
    {
        MLOG(MGetLibLogger(), MERR, "DispatchIOEvent failed");
        return err;
    }
    UpdateTime();
    err = DispatchTimerEvent();
    if (err != MError::No)
    {
        MLOG(MGetLibLogger(), MERR, "DispatchTimerEvent failed");
        return err;
    }
    err = DispatchAfterEvent();
    if (err != MError::No)
    {
        MLOG(MGetLibLogger(), MERR, "DispatchAfterEvent failed");
        return err;
    }
    return MError::No;
}

MError MEventLoop::DispatchIOEvent(bool forever, int64_t outdate)
{
    int timeout = -1;
    bool interrupt = false;
    int count = 48;
    do
    {
        if (!forever)
        {
            timeout = std::max(0, static_cast<int>(outdate - cur_time_));
        }
        int nevents = epoll_wait(epoll_fd_, &io_events_[0], io_events_.size(), timeout);
        if (nevents == -1)
        {
            if (errno != EINTR)
            {
                MLOG(MGetLibLogger(), MERR, "epoll wait failed errno:", errno);
                return MError::Unknown;
            }
        }
        else
        {
            for (int i = 0; i < nevents; ++i)
            {
                void *p_tmp = io_events_[i].data.ptr;
                if (!p_tmp)
                {
                    continue;
                }
                if (p_tmp == interrupter_)
                {
                    interrupt = true;
                    continue;
                }
                MIOEventBase *p_event = static_cast<MIOEventBase*>(p_tmp);
                p_event->OnCallback(io_events_[i].events);
            }
        }
        if (interrupt)
        {
            break;
        }
        if (nevents > 0)
        {
            if (static_cast<size_t>(nevents) == io_events_.size() && (--count > 0))
            {
                outdate = cur_time_;
                continue;
            }
            break;
        }
        UpdateTime();
        if (!forever && (outdate <= cur_time_))
        {
            break;
        }
    } while (true);
    return MError::No;
}

MError MEventLoop::DispatchTimerEvent()
{
    auto it = timer_events_.begin();
    while (it != timer_events_.end()
        && it->first <= cur_time_)
    {
        MTimerEventBase *p_event = it->second;
        timer_events_.erase(it);
        if (p_event)
        {
            p_event->SetActived(false);
            p_event->OnCallback();
        }
        it = timer_events_.begin();
    }
    return MError::No;
}

MError MEventLoop::DispatchBeforeEvent()
{
    std::list<MBeforeEventBase*> events;
    events.swap(before_events_);
    auto it = events.begin();
    while (it != events.end())
    {
        MBeforeEventBase *p_event = *it;
        it = events.erase(it);
        if (p_event)
        {
            p_event->SetActived(false);
            p_event->OnCallback();
        }
    }
    return MError::No;
}

MError MEventLoop::DispatchAfterEvent()
{
    std::list<MAfterEventBase*> events;
    events.swap(after_events_);
    auto it = events.begin();
    while (it != events.end())
    {
        MAfterEventBase *p_event = *it;
        it = events.erase(it);
        if (p_event)
        {
            p_event->SetActived(false);
            p_event->OnCallback();
        }
    }
    return MError::No;
}
