#ifndef _M_LOGGER_H_
#define _M_LOGGER_H_

#include <iostream>
#include <mutex>
#include <util/m_singleton.h>

#define MLOG(logger, level, args...) logger.Print(level, __FILE__, __LINE__, args)
#define MLOG_IF(logger, level, condition, args...) !(condition) ? (void)0 : MLOG(logger, level, args)

#define MINFO  0
#define MWARN  1
#define MERR   2
#define MFATAL 3

class MDefaultLogger
{
public:
    MDefaultLogger(int level = MWARN)
        :level_filter_(level)
    {
    }
public:
    void SetLevelFilter(int level)
    {
        level_filter_ = level;
    }
    template <typename ...Args>
    void Print(int level, const char *p_file_name, int line, const Args& ...args)
    {
        if (level < level_filter_)
        {
            return;
        }
        std::lock_guard<std::mutex> lock(mtx_);
        std::cerr << "[" << level << " " << p_file_name << ":" << line << "] ";
        PrintArgs(args...);
    }
    template <typename T, typename ...Args>
    void PrintArgs(const T &arg, const Args& ...args)
    {
        std::cerr << arg;
        PrintArgs(args...);
    }
    void PrintArgs()
    {
        std::cerr << "\n";
    }
private:
    std::mutex mtx_;
    int level_filter_;
};

inline MDefaultLogger& MGetLibLogger()
{
    return MSingleton<MDefaultLogger, std::mutex>::Instance();
}

#endif
