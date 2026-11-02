#ifndef FDK_IDT_H
#define FDK_IDT_H

#include "types.h"

struct fdk_idt_entry {
    u16 base_low;
    u16 sel;
    u8  zero;
    u8  flags;
    u16 base_high;
} FDK_PACKED;

struct fdk_idt_ptr {
    u16 limit;
    u32 base;
} FDK_PACKED;

typedef void (*fdk_irq_handler_t)(u32 irq, u32 *frame);

void fdk_idt_init(void);
void fdk_idt_set_gate(int n, u32 handler, u16 sel, u8 flags);
void fdk_irq_install_handler(int irq, fdk_irq_handler_t h);
void fdk_irq_uninstall_handler(int irq);

#endif /* FDK_IDT_H */
