#include <thread/m_thread.h>
#include <util/m_string.h>
#include <util/m_logger.h>

MThread::MThread()
    :tid_(0)
    ,stop_flag_(false)
{
}

MThread::~MThread()
{
    StopAndJoin();
}

MError MThread::Start()
{
    stop_flag_ = false;
    int err = pthread_create(&tid_, nullptr, &ThreadMain, this);
    if (err != 0)
    {
        MLOG(MGetLibLogger(), MERR, "create thread failed error:", err);
        return MError::Unknown;
    }
    return MError::No;
}

void MThread::Stop()
{
    stop_flag_ = true;
}

MError MThread::Join()
{
    int err = pthread_join(tid_, nullptr);
    if (err != 0)
    {
        MLOG(MGetLibLogger(), MERR, "join thread failed err:", err);
        return MError::Unknown;
    }
    return MError::No;
}

MError MThread::StopAndJoin()
{
    Stop();
    return Join();
}

m_thread_t MThread::GetPID() const
{
    return tid_;
}

m_thread_t MThread::GetCurrentPID()
{
    return pthread_self();
}

void* MThread::ThreadMain(void *p_param)
{
    MThread *p_th = static_cast<MThread*>(p_param);
    if (!p_th)
    {
        MLOG(MGetLibLogger(), MERR, "param is null");
        return nullptr;
    }
    if (!p_th->_BeforeRun())
    {
        MLOG(MGetLibLogger(), MERR, "before run failed");
        return nullptr;
    }
    while (!p_th->stop_flag_)
    {
        p_th->_Run();
    }
    p_th->_AfterRun();
    //pthread_exit(nullptr);
    return nullptr;
}
