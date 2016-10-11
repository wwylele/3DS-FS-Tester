#include "util.h"
#include "archive.h"

static std::vector<u32> MakeBinaryPath_EMPTY(u32, u32) {
    return {};
}

static std::vector<u32> MakeBinaryPath_SD(u32 low, u32 high) {
    return {1, low, high};
}

static std::vector<u32> MakeBinaryPath_SelfExt(u32, u32) {
    return {1, 0x12345678, 0};
}


static std::vector<u32> MakeBinaryPath_NAND(u32 low, u32) {
    return {0, low};
}

static std::vector<u32> MakeBinaryPath_NANDExt(u32 low, u32) {
    return {0, low, 0x00048000};
}

const ArchiveIDDesc archive_id_descs[SUPPORT_ARCHIVE_TYPE] = {
   {ARCHIVE_SAVEDATA, "Self", false, false, MakeBinaryPath_EMPTY},
   {ARCHIVE_GAMECARD_SAVEDATA, "Gamecard Save", false, false, MakeBinaryPath_EMPTY},
   {ARCHIVE_USER_SAVEDATA, "SD Save", true, true, MakeBinaryPath_SD},
   {ARCHIVE_SYSTEM_SAVEDATA, "NAND Save", true, false, MakeBinaryPath_NAND},
   {ARCHIVE_EXTDATA, "Self Ext", false, false, MakeBinaryPath_SelfExt},
   {ARCHIVE_EXTDATA, "SD Ext", true, true, MakeBinaryPath_SD},
   {ARCHIVE_SHARED_EXTDATA, "NAND Ext", true, false, MakeBinaryPath_NANDExt},
   {ARCHIVE_SDMC, "SDMC", false, false, MakeBinaryPath_EMPTY},
   {ARCHIVE_SDMC_WRITE_ONLY, "SDMC WO", false, false, MakeBinaryPath_EMPTY},
};

FS_Path MakeBinaryPath(const ArchiveDesc& desc) {
    return MakePath(archive_id_descs[desc.archive_id_desc_index].binary_path_maker(
        desc.id_low, desc.id_high));
}

std::string MakeArchiveLabel(const ArchiveDesc& desc) {
    char id_string[9] = {};
    std::string label = archive_id_descs[desc.archive_id_desc_index].desc;
    if (archive_id_descs[desc.archive_id_desc_index].has_id_low) {
        sprintf(id_string, "%08lX", desc.id_low);
        label += " ";
        label += id_string;
    }
    if (archive_id_descs[desc.archive_id_desc_index].has_id_high) {
        sprintf(id_string, "%08lX", desc.id_high);
        label += " ";
        label += id_string;
    }
    return label;
}

static void PrintUI(const ArchiveDesc& desc) {
    consoleClear();
    printf("\x1b[0;0HPlease select a savegame to operate.\n");
    printf("A - OK\n");
    printf("B - cancel\n");
    printf("SELECT - touch archive\n\n");
    printf("Type (Use D-pad to change): %s\n", archive_id_descs[desc.archive_id_desc_index].desc);
    printf("ID low  (Press X to change): ");
    if (archive_id_descs[desc.archive_id_desc_index].has_id_low) {
        printf("%08lX\n", desc.id_low);
    } else {
        printf("--------\n");
    }
    printf("ID high (Press Y to change): ");
    if (archive_id_descs[desc.archive_id_desc_index].has_id_high) {
        printf("%08lX\n", desc.id_high);
    } else {
        printf("--------\n");
    }
}

static SwkbdCallbackResult UserInputIDCallback(
    void* user, const char** ppMessage, const char* text, size_t textlen) {
    if (textlen != 8) {
        *ppMessage = "Must input 8 digits.";
		return SWKBD_CALLBACK_CONTINUE;
    }

    for (int i = 0; i < 8; ++i) {
        char c = *text;
        ++text;
        if (c >= '0' && c <= '9')
            continue;
        if (c >= 'A' && c <= 'F')
            continue;
        if (c >= 'a' && c <= 'f')
            continue;
        *ppMessage = "Must input valid hex digits.";
		return SWKBD_CALLBACK_CONTINUE;
    }

	return SWKBD_CALLBACK_OK;
}

static bool UserInputID(u32& input) {
    static SwkbdState swkbd;
	static char mybuf[60];

	swkbdInit(&swkbd, SWKBD_TYPE_WESTERN, 2, 8);
	swkbdSetValidation(&swkbd, SWKBD_FIXEDLEN,
        SWKBD_FILTER_AT | SWKBD_FILTER_PERCENT | SWKBD_FILTER_BACKSLASH | SWKBD_FILTER_CALLBACK, 0);
	swkbdSetHintText(&swkbd, "Please input an 8-digit hex ID.");
    swkbdSetFeatures(&swkbd, SWKBD_FIXED_WIDTH);
    swkbdSetFilterCallback(&swkbd, UserInputIDCallback, nullptr);

	if (SWKBD_BUTTON_RIGHT != swkbdInputText(&swkbd, mybuf, sizeof(mybuf)))
        return false;

    input = 0;
    for (int i = 0; i < 8; ++i) {
        input *= 16;
        char c = mybuf[i];
        if (c >= '0' && c <= '9')
            input += c - '0';
        else if (c >= 'A' && c <= 'F')
            input += c - 'A' + 10;
        else if (c >= 'a' && c <= 'f')
            input += c - 'a' + 10;
    }

    return true;
}

ArchiveDesc Scene_Archive() {
    ArchiveDesc desc = {0, 0x00000000, 0x00040000};
    for (;;) {
        PrintUI(desc);
        u32 key = ListenKey();
        switch (key) {
        case KEY_DLEFT:
            if (desc.archive_id_desc_index == 0) {
                desc.archive_id_desc_index = SUPPORT_ARCHIVE_TYPE - 1;
            } else {
                --desc.archive_id_desc_index;
            }
            break;
        case KEY_DRIGHT:
            if (desc.archive_id_desc_index == SUPPORT_ARCHIVE_TYPE - 1) {
                desc.archive_id_desc_index = 0;
            } else {
                ++desc.archive_id_desc_index;
            }
            break;
        case KEY_A:
            return desc;
        case KEY_B:
            desc.archive_id_desc_index = -1;
            return desc;
        case KEY_X:
            if (archive_id_descs[desc.archive_id_desc_index].has_id_low)
                UserInputID(desc.id_low);
            break;
        case KEY_Y:
            if (archive_id_descs[desc.archive_id_desc_index].has_id_high)
                UserInputID(desc.id_high);
            break;
        case KEY_SELECT:
            {
                FS_Archive archive;
                Log("OpenArchive\n");
                if (ParseResult(FSUSER_OpenArchive(&archive,
                    archive_id_descs[desc.archive_id_desc_index].id, MakeBinaryPath(desc)))) {
                    Log("CloseArchive\n");
                    ParseResult(FSUSER_CloseArchive(archive));
                }
                break;
            }
        }
    }
}
