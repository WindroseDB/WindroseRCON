#include "commands.h"

namespace Commands {
    std::string ExecuteKick(const std::string& args) {
        std::stringstream response;
        
        std::string accountId = args;
        while (!accountId.empty() && (accountId.front() == ' ' || accountId.front() == '\t')) {
            accountId.erase(accountId.begin());
        }
        
        if (accountId.empty()) {
            response << "Usage: kick <account_id>\n";
        } else if (UnrealEngine::g_Engine->KickPlayer(accountId)) {
            response << "Player " << accountId << " kicked\n";
        } else {
            response << "Failed to kick player " << accountId << "\n";
        }
        
        return response.str();
    }
}
