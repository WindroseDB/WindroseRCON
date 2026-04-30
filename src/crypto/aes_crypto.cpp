#include "aes_crypto.h"
#include <random>
#include <cstring>

BCRYPT_ALG_HANDLE AESCrypto::hAlgorithm = nullptr;
BCRYPT_KEY_HANDLE AESCrypto::hKey = nullptr;
std::vector<uint8_t> AESCrypto::keyBytes;
bool AESCrypto::initialized = false;

std::vector<uint8_t> AESCrypto::HexToBytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = (uint8_t)strtol(byteString.c_str(), nullptr, 16);
        bytes.push_back(byte);
    }
    return bytes;
}

bool AESCrypto::Initialize(const std::string& hexKey) {
    if (initialized) {
        Cleanup();
    }
    
    if (hexKey.length() != 64) {
        return false;
    }
    
    keyBytes = HexToBytes(hexKey);
    if (keyBytes.size() != 32) {
        return false;
    }
    
    NTSTATUS status = BCryptOpenAlgorithmProvider(
        &hAlgorithm,
        BCRYPT_AES_ALGORITHM,
        nullptr,
        0
    );
    
    if (!BCRYPT_SUCCESS(status)) {
        return false;
    }
    
    status = BCryptSetProperty(
        hAlgorithm,
        BCRYPT_CHAINING_MODE,
        (PUCHAR)BCRYPT_CHAIN_MODE_GCM,
        sizeof(BCRYPT_CHAIN_MODE_GCM),
        0
    );
    
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlgorithm, 0);
        hAlgorithm = nullptr;
        return false;
    }
    
    status = BCryptGenerateSymmetricKey(
        hAlgorithm,
        &hKey,
        nullptr,
        0,
        keyBytes.data(),
        (ULONG)keyBytes.size(),
        0
    );
    
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlgorithm, 0);
        hAlgorithm = nullptr;
        return false;
    }
    
    initialized = true;
    return true;
}

void AESCrypto::Cleanup() {
    if (hKey) {
        BCryptDestroyKey(hKey);
        hKey = nullptr;
    }
    if (hAlgorithm) {
        BCryptCloseAlgorithmProvider(hAlgorithm, 0);
        hAlgorithm = nullptr;
    }
    keyBytes.clear();
    initialized = false;
}

std::vector<uint8_t> AESCrypto::Encrypt(const uint8_t* data, size_t dataLen) {
    if (!initialized || !hKey) {
        return {};
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    uint8_t nonce[12];
    for (int i = 0; i < 12; i++) {
        nonce[i] = dis(gen);
    }
    
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
    authInfo.pbNonce = nonce;
    authInfo.cbNonce = 12;
    authInfo.pbTag = nullptr;
    authInfo.cbTag = 16;
    
    ULONG ciphertextLen = 0;
    NTSTATUS status = BCryptEncrypt(
        hKey,
        (PUCHAR)data,
        (ULONG)dataLen,
        &authInfo,
        nullptr,
        0,
        nullptr,
        0,
        &ciphertextLen,
        0
    );
    
    if (!BCRYPT_SUCCESS(status)) {
        return {};
    }
    
    std::vector<uint8_t> result(12 + ciphertextLen + 16);
    memcpy(result.data(), nonce, 12);
    
    uint8_t tag[16];
    authInfo.pbTag = tag;
    
    ULONG bytesWritten = 0;
    status = BCryptEncrypt(
        hKey,
        (PUCHAR)data,
        (ULONG)dataLen,
        &authInfo,
        nullptr,
        0,
        result.data() + 12,
        ciphertextLen,
        &bytesWritten,
        0
    );
    
    if (!BCRYPT_SUCCESS(status)) {
        return {};
    }
    
    memcpy(result.data() + 12 + bytesWritten, tag, 16);
    
    return result;
}

std::vector<uint8_t> AESCrypto::Decrypt(const uint8_t* data, size_t dataLen) {
    if (!initialized || !hKey) {
        return {};
    }
    
    if (dataLen < 28) {
        return {};
    }
    
    uint8_t nonce[12];
    memcpy(nonce, data, 12);
    
    uint8_t tag[16];
    memcpy(tag, data + dataLen - 16, 16);
    
    size_t ciphertextLen = dataLen - 28;
    
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
    authInfo.pbNonce = nonce;
    authInfo.cbNonce = 12;
    authInfo.pbTag = tag;
    authInfo.cbTag = 16;
    
    ULONG plaintextLen = 0;
    NTSTATUS status = BCryptDecrypt(
        hKey,
        (PUCHAR)(data + 12),
        (ULONG)ciphertextLen,
        &authInfo,
        nullptr,
        0,
        nullptr,
        0,
        &plaintextLen,
        0
    );
    
    if (!BCRYPT_SUCCESS(status)) {
        return {};
    }
    
    std::vector<uint8_t> result(plaintextLen);
    
    ULONG bytesWritten = 0;
    status = BCryptDecrypt(
        hKey,
        (PUCHAR)(data + 12),
        (ULONG)ciphertextLen,
        &authInfo,
        nullptr,
        0,
        result.data(),
        plaintextLen,
        &bytesWritten,
        0
    );
    
    if (!BCRYPT_SUCCESS(status)) {
        return {};
    }
    
    result.resize(bytesWritten);
    return result;
}
