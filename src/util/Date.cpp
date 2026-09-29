#include "util/Date.hpp"

#include <string>
#include <ctime>

std::string today() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#if defined (_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[11]; // "YYYY-MM-DD + \0"
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
    return buf;
}
