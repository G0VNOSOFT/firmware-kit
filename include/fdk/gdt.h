#ifndef FDK_GDT_H
#define FDK_GDT_H

#include "types.h"

struct fdk_gdt_entry {
    u16 limit_low;
    u16 base_low;
    u8  base_mid;
    u8  access;
    u8  gran;
    u8  base_high;
} FDK_PACKED;

struct fdk_gdt_ptr {
    u16 limit;
    u32 base;
} FDK_PACKED;

void fdk_gdt_init(void);

#endif /* FDK_GDT_H */
