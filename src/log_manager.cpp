#include "log_manager.h"

template <> LogManager* Singleton<LogManager>::singleton = nullptr;

LogManager::LogManager()
{
    current_log = nullptr;
}

LogManager::~LogManager()
{
    for (auto iter = logs.begin(); iter != logs.end(); ++iter) {
        delete iter->second;
    }
}

Log* LogManager::add_log(const std::string& name, LogMessageLevel lvl, bool debug_output)
{
    Log* l = new Log(name, lvl, debug_output);

    if (!current_log) {
        current_log = l;
    }

    logs[name] = l;
    return l;
}


