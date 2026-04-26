#include "commands.h"

namespace Commands {
    std::string ExecuteShowPlayers() {
        std::stringstream response;
        
        auto players = UnrealEngine::g_Engine->GetAllPlayers();
        response << "Players online: " << players.size() << "\n";
        for (const auto& player : players) {
            std::string name(player.playerName.begin(), player.playerName.end());
            response << "  " << name << " - " << player.accountId << "\n";
        }
        
        return response.str();
    }
}
