#include "commands.h"
#include "../windrose_engine.h"
#include <sstream>
#include <iomanip>

namespace Commands {

std::string ExecutePlayerInfo(const std::string& args) {
    if (!UnrealEngine::g_Engine || !UnrealEngine::g_Engine->IsInitialized()) {
        return "Engine not initialized";
    }
    
    if (args.empty()) {
        return "Usage: playerinfo <account_id>";
    }
    
    auto players = UnrealEngine::g_Engine->GetAllPlayers();
    
    for (const auto& p : players) {
        if (p.accountId.find(args) != std::string::npos) {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2);
            
            std::string nameStr(p.playerName.begin(), p.playerName.end());
            ss << "Name: " << (nameStr.empty() ? "<unknown>" : nameStr) << "\n";
            ss << "Account ID: " << p.accountId << "\n";
            
            if (p.hasLocation) {
                ss << "Position: X=" << p.x << " Y=" << p.y << " Z=" << p.z << "\n";
                ss << "Rotation: Yaw=" << p.yaw << " Roll=" << p.roll << "\n";
            } else {
                ss << "Position: <unavailable>\n";
            }
            
            return ss.str();
        }
    }
    
    return "Player not found: " + args;
}

}
