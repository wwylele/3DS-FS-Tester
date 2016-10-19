#pragma once
#include <3ds.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

void InitConsole();
void Log(const char* f, ...);
bool ParseResult(Result r);
u32 ListenKey();
std::string ConvertString(const std::u16string& str);
std::u16string ConvertString(const std::string& str);
FS_Path MakePath(const std::u16string& path);

template <typename T>
FS_Path MakePath(const std::vector<T>& binary) {
    static std::vector<T> t;
    t = binary;
    if (t.size()) {
        return FS_Path{PATH_BINARY, t.size() * sizeof(T), t.data()};
    } else {
        return FS_Path{PATH_EMPTY, 0, nullptr};
    }
}

std::vector<FS_DirectoryEntry> GetEntries(FS_Archive archive, const std::u16string& src_path);

std::vector<std::string> SplitString(std::string str, std::string pattern);

extern bool g_log;
