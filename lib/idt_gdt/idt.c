#include "fdk/idt.h"

#define FDK_IDT_ENTRIES 256

static struct fdk_idt_entry idt[FDK_IDT_ENTRIES];
static struct fdk_idt_ptr   idtp;

extern void fdk_idt_load(u32 idt_ptr_addr);
extern u32  fdk_isr_stub_table[32];
extern u32  fdk_irq_stub_table[16];

void fdk_idt_set_gate(int n, u32 handler, u16 sel, u8 flags) {
    idt[n].base_low  = (u16)(handler & 0xFFFF);
    idt[n].base_high = (u16)((handler >> 16) & 0xFFFF);
    idt[n].sel       = sel;
    idt[n].zero      = 0;
    idt[n].flags     = flags;
}

void fdk_idt_init(void) {
    int i;
    for (i = 0; i < FDK_IDT_ENTRIES; i++) {
        fdk_idt_set_gate(i, 0, 0x08, 0x0E);
    }
    for (i = 0; i < 32; i++) {
        fdk_idt_set_gate(i, fdk_isr_stub_table[i], 0x08, 0x8E);
    }
    for (i = 0; i < 16; i++) {
        fdk_idt_set_gate(32 + i, fdk_irq_stub_table[i], 0x08, 0x8E);
    }
    idtp.limit = (u16)(sizeof(idt) - 1);
    idtp.base  = (u32)&idt;
    fdk_idt_load((u32)&idtp);
}
