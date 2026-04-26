#include "commands.h"

namespace Commands {
    std::string ExecuteHelp() {
        std::stringstream response;
        response << "Available commands:\n";
        response << "  info - Show server information\n";
        response << "  showplayers - List all connected players\n";
        response << "  kick <account_id> - Kick a player\n";
        response << "  ban <account_id> [reason] - Ban a player (also kicks if online)\n";
        response << "  unban <account_id> - Remove a player from the ban list\n";
        response << "  banlist - Show all banned players\n";
        response << "  getpos <account_id> - Show a player's world position\n";
        response << "  help - Show this help message\n";
        return response.str();
    }
}
