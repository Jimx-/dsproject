#ifndef _INTERN_STRING_H_
#define _INTERN_STRING_H_

#include <string>
#include <cstring>

namespace detail {

class InternStringDetail {
public:
    std::string data;

    InternStringDetail(const char* str, size_t len) : data(str, str+len) { }
    bool operator==(const InternStringDetail& rhs) const { return data == rhs.data; }

    InternStringDetail& operator=(const InternStringDetail& rhs) = delete;
};

}

class InternString {
public:
    InternString(const char* str, size_t len = -1) : detail(intern(str, (len == -1) ? strlen(str) : len)) { }
    InternString(const std::string& str) : detail(intern(str.c_str(), str.length())) { }

    bool operator==(const InternString& rhs) const { return detail == rhs.detail; }
    bool operator!=(const InternString& rhs) const { return detail != rhs.detail; }
    bool operator<(const InternString& rhs) const { return detail < rhs.detail; }

    size_t length() const { return detail->data.length(); }
    const char* c_str() const { return detail->data.c_str(); }

private:
    static detail::InternStringDetail* intern(const char* str, size_t len);

    detail::InternStringDetail* detail;
};

#endif
