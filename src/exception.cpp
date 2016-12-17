#include "exception.h"
#include "log_manager.h"

#include <sstream>

Exception::Exception(ExceptionCode code, const std::string& source, const std::string& descr, const char* type, const char* file,
                     int line) : code(code), source(source), descr(descr), type(type), file(file), line(line)
{
    if (LogManager::get_singleton_ptr()) {
        LogManager::get_singleton().error(make_description().c_str());
    }
}

const std::string& Exception::make_description() const
{
    if (!full_descr.length()) {
        std::stringstream ss;
        ss << type << ": " << descr << " in " << source << " at " << file << ":" << line;

        full_descr = ss.str();
    }

    return full_descr;
}

