#include "log.h"

Log::Log(const std::string& filename, LogMessageLevel level, bool debug_output)
    : level(level), timestamp(true), debug_output(debug_output)
{
    stream.open(filename.c_str());
}

void Log::set_timestamp(bool timestamp_)
{
    timestamp = timestamp_;
}

