#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <windows.h>

struct RCONConfig {
    int port = 27065;
    std::string password = "windrose_admin";
    bool enableLogging = true;
    std::string logFile = "windrosercon\\rcon.log";
    
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
            }
        }
        file.close();
    }
    
    void Save(const std::string& configPath) {
        std::ofstream file(configPath);
        if (!file.is_open()) return;
        
        file << "# Windrose RCON Configuration\n";
        file << "# Generated automatically - edit as needed\n\n";
        file << "[RCON]\n";
        file << "Port=" << port << "\n";
        file << "Password=" << password << "\n";
        file << "EnableLogging=" << (enableLogging ? "true" : "false") << "\n";
        file << "LogFile=" << logFile << "\n";
        
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
};

extern RCONConfig g_Config;
