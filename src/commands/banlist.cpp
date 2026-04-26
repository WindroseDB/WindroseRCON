#include "commands.h"
#include "../ban/ban_list.h"

extern BanList g_BanList;

namespace Commands {
    std::string ExecuteBanlist() {
        std::stringstream response;
        
        auto bans = g_BanList.GetAll();
        response << "Bans: " << bans.size() << "\n";
        for (const auto& b : bans) {
            response << "  " << b.accountId;
            if (!b.reason.empty()) response << " - " << b.reason;
            response << "\n";
        }
        
        return response.str();
    }
}
