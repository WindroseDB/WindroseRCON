#include "commands.h"
#include <chrono>
#include <ctime>

extern std::chrono::steady_clock::time_point g_ServerStartTime;
extern std::chrono::system_clock::time_point g_ServerStartSystemTime;

namespace Commands {

std::string ExecuteUptime() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - g_ServerStartTime).count();
    
    auto startTime = std::chrono::system_clock::to_time_t(g_ServerStartSystemTime);
    auto nowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    
    char buf[256];
    sprintf_s(buf, "Started: %lld\nUptime: %lld", (long long)startTime, (long long)nowTime);
    return std::string(buf);
}

}
