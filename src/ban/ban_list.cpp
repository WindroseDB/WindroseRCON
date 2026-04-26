#include "ban_list.h"

#include <windows.h>
#include <fstream>
#include <sstream>
#include <algorithm>

extern void LogMessage(const std::string& message);

BanList g_BanList;

BanList::BanList() {
}

std::string BanList::EscapeJson(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if ((unsigned char)c < 0x20) {
                    char buf[8];
                    sprintf_s(buf, "\\u%04x", (unsigned char)c);
                    out += buf;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

std::string BanList::GetBanListPath() {
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string path(exePath);
    size_t pos = path.find_last_of("\\/");
    if (pos != std::string::npos) {
        path = path.substr(0, pos);
    }
    return path + "\\windrosercon\\banlist.json";
}

void BanList::Initialize() {
    std::lock_guard<std::mutex> lock(mtx);
    filePath = GetBanListPath();
    entries.clear();

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        LogMessage("BanList: no existing banlist.json, starting fresh");
        return;
    }

    std::stringstream ss;
    ss << file.rdbuf();
    std::string data = ss.str();
    file.close();

    size_t i = 0;
    while (i < data.size()) {
        size_t accKey = data.find("\"accountId\"", i);
        if (accKey == std::string::npos) break;
        size_t q1 = data.find('"', accKey + 11);
        if (q1 == std::string::npos) break;
        size_t q2 = data.find('"', q1 + 1);
        if (q2 == std::string::npos) break;
        BanEntry e;
        e.accountId = data.substr(q1 + 1, q2 - q1 - 1);
        e.timestamp = 0;

        size_t reasonKey = data.find("\"reason\"", q2);
        size_t nextAcc = data.find("\"accountId\"", q2);
        if (reasonKey != std::string::npos && (nextAcc == std::string::npos || reasonKey < nextAcc)) {
            size_t r1 = data.find('"', reasonKey + 8);
            size_t r2 = data.find('"', r1 + 1);
            if (r1 != std::string::npos && r2 != std::string::npos) {
                e.reason = data.substr(r1 + 1, r2 - r1 - 1);
            }
        }

        size_t tsKey = data.find("\"timestamp\"", q2);
        if (tsKey != std::string::npos && (nextAcc == std::string::npos || tsKey < nextAcc)) {
            size_t colon = data.find(':', tsKey);
            if (colon != std::string::npos) {
                size_t end = data.find_first_of(",}\n", colon);
                if (end != std::string::npos) {
                    std::string num = data.substr(colon + 1, end - colon - 1);
                    try { e.timestamp = (std::time_t)std::stoll(num); } catch (...) {}
                }
            }
        }

        if (!e.accountId.empty()) entries.push_back(e);
        i = q2 + 1;
    }

    LogMessage("BanList: loaded " + std::to_string(entries.size()) + " ban(s)");
}

void BanList::SaveLocked() {
    HANDLE hFile = CreateFileA(
        filePath.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hFile == INVALID_HANDLE_VALUE) {
        LogMessage("BanList: ERROR unable to write " + filePath);
        return;
    }
    
    DWORD written = 0;
    std::string out = "[\r\n";
    for (size_t i = 0; i < entries.size(); i++) {
        const auto& e = entries[i];
        out += "  {\"accountId\": \"" + EscapeJson(e.accountId) + "\", ";
        out += "\"reason\": \"" + EscapeJson(e.reason) + "\", ";
        out += "\"timestamp\": " + std::to_string((long long)e.timestamp) + "}";
        if (i + 1 < entries.size()) out += ",";
        out += "\r\n";
    }
    out += "]\r\n";
    WriteFile(hFile, out.c_str(), (DWORD)out.length(), &written, NULL);
    
    CloseHandle(hFile);
}

bool BanList::IsBanned(const std::string& accountId) {
    if (accountId.empty()) return false;
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto& e : entries) {
        if (accountId.find(e.accountId) != std::string::npos ||
            e.accountId.find(accountId) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool BanList::Add(const std::string& accountId, const std::string& reason) {
    if (accountId.empty()) return false;
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto& e : entries) {
        if (e.accountId == accountId) {
            return false;
        }
    }
    BanEntry e;
    e.accountId = accountId;
    e.reason = reason;
    e.timestamp = std::time(nullptr);
    entries.push_back(e);
    SaveLocked();
    return true;
}

bool BanList::Remove(const std::string& accountId) {
    if (accountId.empty()) return false;
    std::lock_guard<std::mutex> lock(mtx);
    auto it = std::find_if(entries.begin(), entries.end(),
        [&](const BanEntry& e) { return e.accountId == accountId; });
    if (it == entries.end()) return false;
    entries.erase(it);
    SaveLocked();
    return true;
}

std::vector<BanEntry> BanList::GetAll() {
    std::lock_guard<std::mutex> lock(mtx);
    return entries;
}
