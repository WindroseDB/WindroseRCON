# Windrose RCON

Standalone RCON server for Windrose game servers using source rcon protocol. Injects via `version.dll` proxy and provides server information, query and player management.

## Features

- **Server Information** - View server details, player count, invite code
- **Player List** - See all connected players with their account IDs
- **Configuration** - Settings stored in `windrosercon/settings.ini`
- **Logging** - All activity logged to `windrosercon/rcon.log`
- **Player Management** - Manage players by their account IDs.
- **Shutdown Command** - Shutdown server with optional delay.

### Protocols

- **Source RCON Protocol** - Standard source RCON protocol that transfers data over plain text.
- **Secure RCON Protocol** - Encrypted RCON protocol that uses AES encryption for data transfer.

## Installation

1. Extract the `version.dll` to your server's Win64 folder:
   ```
   YourServer/R5/Binaries/Win64/version.dll
   ```

2. Start your server, RCON will initialize automatically.

3. Configuration file will be created at:
   ```
   YourServer/R5/Binaries/Win64/windrosercon/settings.ini
   ```

## Configuration

Edit `windrosercon/settings.ini`:

```ini
[RCON]
BindAddress=0.0.0.0
Port=27065
Password=windrose_admin
AllowedIPs=
MaxFailedAttempts=5
Timeout=60
EnableLogging=true
LogFile=windrosercon\rcon.log

[SecureRCON]
Enabled=false
AESKey=
```

## Usage

1. Connect with any Source RCON client (e.g., `rcon_client.py`):

```bash
python rcon_client.py
```
2. Connect to the server using Secure RCON client. (e.g., `rcon_secure.py`)

```bash
python rcon_secure.py
```

### Available Commands

- `info` - Display server information
- `showplayers` - List all online players
- `help` - Show available commands
- `kick <AccountId>` - Kicks player by their account ID
- `ban <AccountId> [reason]` - Bans player by their account ID (optional reason)
- `unban <AccountId>` - Unbans player by their account ID
- `banlist` - Shows all banned players
- `shutdown [seconds]` - Shutdown server (default 10s, max 300s)
- `playerinfo <AccountID>` - Gets player information by their account ID
- `getpos <AccountId>` - Gets player world position by their account ID
- `uptime` - Gets server uptime

## Building

### Requirements
- xmake v3.0.2+
- Visual Studio 2022
- Windows SDK

### Build Project
1. Build the project with xmake:
   ```bash
   xmake
   ```

## Technical Details

### How It Works

1. **DLL Proxy Injection**: The mod loads as `version.dll`, intercepting the game's DLL loading process to inject the RCON server.
2. **Memory Access**: Uses SDK offsets to locate game structures like `GWorld`, `GameState`, and `PlayerArray` to read player information.
3. **RCON Server**: Starts a TCP server that implements the Source RCON protocol for authentication and command processing.
4. **Command Execution**: Executes commands by calling Unreal Engine functions through `ProcessEvent` to interact with the game.

## Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on how to contribute to this project.

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

## Credits
- [Dumper7](https://github.com/Encryqed/Dumper-7) - For SDK generation
- [GameHostBros](https://www.gamehostbros.com/) - Providing server for testing on Wine.