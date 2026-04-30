#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <wincrypt.h>

#pragma comment(lib, "advapi32.lib")

struct RCONConfig {
    int port = 27065;
    std::string password = "windrose_admin";
    bool enableLogging = true;
    std::string logFile = "windrosercon\\rcon.log";
    std::string bindAddress = "0.0.0.0";
    std::string allowedIPs = "";
    int maxFailedAttempts = 5;
    int timeout = 60;
    bool secureRconEnabled = false;
    std::string aesKey = "";
    
    void Load(const std::string& configPath) {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            return;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#' || line[0] == ';') continue;
            
            size_t pos = line.find('=');
            if (pos == std::string::npos) continue;
            
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            if (key == "Port") {
                port = std::stoi(value);
            } else if (key == "Password") {
                password = value;
            } else if (key == "EnableLogging") {
                enableLogging = (value == "true" || value == "1");
            } else if (key == "LogFile") {
                logFile = value;
            } else if (key == "BindAddress") {
                bindAddress = value;
            } else if (key == "AllowedIPs") {
                allowedIPs = value;
            } else if (key == "MaxFailedAttempts") {
                maxFailedAttempts = std::stoi(value);
            } else if (key == "Timeout") {
                timeout = std::stoi(value);
            } else if (key == "Enabled") {
                secureRconEnabled = (value == "true" || value == "1");
            } else if (key == "AESKey") {
                aesKey = value;
            }
        }
        file.close();
    }
    
    void Save(const std::string& configPath) {
        if (aesKey.empty()) {
            aesKey = GenerateAESKey();
        }
        
        std::ofstream file(configPath);
        if (!file.is_open()) return;
        
        file << "# Windrose RCON Configuration\n";
        file << "# Generated automatically - edit as needed\n\n";
        file << "[RCON]\n";
        file << "# Bind to 127.0.0.1 to restrict to localhost only (recommended)\n";
        file << "BindAddress=" << bindAddress << "\n";
        file << "Port=" << port << "\n";
        file << "Password=" << password << "\n";
        file << "# Comma separated IP whitelist. Empty = allow all\n";
        file << "AllowedIPs=" << allowedIPs << "\n";
        file << "# Max failed auth attempts before IP is blocked\n";
        file << "MaxFailedAttempts=" << maxFailedAttempts << "\n";
        file << "# Idle/auth timeout in seconds\n";
        file << "Timeout=" << timeout << "\n";
        file << "EnableLogging=" << (enableLogging ? "true" : "false") << "\n";
        file << "LogFile=" << logFile << "\n\n";
        
        file << "[SecureRCON]\n";
        file << "# Enable AES-256-GCM encryption. Needs custom client!\n";
        file << "Enabled=" << (secureRconEnabled ? "true" : "false") << "\n";
        file << "# Auto-generated 256-bit encryption key. Keep this secret!\n";
        file << "AESKey=" << aesKey << "\n";
        
        file.close();
    }
    
    static std::string GetConfigPath() {
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        std::string path(exePath);
        size_t pos = path.find_last_of("\\/");
        if (pos != std::string::npos) {
            path = path.substr(0, pos);
        }
        return path + "\\windrosercon\\settings.ini";
    }
    
    static void EnsureConfigDirectory() {
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        std::string path(exePath);
        size_t pos = path.find_last_of("\\/");
        if (pos != std::string::npos) {
            path = path.substr(0, pos);
        }
        std::string configDir = path + "\\windrosercon";
        CreateDirectoryA(configDir.c_str(), NULL);
    }
    
    static std::string GenerateAESKey() {
        HCRYPTPROV hProv = 0;
        if (!CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
            return "";
        }
        
        unsigned char randomBytes[32];
        if (!CryptGenRandom(hProv, 32, randomBytes)) {
            CryptReleaseContext(hProv, 0);
            return "";
        }
        
        CryptReleaseContext(hProv, 0);
        
        const char* hexChars = "0123456789abcdef";
        std::string hexKey;
        hexKey.reserve(64);
        for (int i = 0; i < 32; i++) {
            hexKey += hexChars[(randomBytes[i] >> 4) & 0xF];
            hexKey += hexChars[randomBytes[i] & 0xF];
        }
        
        return hexKey;
    }
};

extern RCONConfig g_Config;
