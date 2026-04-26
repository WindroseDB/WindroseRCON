#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <ctime>

struct BanEntry {
    std::string accountId;
    std::string reason;
    std::time_t timestamp;
};

class BanList {
private:
    std::vector<BanEntry> entries;
    std::mutex mtx;
    std::string filePath;

    void SaveLocked();

public:
    BanList();

    void Initialize();

    bool IsBanned(const std::string& accountId);
    bool Add(const std::string& accountId, const std::string& reason);
    bool Remove(const std::string& accountId);
    std::vector<BanEntry> GetAll();

    static std::string GetBanListPath();

private:
    static std::string EscapeJson(const std::string& s);
};

extern BanList g_BanList;
