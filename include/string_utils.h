#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_

#include <string>

class StringUtils {
public:
    static int parse_int(const std::string& val, int defval = 0);
};

#endif
