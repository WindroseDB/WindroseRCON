#include "rcon_server.h"
#include "../config/config.h"
#include <fstream>
#include <ctime>

extern RCONConfig g_Config;
extern void LogMessage(const std::string& message);
extern std::string ExecuteRCONCommand(const std::string& command);

RCONServer::RCONServer() : listenSocket(INVALID_SOCKET), running(false), listenerThread(nullptr), clientThread(nullptr) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

RCONServer::~RCONServer() {
    Stop();
    WSACleanup();
}

bool RCONServer::Start(int port) {
    m_Port = port;
    if (running) return false;

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        LogMessage("RCON: Failed to create socket");
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(m_Port);
    
    if (g_Config.bindAddress.empty() || g_Config.bindAddress == "0.0.0.0") {
        serverAddr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, g_Config.bindAddress.c_str(), &serverAddr.sin_addr) != 1) {
            LogMessage("RCON: Invalid BindAddress '" + g_Config.bindAddress + "', falling back to 0.0.0.0");
            serverAddr.sin_addr.s_addr = INADDR_ANY;
        }
    }

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        LogMessage("RCON: Failed to bind socket on " + g_Config.bindAddress + ":" + std::to_string(m_Port));
        closesocket(listenSocket);
        return false;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        LogMessage("RCON: Failed to listen on socket");
        closesocket(listenSocket);
        return false;
    }

    running = true;
    listenerThread = new std::thread(&RCONServer::ListenerLoop, this);
    
    LogMessage("RCON: Server started on port " + std::to_string(m_Port));
    return true;
}

void RCONServer::Stop() {
    if (!running) return;
    
    running = false;
    
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }

    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (SOCKET client : clients) {
            closesocket(client);
        }
        clients.clear();
    }

    if (listenerThread && listenerThread->joinable()) {
        listenerThread->join();
        delete listenerThread;
        listenerThread = nullptr;
    }

    LogMessage("RCON: Server stopped");
}

void RCONServer::ListenerLoop() {
    while (running) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(listenSocket, &readSet);

        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int result = select(0, &readSet, nullptr, nullptr, &timeout);
        if (result > 0 && FD_ISSET(listenSocket, &readSet)) {
            sockaddr_in clientAddr;
            int addrLen = sizeof(clientAddr);
            SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);
            if (clientSocket != INVALID_SOCKET) {
                char ipBuf[INET_ADDRSTRLEN] = {0};
                inet_ntop(AF_INET, &clientAddr.sin_addr, ipBuf, sizeof(ipBuf));
                std::string clientIP(ipBuf);
                
                if (!IsIPAllowed(clientIP)) {
                    LogMessage("RCON: Rejected connection from " + clientIP + " (not in AllowedIPs)");
                    closesocket(clientSocket);
                    continue;
                }
                
                if (IsIPBlocked(clientIP)) {
                    LogMessage("RCON: Rejected connection from " + clientIP + " (blocked due to too many failed attempts)");
                    closesocket(clientSocket);
                    continue;
                }
                
                if (g_Config.timeout > 0) {
                    DWORD timeoutMs = g_Config.timeout * 1000;
                    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
                    setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
                }
                
                {
                    std::lock_guard<std::mutex> lock(clientsMutex);
                    clients.push_back(clientSocket);
                }
                
                std::thread([this, clientSocket, clientIP]() {
                    HandleClient(clientSocket, clientIP);
                }).detach();
                
                LogMessage("RCON: Client connected from " + clientIP);
            }
        }
    }
}

bool RCONServer::IsIPAllowed(const std::string& ip) {
    if (g_Config.allowedIPs.empty()) return true;
    
    std::string list = g_Config.allowedIPs;
    size_t start = 0;
    while (start < list.size()) {
        size_t end = list.find(',', start);
        if (end == std::string::npos) end = list.size();
        std::string entry = list.substr(start, end - start);
        // trim
        while (!entry.empty() && (entry.front() == ' ' || entry.front() == '\t')) entry.erase(entry.begin());
        while (!entry.empty() && (entry.back() == ' ' || entry.back() == '\t')) entry.pop_back();
        if (!entry.empty() && entry == ip) return true;
        start = end + 1;
    }
    return false;
}

bool RCONServer::IsIPBlocked(const std::string& ip) {
    if (g_Config.maxFailedAttempts <= 0) return false;
    std::lock_guard<std::mutex> lock(failedAttemptsMutex);
    auto it = failedAttempts.find(ip);
    return it != failedAttempts.end() && it->second >= g_Config.maxFailedAttempts;
}

void RCONServer::RecordFailedAttempt(const std::string& ip) {
    std::lock_guard<std::mutex> lock(failedAttemptsMutex);
    failedAttempts[ip]++;
    LogMessage("RCON: Failed auth attempt from " + ip + " (" + std::to_string(failedAttempts[ip]) + "/" + std::to_string(g_Config.maxFailedAttempts) + ")");
}

void RCONServer::ResetFailedAttempts(const std::string& ip) {
    std::lock_guard<std::mutex> lock(failedAttemptsMutex);
    failedAttempts.erase(ip);
}

void RCONServer::HandleClient(SOCKET clientSocket, std::string clientIP) {
    bool authenticated = false;
    
    while (running) {
        RCONPacket packet;
        int received = recv(clientSocket, (char*)&packet, sizeof(int32_t), 0);
        if (received <= 0) break;

        if (packet.size > sizeof(packet.body)) break;

        received = recv(clientSocket, (char*)&packet.id, packet.size, 0);
        if (received <= 0) break;

        if (packet.type == (int32_t)RCONPacketType::AUTH) {
            std::string password(packet.body);
            if (password == g_Config.password) {
                authenticated = true;
                ResetFailedAttempts(clientIP);
                SendPacket(clientSocket, packet.id, RCONPacketType::AUTH_RESPONSE, "");
                LogMessage("RCON: Client " + clientIP + " authenticated");
            } else {
                RecordFailedAttempt(clientIP);
                SendPacket(clientSocket, -1, RCONPacketType::AUTH_RESPONSE, "");
                LogMessage("RCON: Authentication failed from " + clientIP);
                break;
            }
        } else if (packet.type == (int32_t)RCONPacketType::EXECCOMMAND) {
            if (!authenticated) {
                break;
            }
            
            std::string command(packet.body);
            LogMessage("RCON: " + clientIP + " executing command: " + command);
            
            std::string response = ExecuteRCONCommand(command);
            SendPacket(clientSocket, packet.id, RCONPacketType::RESPONSE_VALUE, response);
        }
    }

    closesocket(clientSocket);
    
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
    }
    
    LogMessage("RCON: Client " + clientIP + " disconnected");
}

void RCONServer::SendPacket(SOCKET sock, int32_t id, RCONPacketType type, const std::string& body) {
    RCONPacket packet;
    packet.id = id;
    packet.type = (int32_t)type;
    
    size_t bodyLen = body.length() + 1;
    if (bodyLen > sizeof(packet.body) - 1) {
        bodyLen = sizeof(packet.body) - 1;
    }
    
    memcpy(packet.body, body.c_str(), bodyLen);
    packet.body[bodyLen] = '\0';
    
    packet.size = sizeof(packet.id) + sizeof(packet.type) + (int32_t)bodyLen + 1;
    
    send(sock, (char*)&packet, sizeof(packet.size) + packet.size, 0);
}

std::string RCONServer::ExecuteCommand(const std::string& command) {
    return ExecuteRCONCommand(command);
}
