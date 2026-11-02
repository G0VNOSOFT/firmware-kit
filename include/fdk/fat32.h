#ifndef FDK_FAT32_H
#define FDK_FAT32_H

#include "types.h"

/*
 * Read-only FAT32 filesystem.
 *
 * Pluggable block layer: pass a `read` callback. Nothing here knows about ATA.
 *
 * Supported:
 *   - BPB parsing (bytes-per-sector must be 512, sectors-per-cluster power of 2)
 *   - 8.3 short names only (LFN entries are skipped)
 *   - files in the root directory
 *   - cluster-chain traversal
 *
 * Not yet: LFN, subdirectories, FAT12/FAT16, write.
 */

typedef fdk_status (*fdk_fat32_read_fn)(void *ctx, u32 lba, u32 count, void *buf);

typedef struct {
    void              *ctx;
    fdk_fat32_read_fn  read;

    u16 bytes_per_sector;
    u8  sectors_per_cluster;
    u8  num_fats;
    u32 fat_start;          /* LBA of first FAT */
    u32 fat_sectors;        /* sectors per FAT */
    u32 data_start;         /* LBA of first data cluster (cluster #2) */
    u32 root_cluster;
} fdk_fat32_fs_t;

typedef struct {
    u32  cluster;
    u32  size;
    u8   attr;
    char name[13];          /* "NAME.EXT\0" */
} fdk_fat32_entry_t;

fdk_status fdk_fat32_mount(fdk_fat32_fs_t *fs, void *ctx, fdk_fat32_read_fn read);

/* Look up a file by name in the root directory. Case-insensitive, "FOO.TXT" form. */
fdk_status fdk_fat32_lookup(fdk_fat32_fs_t *fs, const char *name, fdk_fat32_entry_t *out);

/* Read up to max_size bytes of the file into buf. */
fdk_status fdk_fat32_read(fdk_fat32_fs_t *fs, const fdk_fat32_entry_t *e,
                          void *buf, u32 max_size, u32 *read_size);

#endif /* FDK_FAT32_H */
