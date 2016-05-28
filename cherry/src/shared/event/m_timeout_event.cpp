#include <event/m_timeout_event.h>

MTimeoutEvent::MTimeoutEvent()
    :p_event_loop_(nullptr)
    ,timeout_(0)
    ,repeated_(0)
{
}

MTimeoutEvent::~MTimeoutEvent()
{
    Clear();
}

MError MTimeoutEvent::Init(MEventLoop *p_event_loop)
{
    if (!p_event_loop)
    {
        return MError::Invalid;
    }
    MError err = this->MTimerEventBase::Init(p_event_loop);
    if (err != MError::No)
    {
        return err;
    }
    p_event_loop_ = p_event_loop;
    return MError::No;
}

void MTimeoutEvent::Clear()
{
    this->MTimerEventBase::Clear();
}

MError MTimeoutEvent::EnableEvent(const std::function<void ()> &cb, int timeout, int repeated = 0)
{
    if (!cb || timeout <= 0)
    {
        return MError::Invalid;
    }
    MError err = DisableEvent();
    if (err != MError::No)
    {
        return err;
    }
    cb_ = cb;
    timeout_ = timeout;
    repeated_ = repeated;
    err = this->MTimerEventBase::EnableEvent(p_event_loop_->GetTime() + timeout_);
    return err;
}

MError MTimeoutEvent::DisableEvent()
{
    return this->MTimerEventBase::DisableEvent();
}

void MTimeoutEvent::_OnCallback()
{
    cb_();
    if (repeated_ != 0)
    {
        if (repeated_ > 0)
        {
            --repeated_;
        }
        this->MTimerEventBase::EnableEvent(p_event_loop_->GetTime() + timeout_);
    }
}
