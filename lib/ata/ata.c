#include "fdk/ata.h"
#include "fdk/io.h"

#define ATA_PRI_BASE 0x1F0
#define ATA_PRI_CTRL 0x3F6
#define ATA_SEC_BASE 0x170
#define ATA_SEC_CTRL 0x376

#define R_DATA     0
#define R_ERROR    1
#define R_FEATURES 1
#define R_SECCOUNT 2
#define R_LBA_LO   3
#define R_LBA_MID  4
#define R_LBA_HI   5
#define R_DRIVE    6
#define R_STATUS   7
#define R_CMD      7

#define SR_BSY 0x80
#define SR_DRDY 0x40
#define SR_DF   0x20
#define SR_DRQ  0x08
#define SR_ERR  0x01

static u16 base(u8 ch) { return ch == 0 ? ATA_PRI_BASE : ATA_SEC_BASE; }
static u16 ctrl(u8 ch) { return ch == 0 ? ATA_PRI_CTRL : ATA_SEC_CTRL; }

static void io_wait(u8 ch) {
    for (int i = 0; i < 4; i++) (void)fdk_inb(ctrl(ch));
}

static int wait_not_busy(u16 b, u32 tries) {
    while (tries--) {
        u8 s = fdk_inb(b + R_STATUS);
        if (s == 0x00) return -2;   /* no device */
        if (!(s & SR_BSY)) return 0;
    }
    return -1;
}

fdk_status fdk_ata_probe(u8 ch, u8 drv, fdk_ata_dev_t *out) {
    if (!out) return FDK_EINVAL;
    out->present = 0;

    u16 b = base(ch);
    fdk_outb(b + R_DRIVE, (u8)(0xE0 | (drv << 4)));
    io_wait(ch);

    fdk_outb(b + R_SECCOUNT, 0);
    fdk_outb(b + R_LBA_LO,   0);
    fdk_outb(b + R_LBA_MID,  0);
    fdk_outb(b + R_LBA_HI,   0);
    fdk_outb(b + R_CMD,      0xEC);   /* IDENTIFY */

    u8 st = fdk_inb(b + R_STATUS);
    if (st == 0x00) return FDK_ENOENT;   /* no drive */

    if (wait_not_busy(b, 1000000) != 0) return FDK_EIO;

    /* ATAPI (0x14/0xEB) or SATA (0x3C/0xC3) — not supported here. */
    if (fdk_inb(b + R_LBA_MID) || fdk_inb(b + R_LBA_HI)) return FDK_ENOTSUP;

    while (!(fdk_inb(b + R_STATUS) & SR_DRQ)) { }

    u16 id[256];
    for (int i = 0; i < 256; i++) id[i] = fdk_inw(b + R_DATA);

    out->channel = ch;
    out->drive   = drv;
    out->sectors = ((u32)id[61] << 16) | (u32)id[60];
    out->present = 1;

    for (int i = 0; i < 20; i++) {
        out->model[i * 2]     = (char)(id[27 + i] >> 8);
        out->model[i * 2 + 1] = (char)(id[27 + i] & 0xFF);
    }
    out->model[40] = 0;
    for (int i = 39; i >= 0 && out->model[i] == ' '; i--) out->model[i] = 0;

    return FDK_OK;
}

fdk_status fdk_ata_read(const fdk_ata_dev_t *dev, u32 lba, u32 count, void *buf) {
    if (!dev || !dev->present || !buf) return FDK_EINVAL;
    if (lba + count > dev->sectors) return FDK_EINVAL;

    u16 b = base(dev->channel);
    u8 *p = (u8 *)buf;

    while (count > 0) {
        u32 chunk = count > 256 ? 256 : count;

        if (wait_not_busy(b, 1000000) != 0) return FDK_EIO;

        fdk_outb(b + R_DRIVE, (u8)(0xE0 | (dev->drive << 4) |
                                    ((lba >> 24) & 0x0F)));
        io_wait(dev->channel);
        fdk_outb(b + R_SECCOUNT, (u8)(chunk == 256 ? 0 : chunk));
        fdk_outb(b + R_LBA_LO,   (u8)(lba        & 0xFF));
        fdk_outb(b + R_LBA_MID,  (u8)((lba >>  8) & 0xFF));
        fdk_outb(b + R_LBA_HI,   (u8)((lba >> 16) & 0xFF));
        fdk_outb(b + R_CMD,      0x20);   /* READ SECTORS */

        for (u32 s = 0; s < chunk; s++) {
            if (wait_not_busy(b, 1000000) != 0) return FDK_EIO;
            while (!(fdk_inb(b + R_STATUS) & SR_DRQ)) {
                if (fdk_inb(b + R_STATUS) & SR_ERR) return FDK_EIO;
            }
            for (int i = 0; i < 256; i++) {
                u16 w = fdk_inw(b + R_DATA);
                *p++ = (u8)(w & 0xFF);
                *p++ = (u8)(w >> 8);
            }
        }
        lba   += chunk;
        count -= chunk;
    }
    return FDK_OK;
}
