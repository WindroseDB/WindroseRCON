# Contributing to Windrose RCON

Thank you for considering contributing to Windrose RCON! This document provides guidelines for contributing to the project.

## Code of Conduct

Be respectful and professional in all interactions. We're here to build something useful together.

## How to Contribute

### Reporting Bugs

If you find a bug, please open an issue with:
- A clear description of the problem
- Steps to reproduce the issue
- Expected vs actual behavior
- Your environment (OS, game version, etc.)
- Relevant logs from `windrosercon/rcon.log`

### Suggesting Features

Feature requests are welcome! Please open an issue describing:
- The feature you'd like to see
- Why it would be useful
- How it might work

### Pull Requests

1. **Fork the repository** and create a new branch for your feature/fix
2. **Follow the existing code style** - no unnecessary comments, clean code
3. **Test your changes** thoroughly on a real server
4. **Update documentation** if you're adding new features or commands
5. **Keep commits focused** - one logical change per commit

#### Development Setup

```bash
# Clone your fork
git clone https://github.com/YOUR_USERNAME/WindroseRCON.git
cd WindroseRCON

# Build the project
xmake

# The output will be in build/windows/x64/release/version.dll
```

#### Code Style Guidelines

- Use existing patterns from the codebase
- Avoid adding comments unless absolutely necessary
- Keep functions focused and single-purpose
- Use meaningful variable names
- Follow the existing file structure

#### Testing

Before submitting a PR:
- Build successfully with `xmake`
- Test on an actual Windrose server
- Verify RCON commands work as expected
- Check logs for errors or warnings
- Test edge cases (invalid input, disconnections, etc.)

### Adding New Commands

When adding a new RCON command:

1. Create a new file in `src/commands/` (e.g., `mycommand.cpp`)
2. Implement the command function in the `Commands` namespace
3. Add the declaration to `src/commands/commands.h`
4. Register the command in `src/rcon/rcon_server.cpp`
5. Add it to the help text in `src/commands/help.cpp`
6. Update `xmake.lua` to include the new file
7. Document changes in [CHANGELOG.md](CHANGELOG.md)

### Working with Unreal Engine Internals

This project directly interacts with Unreal Engine's internal structures:
- Be careful with memory access and pointer validation
- Use `IsBadReadPtr` before dereferencing pointers
- Understand the GObjects array structure
- Test thoroughly - crashes can bring down the server
- Offsets may change with game updates

## License

By contributing to Windrose RCON, you agree that your contributions will be licensed under the Apache License 2.0.

## Questions?

Open an issue for questions or discussion. We're happy to help!
