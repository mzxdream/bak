#include <util/m_time.h>
#include <sys/time.h>

int64_t MTime::GetTime()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return static_cast<int64_t>(tv.tv_sec) * 1000
        + static_cast<int64_t>(tv.tv_usec) / 1000;
}
