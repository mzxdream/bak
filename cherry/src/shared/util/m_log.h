#ifndef _M_LOG_H_
#define _M_LOG_H_

#include <string>
#include <sstream>
#include <iostream>

enum class MLogLevelType
{
    Info = 0,
    Warning = 1,
    Error = 2,
    Fatal = 3,
};

typedef void MLogHandler(MLogLevelType level, const char *p_file_name, int line, const std::string &message);

void MDefaultLogHandler(MLogLevelType level, const char *p_file_name, int line, const std::string &message);

class MLogFinisher;

class MLogMessage final
{
public:
    explicit MLogMessage(MLogLevelType level, const char *p_file_name, int line)
        :level_(level)
        ,p_file_name_(p_file_name)
        ,line_(line)
    {
    }
public:
    template<typename T>
    MLogMessage& operator<<(const T& value)
    {
        if (static_cast<unsigned>(level_) >= static_cast<unsigned>(s_listen_level_))
        {
            stream_ << value;
        }
        return *this;
    }
    MLogHandler* SetLogHandler(MLogHandler *p_new_handler)
    {
        MLogHandler *p_old_handler = sp_log_handler_;
        sp_log_handler_ = p_new_handler;
        return p_old_handler;
    }
    MLogLevelType SetLogListenLevel(MLogLevelType new_level)
    {
        MLogLevelType old_level = s_listen_level_;
        s_listen_level_ = new_level;
        return old_level;
    }
private:
    friend class MLogFinisher;
    void Finish() const
    {
        if (static_cast<unsigned>(level_) >= static_cast<unsigned>(s_listen_level_)
            && sp_log_handler_)
        {
            sp_log_handler_(level_, p_file_name_, line_, stream_.str());
        }
    }
private:
    MLogLevelType level_;
    const char *p_file_name_;
    int line_;
    std::stringstream stream_;
    static MLogHandler *sp_log_handler_;
    static MLogLevelType s_listen_level_;
};

class MLogFinisher final
{
public:
    void operator=(const MLogMessage &msg)
    {
        msg.Finish();
    }
};

#define MLOG(LEVEL) MLogFinisher() = MLogMessage(MLogLevelType::LEVEL, __FILE__, __LINE__)
#define MLOG_IF(LEVEL, CONDITION) !(CONDITION) ? (void)0 : MLOG(LEVEL)

#endif
