#pragma once
#include <string>
#include <sstream>
#include "../windrose_engine.h"

namespace Commands {
    std::string ExecuteHelp();
    std::string ExecuteInfo();
    std::string ExecuteShowPlayers();
    std::string ExecuteKick(const std::string& args);
    std::string ExecuteBan(const std::string& args);
    std::string ExecuteUnban(const std::string& args);
    std::string ExecuteBanlist();
    std::string ExecuteGetPos(const std::string& args);
    std::string Shutdown(const std::string& args);
    std::string ExecuteUptime();
    std::string ExecutePlayerInfo(const std::string& args);
}
