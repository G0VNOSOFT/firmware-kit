#include "fdk/pci.h"
#include "fdk/io.h"

static u32 make_addr(u8 bus, u8 slot, u8 func, u8 offset) {
    return 0x80000000u
         | ((u32)bus  << 16)
         | ((u32)(slot & 0x1F) << 11)
         | ((u32)(func & 0x07) <<  8)
         | ((u32)offset & 0xFC);
}

u32 fdk_pci_read32(u8 bus, u8 slot, u8 func, u8 offset) {
    fdk_outl(FDK_PCI_ADDR_PORT, make_addr(bus, slot, func, offset));
    return fdk_inl(FDK_PCI_DATA_PORT);
}

void fdk_pci_write32(u8 bus, u8 slot, u8 func, u8 offset, u32 val) {
    fdk_outl(FDK_PCI_ADDR_PORT, make_addr(bus, slot, func, offset));
    fdk_outl(FDK_PCI_DATA_PORT, val);
}

u16 fdk_pci_read16(u8 bus, u8 slot, u8 func, u8 offset) {
    u32 dw = fdk_pci_read32(bus, slot, func, (u8)(offset & 0xFC));
    return (u16)((dw >> ((offset & 2) * 8)) & 0xFFFF);
}

u8 fdk_pci_read8(u8 bus, u8 slot, u8 func, u8 offset) {
    u32 dw = fdk_pci_read32(bus, slot, func, (u8)(offset & 0xFC));
    return (u8)((dw >> ((offset & 3) * 8)) & 0xFF);
}

fdk_status fdk_pci_get_device(u8 bus, u8 slot, u8 func, fdk_pci_dev_t *out) {
    if (!out) return FDK_EINVAL;
    u32 d00 = fdk_pci_read32(bus, slot, func, 0x00);
    u16 vendor = (u16)(d00 & 0xFFFF);
    if (vendor == 0x0000 || vendor == 0xFFFF) return FDK_ENOENT;

    out->bus    = bus;
    out->slot   = slot;
    out->func   = func;
    out->vendor = vendor;
    out->device = (u16)((d00 >> 16) & 0xFFFF);

    u32 d08 = fdk_pci_read32(bus, slot, func, 0x08);
    out->revision   = (u8)( d08        & 0xFF);
    out->prog_if    = (u8)((d08 >>  8) & 0xFF);
    out->subclass   = (u8)((d08 >> 16) & 0xFF);
    out->class_code = (u8)((d08 >> 24) & 0xFF);

    u32 d0C = fdk_pci_read32(bus, slot, func, 0x0C);
    out->header_type = (u8)((d0C >> 16) & 0xFF);

    return FDK_OK;
}

void fdk_pci_enumerate(fdk_pci_enum_cb cb, void *user) {
    if (!cb) return;
    for (u32 bus = 0; bus < 256; bus++) {
        for (u32 slot = 0; slot < 32; slot++) {
            fdk_pci_dev_t dev;
            if (fdk_pci_get_device((u8)bus, (u8)slot, 0, &dev) != FDK_OK) continue;
            if (cb(&dev, user)) return;
            if (dev.header_type & 0x80) {
                for (u32 func = 1; func < 8; func++) {
                    if (fdk_pci_get_device((u8)bus, (u8)slot, (u8)func, &dev) == FDK_OK)
                        if (cb(&dev, user)) return;
                }
            }
        }
    }
}

struct find_ctx {
    u8   class_code;
    u8   subclass;
    u16  vendor;
    u16  device;
    int  by_id;
    fdk_pci_dev_t *out;
    int  found;
};

static int find_cb(const fdk_pci_dev_t *d, void *user) {
    struct find_ctx *c = user;
    if (c->by_id) {
        if (d->vendor == c->vendor && d->device == c->device) {
            *c->out = *d;
            c->found = 1;
            return 1;
        }
    } else {
        if (d->class_code == c->class_code && d->subclass == c->subclass) {
            *c->out = *d;
            c->found = 1;
            return 1;
        }
    }
    return 0;
}

fdk_status fdk_pci_find_by_class(u8 class_code, u8 subclass, fdk_pci_dev_t *out) {
    if (!out) return FDK_EINVAL;
    struct find_ctx c = { class_code, subclass, 0, 0, 0, out, 0 };
    fdk_pci_enumerate(find_cb, &c);
    return c.found ? FDK_OK : FDK_ENOENT;
}

fdk_status fdk_pci_find_by_id(u16 vendor, u16 device, fdk_pci_dev_t *out) {
    if (!out) return FDK_EINVAL;
    struct find_ctx c = { 0, 0, vendor, device, 1, out, 0 };
    fdk_pci_enumerate(find_cb, &c);
    return c.found ? FDK_OK : FDK_ENOENT;
}

u32 fdk_pci_read_bar(const fdk_pci_dev_t *dev, int idx) {
    if (!dev) return 0;
    if ((dev->header_type & 0x7F) != 0x00) return 0;
    if (idx < 0 || idx > 5) return 0;
    return fdk_pci_read32(dev->bus, dev->slot, dev->func, (u8)(0x10 + idx * 4));
}

u32 fdk_pci_bar_size(const fdk_pci_dev_t *dev, int idx) {
    if (!dev) return 0;
    if ((dev->header_type & 0x7F) != 0x00) return 0;
    if (idx < 0 || idx > 5) return 0;
    u8  off  = (u8)(0x10 + idx * 4);
    u32 orig = fdk_pci_read32(dev->bus, dev->slot, dev->func, off);
    if (orig == 0) return 0;
    u32 mask = (orig & 1) ? 0xFFFFFFFCu : 0xFFFFFFF0u;  /* I/O vs MMIO */
    fdk_pci_write32(dev->bus, dev->slot, dev->func, off, 0xFFFFFFFFu);
    u32 sz = fdk_pci_read32(dev->bus, dev->slot, dev->func, off);
    fdk_pci_write32(dev->bus, dev->slot, dev->func, off, orig);
    sz &= mask;
    if (sz == 0) return 0;
    return (~sz) + 1;
}

const char *fdk_pci_class_name(u8 cc) {
    switch (cc) {
        case 0x00: return "unclassified";
        case 0x01: return "storage";
        case 0x02: return "network";
        case 0x03: return "display";
        case 0x04: return "multimedia";
        case 0x05: return "memory";
        case 0x06: return "bridge";
        case 0x07: return "comm";
        case 0x08: return "system";
        case 0x09: return "input";
        case 0x0A: return "docking";
        case 0x0B: return "processor";
        case 0x0C: return "serial bus";
        case 0x0D: return "wireless";
        case 0x0E: return "intelligent io";
        case 0x0F: return "satellite";
        case 0x10: return "crypto";
        case 0x11: return "signal proc";
        case 0x12: return "processing accel";
        case 0x13: return "non-essential";
        case 0x40: return "co-processor";
        case 0xFF: return "unassigned";
        default:   return "?";
    }
}
