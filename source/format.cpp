#include "util.h"
#include "format.h"
#include "ext_icon_bin.h"

#define NUM_PAD_WIDTH 5
#define NUM_PAD_ROW 5
static int num_pad[NUM_PAD_ROW][NUM_PAD_WIDTH] = {
    {0, 0, 1, 2, 3},
    {0, 0, 0, 0, 7},
    {0, 0, 4, 5, 6},
    {0, 0, 0, 0, 9},
    {0, 0, 5, 1, 2},
};

static int num_pad_row = 0, num_pad_i = 0;

static const FS_ExtSaveDataInfo self_ext {
    MEDIATYPE_SD, 0, 0, 0x0000000012345678ULL, 0
};


static void PrintUI() {
    consoleClear();

    printf("\x1b[3;0H\x1b[31mFormat save / extsave\x1b[0m\n\n");
    printf("Hold SELECT and press A - format current save!\n");
    printf("Hold SELECT and press X - delete self extsave!\n");
    printf("Hold SELECT and press Y - create self extsave!\n");
    printf("B - back\n");
    printf("D-pad - move cursor\n");
    printf("L/R - edit attributes\n");

    printf("\x1b[12;0H");
    printf("  Dir count    =\n");
    printf("  Dir buckets  =\n");
    printf("  File count   =\n");
    printf("  File buckets =\n");
    printf("  Blocks       =\n");

    for (int row = 0; row < NUM_PAD_ROW; ++row) {
        for (int i = 0; i < NUM_PAD_WIDTH; ++i) {
            printf("\x1b[%d;%dH\x1b[%dm%d\x1b[0m", 12 + row, 18 + i,
            (row == num_pad_row && i == num_pad_i) ? 33 : 37, num_pad[row][i]);
        }
    }
}

static u32 GetNumber(int row) {
    u32 result = 0;
    for (int i = 0; i < NUM_PAD_WIDTH; ++i) {
        result *= 10;
        result += num_pad[row][i];
    }
    return result;
}

static void Format(FS_ArchiveID archive_id, const FS_Path& archive_path) {
    u32 dir_count = GetNumber(0);
    u32 dir_buckets = GetNumber(1);
    u32 file_count = GetNumber(2);
    u32 file_buckets = GetNumber(3);
    u32 blocks = GetNumber(4);
    Log("FormatSaveData(dir_count=%d, dir_buckets=%d, file_count=%d, file_buckets=%d, blocks%d)\n",
        dir_count, dir_buckets, file_count, file_buckets, blocks);

    ParseResult(FSUSER_FormatSaveData(archive_id, archive_path, blocks,
        dir_count, file_count, dir_buckets, file_buckets, true));
}

static void CreateExt() {
    u32 dir_count = GetNumber(0);
    u32 file_count = GetNumber(2);
    Log("CreateExtSaveData\n");

    std::vector<u8> icon(14016);
    std::memcpy(icon.data(), ext_icon_bin, 14016);
    ParseResult(FSUSER_CreateExtSaveData(self_ext, dir_count, file_count, -1, 14016, icon.data()));
}

void Scene_Format(FS_ArchiveID archive_id, const FS_Path& archive_path) {
    for (;;) {
        PrintUI();
        u32 key = ListenKey();
        u32 key_held = hidKeysHeld();
        switch (key) {
        case KEY_DUP:
            if (num_pad_row)
                --num_pad_row;
            break;
        case KEY_DDOWN:
            if (num_pad_row != NUM_PAD_ROW - 1)
                ++num_pad_row;
            break;
        case KEY_DLEFT:
            if (num_pad_i)
                --num_pad_i;
            break;
        case KEY_DRIGHT:
            if (num_pad_i != NUM_PAD_WIDTH - 1)
                ++num_pad_i;
            break;
        case KEY_L:
            --num_pad[num_pad_row][num_pad_i];
            if (num_pad[num_pad_row][num_pad_i] == -1)
                num_pad[num_pad_row][num_pad_i] = 9;
            break;
        case KEY_R:
            ++num_pad[num_pad_row][num_pad_i];
            if (num_pad[num_pad_row][num_pad_i] == 10)
                num_pad[num_pad_row][num_pad_i] = 0;
            break;
        case KEY_A:
            if (key_held & KEY_SELECT)
                Format(archive_id, archive_path);
            break;
        case KEY_X:
            if (key_held & KEY_SELECT) {
                Log("DeleteExtSaveData\n");
                ParseResult(FSUSER_DeleteExtSaveData(self_ext));
            }
            break;
        case KEY_Y:
            if (key_held & KEY_SELECT)
                CreateExt();
            break;
        case KEY_B:
            return;
        }
    }
}
