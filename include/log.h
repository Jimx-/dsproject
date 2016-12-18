#ifndef LOG_H_
#define LOG_H

#include <fstream>
#include <memory>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <string>

/* wrapper class for logging to files */

/* message is logged if message level >= logging level */
enum class LogMessageLevel {
    OFF = 0,
    DEBUG,
    INFO,
    WARN,
#ifdef _WIN32_
    ERROR_,
#else
	ERROR,
#endif
    ALL,
};

class Log {
public:
    Log(const std::string& filename, LogMessageLevel level, bool debug_output);

    void set_timestamp(bool timestamp_);

    template <typename ... Args>
    void log_format(LogMessageLevel lvl, const char* format, Args ... args)
    {
        std::string type = "";

#define DEF_LOG_TYPE(t) \
        case LogMessageLevel:: t: \
            type = #t; \
            break;

        switch (lvl) {
            DEF_LOG_TYPE(DEBUG)
            DEF_LOG_TYPE(INFO)
            DEF_LOG_TYPE(WARN)
#ifdef _WIN32_
            DEF_LOG_TYPE(ERROR_)
#else
			DEF_LOG_TYPE(ERROR)
#endif
        }

        if (lvl >= level) {

            if (timestamp) {
                time_t now = time(nullptr);
                struct tm* t = localtime(&now);
                stream << "[" << std::setw(2) << std::setfill('0') << t->tm_hour << ":"
                    << std::setw(2) << std::setfill('0') << t->tm_min << ":"
                    << std::setw(2) << std::setfill('0') << t->tm_sec << "]";
            }

            stream << "[" << type << "] ";
            size_t size = snprintf(nullptr, 0, format, args ...) + 1;
            std::unique_ptr<char[]> buf(new char[size]);
            snprintf(buf.get(), size, format, args ...);
            stream << buf.get() << std::endl;

            stream.flush();

            if (debug_output) {
                (lvl >= LogMessageLevel::WARN ? std::cerr : std::cout) <<
                    "[" << type << "] " << buf.get() << std::endl;
            }
        }
    }

private:
    std::ofstream stream;
    /* current logging level */
    LogMessageLevel level;
    /* timestamp enabled ? */
    bool timestamp;
    /* print message to standard output ? */
    bool debug_output;
};

#endif
