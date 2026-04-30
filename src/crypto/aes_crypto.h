#pragma once
#include <string>
#include <vector>
#include <windows.h>
#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")

class AESCrypto {
public:
    static bool Initialize(const std::string& hexKey);
    static void Cleanup();
    
    static std::vector<uint8_t> Encrypt(const uint8_t* data, size_t dataLen);
    static std::vector<uint8_t> Decrypt(const uint8_t* data, size_t dataLen);
    
private:
    static BCRYPT_ALG_HANDLE hAlgorithm;
    static BCRYPT_KEY_HANDLE hKey;
    static std::vector<uint8_t> keyBytes;
    static bool initialized;
    
    static std::vector<uint8_t> HexToBytes(const std::string& hex);
};
