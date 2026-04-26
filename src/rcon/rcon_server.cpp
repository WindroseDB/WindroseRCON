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
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(m_Port);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        LogMessage("RCON: Failed to bind socket on port " + std::to_string(m_Port));
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
            SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
            if (clientSocket != INVALID_SOCKET) {
                {
                    std::lock_guard<std::mutex> lock(clientsMutex);
                    clients.push_back(clientSocket);
                }
                
                std::thread([this, clientSocket]() {
                    HandleClient(clientSocket);
                }).detach();
                
                LogMessage("RCON: Client connected");
            }
        }
    }
}

void RCONServer::HandleClient(SOCKET clientSocket) {
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
                SendPacket(clientSocket, packet.id, RCONPacketType::AUTH_RESPONSE, "");
                LogMessage("RCON: Client authenticated");
            } else {
                SendPacket(clientSocket, -1, RCONPacketType::AUTH_RESPONSE, "");
                LogMessage("RCON: Authentication failed");
                break;
            }
        } else if (packet.type == (int32_t)RCONPacketType::EXECCOMMAND) {
            if (!authenticated) {
                break;
            }
            
            std::string command(packet.body);
            LogMessage("RCON: Executing command: " + command);
            
            std::string response = ExecuteRCONCommand(command);
            SendPacket(clientSocket, packet.id, RCONPacketType::RESPONSE_VALUE, response);
        }
    }

    closesocket(clientSocket);
    
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
    }
    
    LogMessage("RCON: Client disconnected");
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
