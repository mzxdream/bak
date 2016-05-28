#ifndef _M_EVENT_LOOP_H_
#define _M_EVENT_LOOP_H_

#include <map>
#include <vector>
#include <list>
#include <util/m_errno.h>
#include <util/m_type_define.h>
#include <sys/epoll.h>
#include <event/m_event_base.h>

typedef std::multimap<int64_t, MTimerEventBase*>::iterator MTimerEventLocation;
typedef std::list<MBeforeEventBase*>::iterator MBeforeEventLocation;
typedef std::list<MAfterEventBase*>::iterator MAfterEventLocation;

class MEventLoop
{
public:
    MEventLoop();
    ~MEventLoop();
    MEventLoop(const MEventLoop &) = delete;
    MEventLoop& operator=(const MEventLoop &) = delete;
public:
    MError Init();
    void Clear();

    int64_t GetTime() const;
    void UpdateTime();

    MError AddIOEvent(unsigned events, MIOEventBase *p_event);
    MError DelIOEvent(unsigned events, MIOEventBase *p_event);

    MError AddTimerEvent(int64_t start_time, MTimerEventBase *p_event);
    MError DelTimerEvent(MTimerEventBase *p_event);

    MError AddBeforeEvent(MBeforeEventBase *p_event);
    MError DelBeforeEvent(MBeforeEventBase *p_event);

    MError AddAfterEvent(MAfterEventBase *p_event);
    MError DelAfterEvent(MAfterEventBase *p_event);

    MError Interrupt();
    MError DispatchEvent();
    MError DispatchEventOnce(int timeout = -1);
private:
    MError AddInterrupt();
    MError DispatchIOEvent(bool forever, int64_t outdate);
    MError DispatchTimerEvent();
    MError DispatchBeforeEvent();
    MError DispatchAfterEvent();
private:
    int epoll_fd_;
    int64_t cur_time_;
    int interrupter_[2];
    std::vector<epoll_event> io_events_;
    std::multimap<int64_t, MTimerEventBase*> timer_events_;
    std::list<MBeforeEventBase*> before_events_;
    std::list<MAfterEventBase*> after_events_;
};

#endif
