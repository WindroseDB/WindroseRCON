#include "commands.h"
#include <cstdlib>
#include <thread>
#include <chrono>

extern void LogMessage(const std::string& message);

namespace Commands {

void DelayedShutdown(int seconds) {
    char msg[256];
    sprintf_s(msg, "Server shutting down in %d seconds...", seconds);
    LogMessage(msg);
    
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    
    LogMessage("Server shutting down now!");
    exit(0);
}

std::string Shutdown(const std::string& args) {
    int delay = 10;
    if (!args.empty()) {
        try {
            delay = std::stoi(args);
            if (delay < 0) delay = 0;
            if (delay > 300) delay = 300;
        } catch (...) {
            return "Invalid delay. Usage: shutdown [seconds] (0-300)";
        }
    }

    if (delay == 0) {
        LogMessage("Server shutting down immediately!");
        exit(0);
    }
    
    std::thread(DelayedShutdown, delay).detach();
    
    char msg[256];
    sprintf_s(msg, "Server will shutdown in %d seconds", delay);
    return std::string(msg);
}

}
