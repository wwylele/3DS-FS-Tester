#include "util.h"
#include "format.h"
#include "dir.h"
#include "archive.h"
#include "transfer.h"

static void PrintUI(const ArchiveDesc& desc) {
    consoleClear();

    printf("\x1b[3;0HSavegame Browser  by wwylele\n\n");
    printf("    A - open archive\n");
    printf("    X - format save / extsave\n");
    printf("    Y - select archive\n");
    printf("    SELECT - transfer archive\n\n");
    auto label = MakeArchiveLabel(desc);
    printf("current archive: %s", label.data());
}

int main() {
    fsInit();
    InitConsole();

    ArchiveDesc desc = {0, 0, 0};

    std::string archive_label;

    for (;;) {
        PrintUI(desc);
        u32 key = ListenKey();
        switch (key) {
        case KEY_A:
            {
                FS_Archive archive;
                Log("OpenArchive\n");
                if (ParseResult(FSUSER_OpenArchive(&archive,
                    archive_id_descs[desc.archive_id_desc_index].id, MakeBinaryPath(desc)))) {
                    Scene_Dir(archive, u"");
                    Log("CloseArchive\n");
                    ParseResult(FSUSER_CloseArchive(archive));
                }
            }
            break;
        case KEY_X:
            Scene_Format(archive_id_descs[desc.archive_id_desc_index].id, MakeBinaryPath(desc));
            break;
        case KEY_Y:
            {
                ArchiveDesc new_desc = Scene_Archive();
                if (new_desc.archive_id_desc_index != -1)
                    desc = new_desc;
            }
            break;
        case KEY_START:
            goto EXIT;
        case KEY_SELECT:
            {
                FS_Archive archive;
                Log("OpenArchive\n");
                if (ParseResult(FSUSER_OpenArchive(&archive,
                    archive_id_descs[desc.archive_id_desc_index].id, MakeBinaryPath(desc)))) {
                    Scene_Transfer(archive, MakeArchiveLabel(desc));
                    Log("CloseArchive\n");
                    ParseResult(FSUSER_CloseArchive(archive));
                }
            }
        }
    }
EXIT:

	gfxExit();
	return 0;
}
