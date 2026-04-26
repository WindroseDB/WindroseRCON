#include "commands.h"
#include "../ban/ban_list.h"

extern BanList g_BanList;

namespace Commands {
    std::string ExecuteBan(const std::string& args) {
        std::stringstream response;
        
        std::string rest = args;
        while (!rest.empty() && (rest.front() == ' ' || rest.front() == '\t')) {
            rest.erase(rest.begin());
        }
        
        if (rest.empty()) {
            response << "Usage: ban <account_id> [reason]\n";
        } else {
            std::string accountId;
            std::string reason;
            size_t sp = rest.find(' ');
            if (sp == std::string::npos) {
                accountId = rest;
            } else {
                accountId = rest.substr(0, sp);
                reason = rest.substr(sp + 1);
                while (!reason.empty() && (reason.front() == ' ' || reason.front() == '\t')) {
                    reason.erase(reason.begin());
                }
            }

            std::string resolvedId = accountId;
            auto players = UnrealEngine::g_Engine->GetAllPlayers();
            for (const auto& p : players) {
                if (p.accountId.find(accountId) != std::string::npos) {
                    resolvedId = p.accountId;
                    break;
                }
            }

            bool added = g_BanList.Add(resolvedId, reason);
            bool kicked = UnrealEngine::g_Engine->KickPlayer(resolvedId);

            if (added) {
                response << "Banned " << resolvedId;
                if (!reason.empty()) response << " (" << reason << ")";
                response << "\n";
            } else {
                response << resolvedId << " was already banned\n";
            }
            if (kicked) {
                response << "Player was online and has been kicked\n";
            }
        }
        
        return response.str();
    }
}
