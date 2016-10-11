#include <3ds.h>
#include <vector>
#include <tuple>

struct ArchiveIDDesc {
    FS_ArchiveID id;
    const char* desc;
    bool has_id_low;
    bool has_id_high;
    std::vector<u32> (*binary_path_maker)(u32 id_low, u32 id_high);
};

#define SUPPORT_ARCHIVE_TYPE 9
extern const ArchiveIDDesc archive_id_descs[SUPPORT_ARCHIVE_TYPE];

struct ArchiveDesc {
    int archive_id_desc_index;
    u32 id_low;
    u32 id_high;
};

FS_Path MakeBinaryPath(const ArchiveDesc& desc);

std::string MakeArchiveLabel(const ArchiveDesc& desc);

ArchiveDesc Scene_Archive();
