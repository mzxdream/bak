#ifndef _M_THREAD_H_
#define _M_THREAD_H_

#include <pthread.h>
#include <string>
#include <util/m_errno.h>
#include <util/m_type_define.h>

class MThread
{
protected:
    MThread();
    ~MThread();
    MThread(const MThread &) = delete;
    MThread& operator=(const MThread &) = delete;
public:
    MError Start();
    void Stop();
    MError Join();
    MError StopAndJoin();
    m_thread_t GetPID() const;
    static m_thread_t GetCurrentPID();
private:
    virtual bool _BeforeRun() { return true; }
    virtual void _AfterRun() {}
    virtual void _Run() = 0;
private:
    static void* ThreadMain(void *p_param);
private:
    m_thread_t tid_;
    bool stop_flag_;
};

#endif
