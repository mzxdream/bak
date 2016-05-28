#ifndef _M_BEFORE_IDLE_EVENT_H_
#define _M_BEFORE_IDLE_EVENT_H_

#include <util/m_errno.h>
#include <event/m_event_base.h>
#include <functional>

class MBeforeIdleEvent
{
public:
    MBeforeIdleEvent();
    virtual ~MBeforeIdleEvent();
    MBeforeIdleEvent(const MBeforeIdleEvent &) = delete;
    MBeforeIdleEvent& operator=(const MBeforeIdleEvent &) = delete;
public:
    MError Init(MEventLoop *p_event_loop);
    void Clear();
    MError EnableEvent(const std::function<void ()> &cb, int repeated = 0);
    MError DisableEvent();
private:
    virtual void _OnCallback() override;
private:
    std::function<void ()> cb_;
    int repeated_;
};

#endif
