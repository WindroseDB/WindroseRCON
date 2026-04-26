#include "commands.h"

extern std::string ReadServerInfo(const std::string& key);

namespace Commands {
    std::string ExecuteInfo() {
        std::stringstream response;
        
        std::string serverName = ReadServerInfo("ServerName");
        std::string inviteCode = ReadServerInfo("InviteCode");
        std::string deploymentId = ReadServerInfo("DeploymentId");
        std::string maxPlayers = ReadServerInfo("MaxPlayerCount");
        std::string serverAddress = ReadServerInfo("DirectConnectionServerAddress");
        std::string serverPort = ReadServerInfo("DirectConnectionServerPort");
        
        auto players = UnrealEngine::g_Engine->GetAllPlayers();
        
        response << "Game: Windrose\n";
        if (!serverName.empty() && serverName != "N/A") {
            response << "Server Name: " << serverName << "\n";
        }
        response << "Invite Code: " << inviteCode << "\n";
        response << "Version: " << deploymentId << "\n";
        response << "Players: " << players.size() << "/" << maxPlayers << "\n";
        if (!serverAddress.empty() && serverAddress != "N/A") {
            response << "Server Address: " << serverAddress << ":" << serverPort << "\n";
        }
        
        return response.str();
    }
}
