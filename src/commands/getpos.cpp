#include "commands.h"

namespace Commands {
    std::string ExecuteGetPos(const std::string& args) {
        std::stringstream response;
        
        std::string accountId = args;
        while (!accountId.empty() && (accountId.front() == ' ' || accountId.front() == '\t')) {
            accountId.erase(accountId.begin());
        }
        
        if (accountId.empty()) {
            response << "Usage: getpos <account_id>\n";
        } else {
            auto players = UnrealEngine::g_Engine->GetAllPlayers();
            bool found = false;
            for (const auto& p : players) {
                if (p.accountId.find(accountId) != std::string::npos) {
                    std::string name(p.playerName.begin(), p.playerName.end());
                    if (!p.hasLocation) {
                        response << name << " (" << p.accountId << "): no location available (no pawn)\n";
                    } else {
                        char buf[256];
                        sprintf_s(buf, "%s (%s): X=%.2f Y=%.2f Z=%.2f | Pitch=%.2f Yaw=%.2f Roll=%.2f\n",
                            name.c_str(), p.accountId.c_str(),
                            p.x, p.y, p.z, p.pitch, p.yaw, p.roll);
                        response << buf;
                    }
                    found = true;
                    break;
                }
            }
            if (!found) response << "Player " << accountId << " not found\n";
        }
        
        return response.str();
    }
}
