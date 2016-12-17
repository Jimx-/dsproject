#include "string_utils.h"

#include <sstream>

int StringUtils::parse_int(const std::string& val, int defval)
{
    std::stringstream ss(val);
    int v = defval;
    ss >> v;

    return v;
}

