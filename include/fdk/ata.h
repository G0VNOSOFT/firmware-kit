#ifndef FDK_ATA_H
#define FDK_ATA_H

#include "types.h"

/*
 * ATA PIO-mode driver (primary/secondary channels, master/slave drives).
 *
 * Supports LBA28 addressing only (up to 128 GiB), which is more than enough
 * for boot-time filesystems and demo images. No IRQ, no DMA — polling only.
 *
 * Only depends on <fdk/types.h> and <fdk/io.h>.
 */

typedef struct {
    u8  channel;    /* 0 = primary (0x1F0), 1 = secondary (0x170) */
    u8  drive;      /* 0 = master, 1 = slave */
    u32 sectors;    /* total LBA28 sectors */
    int present;
    char model[41];
} fdk_ata_dev_t;

/* Probe a single drive. Returns FDK_OK if a usable ATA disk is found. */
fdk_status fdk_ata_probe(u8 channel, u8 drive, fdk_ata_dev_t *out);

/* Read `count` sectors starting at `lba` into `buf` (must be count*512 bytes). */
fdk_status fdk_ata_read(const fdk_ata_dev_t *dev, u32 lba, u32 count, void *buf);

#endif /* FDK_ATA_H */
