#include "util.h"
#include <cstdarg>

static PrintConsole topScreen, bottomScreen;
bool g_log = true;

void InitConsole() {
    gfxInitDefault();

	consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM, &bottomScreen);

    consoleSelect(&topScreen);
}

void Log(const char* f, ...) {
    if (!g_log) return;
    consoleSelect(&bottomScreen);
    va_list vl;
    va_start(vl, f);
    std::vprintf(f, vl);
    va_end(vl);
    consoleSelect(&topScreen);
}

bool ParseResult(Result r) {
    if (R_SUCCEEDED(r)) {
        Log("\x1b[32mSuccess\x1b[0m\n");
        return true;
    } else {
        Log("\x1b[31mError: %08lX\x1b[0m\n", r);
        return false;
    }
}

u32 ListenKey() {
    while (aptMainLoop()) {
		hidScanInput();

        u32 kDown = hidKeysDown();

		if (kDown)
            return kDown;

		gfxFlushBuffers();
		gfxSwapBuffers();

		gspWaitForVBlank();
	}
    exit(0);
}

std::string ConvertString(const std::u16string& str) {
    std::string r(str.size(), '\0');
    for (unsigned i = 0; i < str.size(); ++i) {
        r[i] = str[i];
    }
    return r;
}
std::u16string ConvertString(const std::string& str) {
    std::u16string r(str.size(), u'\0');
    for (unsigned i = 0; i < str.size(); ++i) {
        r[i] = str[i];
    }
    return r;
}

FS_Path MakePath(const std::u16string& path) {
    static std::u16string t;
    t = path;
    return FS_Path{PATH_UTF16, t.size() * 2 + 2, t.data()};
}

std::vector<FS_DirectoryEntry> GetEntries(FS_Archive archive, const std::u16string& src_path) {
    Handle dir_handle;
    Log("OpenDirectory\n");
    if (!ParseResult(FSUSER_OpenDirectory(&dir_handle, archive, MakePath(src_path)))) {
        return {};
    }

    u32 size;
    std::vector<FS_DirectoryEntry> entries;
    for (;;) {
        Log("Dir_Read\n");
        FS_DirectoryEntry tmp;
        if (!ParseResult(FSDIR_Read(dir_handle, &size, 1, &tmp))) {
            return {};
        }
        if (size == 0)
            break;
        entries.push_back(tmp);
    }

    Log("Dir_Close\n");
    ParseResult(FSDIR_Close(dir_handle));

    return entries;
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
