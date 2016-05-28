#ifndef _M_TIMEOUT_EVENT_H_
#define _M_TIMEOUT_EVENT_H_

#include <util/m_errno.h>
#include <event/m_event_loop.h>
#include <functional>

class MTimeoutEvent
    :public MTimerEventBase
{
public:
    MTimeoutEvent();
    virtual ~MTimeoutEvent();
    MTimeoutEvent(const MTimeoutEvent &) = delete;
    MTimeoutEvent& operator=(const MTimeoutEvent &) = delete;
public:
    MError Init(MEventLoop *p_event_loop);
    void Clear();
    MError EnableEvent(const std::function<void ()> &cb, int timeout, int repeated = 0);
    MError DisableEvent();
private:
    virtual void _OnCallback() override;
private:
    MEventLoop *p_event_loop_;
    std::function<void ()> cb_;
    int timeout_;
    int repeated_;
};

#endif
