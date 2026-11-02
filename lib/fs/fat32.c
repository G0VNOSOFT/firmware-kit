#include "fdk/fat32.h"

static u16 rd16(const u8 *p) { return (u16)(p[0] | (p[1] << 8)); }
static u32 rd32(const u8 *p) {
    return (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24);
}

static fdk_status read_cluster(fdk_fat32_fs_t *fs, u32 cluster, void *buf) {
    u32 lba = fs->data_start + (cluster - 2) * fs->sectors_per_cluster;
    return fs->read(fs->ctx, lba, fs->sectors_per_cluster, buf);
}

static fdk_status next_cluster(fdk_fat32_fs_t *fs, u32 cluster, u32 *out) {
    u32 off    = cluster * 4;
    u32 sector = fs->fat_start + off / fs->bytes_per_sector;
    u32 idx    = (off % fs->bytes_per_sector) / 4;
    u8  buf[512];
    if (fs->bytes_per_sector > sizeof(buf)) return FDK_ENOTSUP;
    fdk_status s = fs->read(fs->ctx, sector, 1, buf);
    if (s) return s;
    *out = rd32(buf + idx * 4) & 0x0FFFFFFF;
    return FDK_OK;
}

fdk_status fdk_fat32_mount(fdk_fat32_fs_t *fs, void *ctx, fdk_fat32_read_fn read) {
    if (!fs || !read) return FDK_EINVAL;
    u8 bs[512];
    fdk_status s = read(ctx, 0, 1, bs);
    if (s) return s;

    if (bs[510] != 0x55 || bs[511] != 0xAA) return FDK_EINVAL;

    u16 bps  = rd16(bs + 0x0B);
    u8  spc  = bs[0x0D];
    u16 rsv  = rd16(bs + 0x0E);
    u8  nf   = bs[0x10];
    u32 fat  = rd32(bs + 0x24);
    u32 root = rd32(bs + 0x2C);

    if (bps != 512)              return FDK_ENOTSUP;
    if (spc == 0 || (spc & (spc - 1))) return FDK_EINVAL;
    if (fat == 0)                return FDK_ENOTSUP;   /* not FAT32 */
    if (nf == 0)                 return FDK_EINVAL;

    fs->ctx                = ctx;
    fs->read               = read;
    fs->bytes_per_sector   = bps;
    fs->sectors_per_cluster= spc;
    fs->num_fats           = nf;
    fs->fat_start          = rsv;
    fs->fat_sectors        = fat;
    fs->data_start         = rsv + (u32)nf * fat;
    fs->root_cluster       = root;
    return FDK_OK;
}

static int name_match(const u8 *entry, const char *name) {
    char padded[11];
    for (int i = 0; i < 11; i++) padded[i] = ' ';

    int i = 0;
    while (*name && *name != '.' && i < 8) {
        char c = *name++;
        padded[i++] = (c >= 'a' && c <= 'z') ? (char)(c - 32) : c;
    }
    if (*name == '.') name++;
    int j = 0;
    while (*name && j < 3) {
        char c = *name++;
        padded[8 + j++] = (c >= 'a' && c <= 'z') ? (char)(c - 32) : c;
    }
    for (i = 0; i < 11; i++)
        if (entry[i] != (u8)padded[i]) return 0;
    return 1;
}

static void entry_name(const u8 *e, char *out) {
    int p = 0;
    for (int i = 0; i < 8 && e[i] != ' '; i++) out[p++] = (char)e[i];
    int has_ext = 0;
    for (int i = 8; i < 11; i++) if (e[i] != ' ') { has_ext = 1; break; }
    if (has_ext) {
        out[p++] = '.';
        for (int i = 8; i < 11 && e[i] != ' '; i++) out[p++] = (char)e[i];
    }
    out[p] = 0;
}

fdk_status fdk_fat32_lookup(fdk_fat32_fs_t *fs, const char *name, fdk_fat32_entry_t *out) {
    if (!fs || !name || !out) return FDK_EINVAL;

    u32 cluster = fs->root_cluster;
    u32 per_cluster = (u32)fs->sectors_per_cluster * fs->bytes_per_sector / 32;
    u8  buf[512 * 128];

    if (fs->sectors_per_cluster > 128) return FDK_ENOTSUP;

    while (cluster >= 2 && cluster < 0x0FFFFFF8) {
        fdk_status s = read_cluster(fs, cluster, buf);
        if (s) return s;

        for (u32 i = 0; i < per_cluster; i++) {
            u8 *e = buf + i * 32;
            if (e[0] == 0x00) return FDK_ENOENT;
            if (e[0] == 0xE5) continue;
            if (e[11] == 0x0F) continue;         /* LFN */
            if (e[11] & 0x08) continue;          /* volume label */
            if (name_match(e, name)) {
                out->cluster = ((u32)e[21] << 24) | ((u32)e[20] << 16) |
                               ((u32)e[27] <<  8) |  (u32)e[26];
                out->size    = rd32(e + 28);
                out->attr    = e[11];
                entry_name(e, out->name);
                return FDK_OK;
            }
        }
        if (next_cluster(fs, cluster, &cluster)) return FDK_EIO;
    }
    return FDK_ENOENT;
}

fdk_status fdk_fat32_read(fdk_fat32_fs_t *fs, const fdk_fat32_entry_t *e,
                          void *buf, u32 max_size, u32 *read_size) {
    if (!fs || !e || !buf) return FDK_EINVAL;
    if (read_size) *read_size = 0;
    if (e->attr & 0x10) return FDK_EINVAL;   /* directory */

    u32 want = e->size < max_size ? e->size : max_size;
    u32 done = 0;
    u32 cluster = e->cluster;
    u8 *p = buf;
    u8  tmp[512];

    while (cluster >= 2 && cluster < 0x0FFFFFF8 && done < want) {
        u32 lba = fs->data_start + (cluster - 2) * fs->sectors_per_cluster;
        for (u32 s = 0; s < fs->sectors_per_cluster && done < want; s++) {
            fdk_status st = fs->read(fs->ctx, lba + s, 1, tmp);
            if (st) return st;
            u32 chunk = want - done;
            if (chunk > 512) chunk = 512;
            for (u32 i = 0; i < chunk; i++) p[done + i] = tmp[i];
            done += chunk;
        }
        if (done >= want) break;
        if (next_cluster(fs, cluster, &cluster)) return FDK_EIO;
    }
    if (read_size) *read_size = done;
    return FDK_OK;
}
