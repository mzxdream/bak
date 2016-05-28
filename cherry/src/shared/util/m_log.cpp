#include <util/m_log.h>

MLogHandler* MLogMessage::sp_log_handler_ = &MDefaultLogHandler;
MLogLevelType MLogMessage::s_listen_level_ = MLogLevelType::Warning;

void MDefaultLogHandler(MLogLevelType level, const char *p_file_name, int line, const std::string &message)
{
    static const char* level_names[] = {"INFO", "WARNING", "ERROR", "FATAL"};
    std::cerr << "[" << level_names[static_cast<unsigned>(level)] << " " << p_file_name << ":" << line << "] " << message << "\n";
}
