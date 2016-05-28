#include <event/m_before_idle_event.h>

MBeforeIdleEvent::MBeforeIdleEvent()
    :repeated_(0)
{
}

MBeforeIdleEvent::~MBeforeIdleEvent()
{
    Clear();
}

MError MBeforeIdleEvent::Init(MEventLoop *p_event_loop)
{
    if (!p_event_loop)
    {
        return MError::Invalid;
    }
    return this->MTimerEventBase::Init(p_event_loop);
}

void MBeforeIdleEvent::Clear()
{
    this->MTimerEventBase::Clear();
}

MError MBeforeIdleEvent::EnableEvent(const std::function<void ()> &cb, int repeated)
{
    if (!cb)
    {
        return MError::Invalid;
    }
    MError err = DisableEvent();
    if (err != MError::No)
    {
        return err;
    }
    cb_ = cb;
    repeated_ = repeated;
    return this->MTimerEventBase::EnableEvent();
}

MError MBeforeIdleEvent::DisableEvent()
{
    return this->MTimerEventBase::DisableEvent();
}

void MBeforeIdleEvent::_OnCallback()
{
    cb_();
    if (repeated_ != 0)
    {
        if (repeated_ > 0)
        {
            --repeated_;
        }
        this->MTimerEventBase::EnableEvent();
    }
}
