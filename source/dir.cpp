#include "util.h"
#include "dir.h"

const int MAX_DISPLAY_ENTRY = 20;

static void PrintUI(u32 cursor, u32 display_offset, const std::u16string& path, const std::vector<FS_DirectoryEntry>& dir_entries) {
    consoleClear();
    std::string path_c = ":" + ConvertString(path);
    if (path_c.size() > 50)
        path_c = path_c.substr(path_c.size() - 50);
    printf("\x1b[0;0H");
    printf("    A - open directory\n");
    printf("    START - input command\n");
    printf("    B - back\n");
    printf("%s\n", path_c.data());
    u32 iend = dir_entries.size() - display_offset;
    const int display_pivot = 5;
    if (MAX_DISPLAY_ENTRY < iend) {
        iend = MAX_DISPLAY_ENTRY;
        printf("\x1b[%d;4H...", MAX_DISPLAY_ENTRY + display_pivot);
    }
    if (display_offset != 0) {
        printf("\x1b[%d;4H...", -1 + display_pivot);
    }
    for (u32 i = 0; i < iend; ++i) {
        u32 index = display_offset + i;
        std::string name_c = ConvertString(std::u16string((const char16_t*)dir_entries[index].name));
        bool is_dir = (dir_entries[index].attributes & FS_ATTRIBUTE_DIRECTORY) != 0 ;
        bool is_hidden = (dir_entries[index].attributes & FS_ATTRIBUTE_HIDDEN) != 0 ;
        bool is_archive = (dir_entries[index].attributes & FS_ATTRIBUTE_ARCHIVE) != 0 ;
        bool is_ro = (dir_entries[index].attributes & FS_ATTRIBUTE_READ_ONLY) != 0 ;
        printf("\x1b[%lu;4H\x1b[%dm%c%c%c%c%s\n\x1b[0m",
            i + display_pivot,
            index == cursor ? 33 : 37,
            is_dir ? '+' : ' ',
            is_hidden ? '~' : ' ',
            is_archive ? '#' : ' ',
            is_ro ? '$' : ' ',
            name_c.data());
    }
}

static std::string GetCommand() {
    static SwkbdState swkbd;
	static char mybuf[60];

	swkbdInit(&swkbd, SWKBD_TYPE_WESTERN, 2, -1);
	swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
	swkbdSetHintText(&swkbd, "Please input a command.");

	if (SWKBD_BUTTON_RIGHT != swkbdInputText(&swkbd, mybuf, sizeof(mybuf)))
        return "";

    return mybuf;
}

std::vector<std::string> SplitString(std::string str, std::string pattern) {
    std::string::size_type pos;
    std::vector<std::string> result;
    str += pattern;
    std::string::size_type size = str.size();

    for(std::string::size_type i = 0; i < size; i++) {
        pos = str.find(pattern, i);
        if(pos < size) {
            std::string s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}

template <typename T>
bool ParseParams(const std::vector<std::string>& params, T parser) {
    for (unsigned i = 1; i < params.size(); ++i) {
        if (params[i].size() < 1) continue;
        if (!parser(params[i][0], params[i].substr(1))) {
            Log("ParseParams: unknown param \"%s\"\n", params[i].data());
            return false;
        }
    }
    return true;
}

static void RunCommand(FS_Archive archive, const std::string& command) {
    auto params = SplitString(command, " ");
    if (params.size() == 0) {
        Log("Empty command!\n");
        return;
    } else if (params[0] == "c") {
        std::string path = "";
        u64 size = 0;
        u32 attr = 0;
        if (!ParseParams(params, [&](char slot, const std::string& content) {
            switch (slot) {
            case 'p':
                path = content;
                return true;
            case 's':
                size = std::strtoul(content.data(), 0, 0);
                return true;
            case 'a':
                attr = std::strtoul(content.data(), 0, 0);
                return true;
            default:
                return false;
            }
        })) return;
        Log("CreateFile(\"%s\", size=%llu, attr=0x%08X)\n", path.data(), size, attr);
        ParseResult(FSUSER_CreateFile(archive, MakePath(ConvertString(path)), attr, size));
    } else if (params[0] == "cd") {
        std::string path = "";
        u32 attr = 0;
        if (!ParseParams(params, [&](char slot, const std::string& content) {
            switch (slot) {
            case 'p':
                path = content;
                return true;
            case 'a':
                attr = std::strtoul(content.data(), 0, 0);
                return true;
            default:
                return false;
            }
        })) return;
        Log("CreateDirectory(\"%s\", attr=0x%08X)\n", path.data(), attr);
        ParseResult(FSUSER_CreateDirectory(archive, MakePath(ConvertString(path)), attr));
    } else if (params[0] == "d") {
        std::string path = "";
        if (!ParseParams(params, [&](char slot, const std::string& content) {
            switch (slot) {
            case 'p':
                path = content;
                return true;
            default:
                return false;
            }
        })) return;
        Log("DeleteFile(\"%s\")\n", path.data());
        ParseResult(FSUSER_DeleteFile(archive, MakePath(ConvertString(path))));
    } else if (params[0] == "dd") {
        std::string path = "";
        if (!ParseParams(params, [&](char slot, const std::string& content) {
            switch (slot) {
            case 'p':
                path = content;
                return true;
            default:
                return false;
            }
        })) return;
        Log("DeleteDirectory(\"%s\")\n", path.data());
        ParseResult(FSUSER_DeleteDirectory(archive, MakePath(ConvertString(path))));
    } else if (params[0] == "dr") {
        std::string path = "";
        if (!ParseParams(params, [&](char slot, const std::string& content) {
            switch (slot) {
            case 'p':
                path = content;
                return true;
            default:
                return false;
            }
        })) return;
        Log("DeleteDirectoryRecursively(\"%s\")\n", path.data());
        ParseResult(FSUSER_DeleteDirectoryRecursively(archive, MakePath(ConvertString(path))));
    } else if (params[0] == "o") {
        std::string path = "";
        u32 flags = 0;
        u32 attr = 0;
        if (!ParseParams(params, [&](char slot, const std::string& content) {
            switch (slot) {
            case 'p':
                path = content;
                return true;
            case 'a':
                attr = std::strtoul(content.data(), 0, 0);
                return true;
            case 'f':
                for (char f : content) {
                    if (f == 'r') {
                        flags |= FS_OPEN_READ;
                    } else if (f == 'w') {
                        flags |= FS_OPEN_WRITE;
                    } else if (f == 'c') {
                        flags |= FS_OPEN_CREATE;
                    }
                }
                return true;
            default:
                return false;
            }
        })) return;
        Log("OpenFile(\"%s\", flag=%d, attr=0x%08X)\n", path.data(), flags, attr);
        Handle file;
        if (ParseResult(FSUSER_OpenFile(&file, archive, MakePath(ConvertString(path)), flags, attr))) {
            FSFILE_Close(file);
        }
    } else if (params[0] == "od") {
        std::string path = "";
        if (!ParseParams(params, [&](char slot, const std::string& content) {
            switch (slot) {
            case 'p':
                path = content;
                return true;
            default:
                return false;
            }
        })) return;
        Log("OpenDirectory(\"%s\")\n", path.data());
        Handle dir;
        if (ParseResult(FSUSER_OpenDirectory(&dir, archive, MakePath(ConvertString(path))))) {
            FSDIR_Close(dir);
        }
    }
}

void Scene_Dir(FS_Archive archive, const std::u16string& path) {
    std::vector<FS_DirectoryEntry> dir_entries;
    u32 cursor = 0;
    u32 display_offset = 0;
    std::u16string path_fix = path.size() == 0 ? u"/" : path;
    dir_entries = GetEntries(archive, path_fix);

    for (;;) {
        PrintUI(cursor, display_offset, path_fix, dir_entries);
        u32 key = ListenKey();
        switch (key) {
        case KEY_DUP:
            if (cursor)
                --cursor;
            if (cursor < display_offset)
                display_offset = cursor;
            break;
        case KEY_DDOWN:
            if (cursor < dir_entries.size() - 1)
                ++cursor;
            if (cursor >= display_offset + MAX_DISPLAY_ENTRY)
                display_offset = cursor - MAX_DISPLAY_ENTRY + 1;
            break;
        case KEY_A:
            if (cursor < dir_entries.size()) {
                if (dir_entries[cursor].attributes & FS_ATTRIBUTE_DIRECTORY) {
                    Scene_Dir(archive, path + u"/" + (const char16_t*)dir_entries[cursor].name);
                } else {

                }
            }
            break;
        case KEY_B:
            return;
        case KEY_START:
            RunCommand(archive, GetCommand());
            g_log = false;
            dir_entries = GetEntries(archive, path_fix);
            g_log = true;
            cursor = 0;
            display_offset = 0;
            break;
        }
    }
}
