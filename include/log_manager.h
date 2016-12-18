#ifndef _MEGUMI_LOG_MANAGER_H_
#define _MEGUMI_LOG_MANAGER_H_

#include "singleton.h"
#include "log.h"

#include <unordered_map>

class LogManager : public Singleton<LogManager> {
public:
    using LogList = std::unordered_map<std::string, Log*>;

    LogManager();
    ~LogManager();

    /* create a new log and add it to the log list */
    Log* add_log(const std::string& name, LogMessageLevel lvl = LogMessageLevel::INFO, bool debug_output = true);

    template <typename ... Args>
    void log_format(LogMessageLevel lvl, const char* format, Args ... args)
    {
        if (current_log) {
            current_log->log_format(lvl, format, args ...);
        }
    }

    /* for convenience */
    template <typename ... Args>
    void debug(const char* format, Args ... args)
    {
        log_format(LogMessageLevel::DEBUG, format, args ...);
    }
    template <typename ... Args>
    void info(const char* format, Args ... args)
    {
        log_format(LogMessageLevel::INFO, format, args ...);
    }
    template <typename ... Args>
    void warn(const char* format, Args ... args)
    {
        log_format(LogMessageLevel::WARN, format, args ...);
    }
    template <typename ... Args>
    void error(const char* format, Args ... args)
    {
#ifdef _WIN32_
        log_format(LogMessageLevel::ERROR_, format, args ...);
#else
		log_format(LogMessageLevel::ERROR, format, args ...);
#endif
    }

private:
    LogList logs;
    Log* current_log;
};

#define LOG LogManager::get_singleton()

#endif

