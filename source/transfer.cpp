#include "util.h"
#include "transfer.h"

static void CopyDir(FS_Archive src_archive, const std::u16string& src_path,
    FS_Archive dst_archive, const std::u16string& dst_path) {
    FSUSER_CreateDirectory(dst_archive, MakePath(dst_path), 0);
    std::u16string src_path_fix = src_path.size() == 0 ? u"/" : src_path;
    auto entries = GetEntries(src_archive, src_path_fix);
    for (auto entry : entries) {
        std::u16string sub_name((const char16_t*)entry.name);
        if (entry.attributes & FS_ATTRIBUTE_DIRECTORY) {
            CopyDir(src_archive, src_path + u"/" + sub_name, dst_archive, dst_path + u"/" + sub_name);
        } else {
            Handle file;
            Log("OpenFile\n");
            if (!ParseResult(FSUSER_OpenFile(&file, src_archive,
                MakePath(src_path + u"/" + sub_name), FS_OPEN_READ, 0)))
                break;

            u64 size;
            Log("File_GetSize\n");
            if (!ParseResult(FSFILE_GetSize(file, &size)))
                break;

            std::vector<u8> buffer(size);
            u32 bytes;
            Log("File_Read\n");
            ParseResult(FSFILE_Read(file, &bytes, 0, buffer.data(), size));

            Log("File_Close\n");
            ParseResult(FSFILE_Close(file));

            Log("OpenFile\n");
            if (!ParseResult(FSUSER_OpenFile(&file, dst_archive,
                MakePath(dst_path + u"/" + sub_name), FS_OPEN_WRITE | FS_OPEN_CREATE, 0)))
                break;

            Log("File_Write\n");
            ParseResult(FSFILE_Write(file, &bytes, 0, buffer.data(), size, 0));

            Log("File_Close\n");
            ParseResult(FSFILE_Close(file));
        }
    }
}


static void Extract(FS_Archive archive, const std::string& label) {
    FS_Path sd_path{PATH_EMPTY, 0, nullptr};
    FS_Archive sd;
    Log("OpenArchive(SDMC)\n");
    if (ParseResult(FSUSER_OpenArchive(&sd, ARCHIVE_SDMC, sd_path))) {
        std::u16string main_path = u"/savegamebrowser";

        FSUSER_CreateDirectory(sd, MakePath(main_path), 0);

        std::u16string root = main_path + u"/" + ConvertString(label);
        FSUSER_DeleteDirectoryRecursively(sd, MakePath(root));

        CopyDir(archive, u"", sd, root);

        Log("CloseArchive(SDMC)\n");
        ParseResult(FSUSER_CloseArchive(sd));
    }

    Log("Extract done!\n");
}

static void Import(FS_Archive archive, const std::string& label) {
    FS_Path sd_path{PATH_EMPTY, 0, nullptr};
    FS_Archive sd;
    Log("OpenArchive(SDMC)\n");
    if (ParseResult(FSUSER_OpenArchive(&sd, ARCHIVE_SDMC, sd_path))) {
        std::u16string main_path = u"/savegamebrowser";

        FSUSER_CreateDirectory(sd, MakePath(main_path), 0);

        std::u16string root = main_path + u"/" + ConvertString(label);
        //FSUSER_DeleteDirectoryRecursively(archive, MakePath(u""));
        auto entries = GetEntries(archive, u"/");
        for (auto entry : entries) {
            std::u16string sub_name((const char16_t*)entry.name);
            if (entry.attributes & FS_ATTRIBUTE_DIRECTORY) {
                ParseResult(FSUSER_DeleteDirectoryRecursively(archive, MakePath(u"/" + sub_name)));
            } else {
                ParseResult(FSUSER_DeleteFile(archive, MakePath(u"/" + sub_name)));
            }
        }

        CopyDir(sd, root, archive, u"");

        Log("CloseArchive(SDMC)\n");
        ParseResult(FSUSER_CloseArchive(sd));

        Log("ControlArchive(Commit)\n");
        ParseResult(FSUSER_ControlArchive(archive, ARCHIVE_ACTION_COMMIT_SAVE_DATA, 0, 0, 0, 0));
    }

    Log("Import done!\n");
}

static void PrintUI(const std::string& label) {
    consoleClear();

    printf("\x1b[3;0H\x1b[31mArchive transfer\x1b[0m\n\n");
    printf("    B - back\n");
    printf("    Hold L and press X - import from SD card\n");
    printf("    Hold R and press Y - extract to SD card\n\n");

    printf("curent archive: %s\n", label.data());
    printf("Associated SDMC directory: sdmc:/savegamebrowser/%s", label.data());
}

void Scene_Transfer(FS_Archive archive, const std::string& label) {
    for (;;) {
        PrintUI(label);
        u32 key = ListenKey();
        u32 key_held = hidKeysHeld();
        switch (key) {
        case KEY_B:
            return;
        case KEY_X:
            if (key_held & KEY_L)
                Import(archive, label);
            break;
        case KEY_Y:
            if (key_held & KEY_R)
                Extract(archive, label);
            break;
        }
    }
}
