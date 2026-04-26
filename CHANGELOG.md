# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Initial release of Windrose RCON
- Source RCON protocol implementation
- DLL proxy injection via `version.dll`
- Player management commands (kick, ban, unban)
- Server information query
- Player position tracking
- Configuration file support
- Logging system
- Ban list persistence
- Apache 2.0 license
- Contributing guidelines

### Commands
- `info` - Display server information
- `showplayers` - List all online players
- `help` - Show available commands
- `kick <AccountId>` - Kick player by account ID
- `ban <AccountId>` - Ban player by account ID
- `unban <AccountId>` - Unban player by account ID
- `banlist` - Show all banned players
- `getpos <AccountId>` - Get player world position

### Technical
- GObjects scanning for player states
- Pattern-based offset finding
- Unreal Engine integration
- Multi-threaded RCON server
- Safe memory access with validation

[Unreleased]: https://github.com/yourusername/WindroseRCON/commits/main
