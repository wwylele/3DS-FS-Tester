#include "util.h"
#include "command.h"

static std::vector<u8> rw_buffer;

static void PrintUI(const std::string& description) {
    consoleClear();
    printf("\x1b[0;0HFile: %s\n", description.data());
    printf("Last read/written data:");
    for (unsigned i = 0; i < rw_buffer.size(); ++i) {
        if (i % 16 == 0)
            printf("\n");
        printf("%02X ", rw_buffer[i]);
    }
}

static std::vector<u8> ParseData(const std::string& str) {
    std::vector<u8> data;
    for (unsigned i = 0; i < str.size() / 2; ++i) {
        auto s = str.substr(i * 2, 2);
        data.push_back(std::strtoul(s.data(), 0, 16));
    }
    return data;
}

static void RunCommand(Handle file, const std::string& command) {
    auto params = SplitString(command, " ");
    if (params.size() == 0) {
        Log("Empty command!\n");
        return;
    } else if (params[0] == "r") {
        u64 offset = 0;
        u32 size = 0;
        if (!ParseParams(params, [&](char slot, const std::string& content) {
            switch (slot) {
            case 's':
                size = std::strtoul(content.data(), 0, 0);
                return true;
            case 'o':
                offset = std::strtoul(content.data(), 0, 0);
                return true;
            default:
                return false;
            }
        })) return;
        rw_buffer = {};
        rw_buffer.resize(size, 0xCC);
        Log("Read(size = 0x%X, offset = 0x%08llX)\n", size, offset);
        u32 bytes = 0;
        ParseResult(FSFILE_Read(file, &bytes, offset, rw_buffer.data(), size));
        Log("bytes = 0x%X\n", bytes);
    } else if (params[0] == "w") {
        u64 offset = 0;
        u32 option = 0;
        if (!ParseParams(params, [&](char slot, const std::string& content) {
            switch (slot) {
            case 'o':
                offset = std::strtoul(content.data(), 0, 0);
                return true;
            case 'q':
                option = std::strtoul(content.data(), 0, 0);
                return true;
            case 'd':
                rw_buffer = ParseData(content);
                return true;
            default:
                return false;
            }
        })) return;
        Log("Write(size = 0x%X, offset = 0x%08llX, option = 0x%08X)\n", rw_buffer.size(), offset, option);
        u32 bytes = 0;
        ParseResult(FSFILE_Write(file, &bytes, offset, rw_buffer.data(), rw_buffer.size(), option));
        Log("bytes = 0x%X\n", bytes);
    } else if (params[0] == "gs") {
        u64 size = 0;
        Log("GetSize()\n");
        ParseResult(FSFILE_GetSize(file, &size));
        Log("size = 0x%llX\n", size);
    } else if (params[0] == "ss") {
        u64 size = 0;
        if (!ParseParams(params, [&](char slot, const std::string& content) {
            switch (slot) {
            case 's':
                size = std::strtoul(content.data(), 0, 0);
                return true;
            default:
                return false;
            }
        })) return;
        Log("SetSize(size = 0x%llX)\n", size);
        ParseResult(FSFILE_SetSize(file, size));
    } else {
        Log("Unknown command %s\n", params[0].data());
    }
}
void Scene_File(Handle file, const std::string& description) {
    rw_buffer = {};
    for (;;) {
        PrintUI(description);
        u32 key = ListenKey();
        switch (key) {
        case KEY_B:
            return;
        case KEY_START:
            RunCommand(file, GetCommand());
            break;
        }
    }
}
