#include "commands.h"
#include "../ban/ban_list.h"

extern BanList g_BanList;

namespace Commands {
    std::string ExecuteUnban(const std::string& args) {
        std::stringstream response;
        
        std::string accountId = args;
        while (!accountId.empty() && (accountId.front() == ' ' || accountId.front() == '\t')) {
            accountId.erase(accountId.begin());
        }
        
        if (accountId.empty()) {
            response << "Usage: unban <account_id>\n";
        } else if (g_BanList.Remove(accountId)) {
            response << "Unbanned " << accountId << "\n";
        } else {
            response << "No exact ban entry for " << accountId << "\n";
        }
        
        return response.str();
    }
}
