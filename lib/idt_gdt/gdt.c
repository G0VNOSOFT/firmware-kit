#include "fdk/gdt.h"

extern void fdk_gdt_flush(u32 gdt_ptr_addr);

static struct fdk_gdt_entry gdt[3];
static struct fdk_gdt_ptr   gdtp;

static void set_gate(int i, u32 base, u32 limit, u8 access, u8 gran) {
    gdt[i].base_low  = (u16)(base & 0xFFFF);
    gdt[i].base_mid  = (u8)((base >> 16) & 0xFF);
    gdt[i].base_high = (u8)((base >> 24) & 0xFF);
    gdt[i].limit_low = (u16)(limit & 0xFFFF);
    gdt[i].gran      = (u8)(((limit >> 16) & 0x0F) | (gran & 0xF0));
    gdt[i].access    = access;
}

void fdk_gdt_init(void) {
    set_gate(0, 0, 0,       0x00, 0x00);   /* null descriptor */
    set_gate(1, 0, 0xFFFFF, 0x9A, 0xCF);   /* kernel code: ring 0, exec/read */
    set_gate(2, 0, 0xFFFFF, 0x92, 0xCF);   /* kernel data: ring 0, read/write */
    gdtp.limit = (u16)(sizeof(gdt) - 1);
    gdtp.base  = (u32)&gdt;
    fdk_gdt_flush((u32)&gdtp);
}
