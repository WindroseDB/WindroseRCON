#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

enum class RCONPacketType : int32_t {
    AUTH = 3,
    AUTH_RESPONSE = 2,
    EXECCOMMAND = 2,
    RESPONSE_VALUE = 0
};

#pragma pack(push, 1)
struct RCONPacket {
    int32_t size;
    int32_t id;
    int32_t type;
    char body[4096];
};
#pragma pack(pop)

class RCONServer {
private:
    SOCKET listenSocket;
    std::vector<SOCKET> clients;
    std::mutex clientsMutex;
    bool running;
    int m_Port;
    std::thread* listenerThread;
    std::thread* clientThread;

    void ListenerLoop();
    void HandleClient(SOCKET clientSocket);
    void SendPacket(SOCKET sock, int32_t id, RCONPacketType type, const std::string& body);
    std::string ExecuteCommand(const std::string& command);

public:
    RCONServer();
    ~RCONServer();
    bool Start(int port = 27065);
    void Stop();
    bool IsRunning() const { return running; }
};
