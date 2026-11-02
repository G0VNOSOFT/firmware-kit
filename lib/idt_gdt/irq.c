#include "fdk/idt.h"
#include "fdk/io.h"

static fdk_irq_handler_t irq_handlers[16];
static int pic_remapped = 0;

static void pic_remap(void) {
    fdk_outb(0x20, 0x11); fdk_io_wait();
    fdk_outb(0xA0, 0x11); fdk_io_wait();
    fdk_outb(0x21, 0x20); fdk_io_wait();   /* master vector offset 32 */
    fdk_outb(0xA1, 0x28); fdk_io_wait();   /* slave  vector offset 40 */
    fdk_outb(0x21, 0x04); fdk_io_wait();   /* slave on IRQ2 */
    fdk_outb(0xA1, 0x02); fdk_io_wait();
    fdk_outb(0x21, 0x01); fdk_io_wait();
    fdk_outb(0xA1, 0x01); fdk_io_wait();
    fdk_outb(0x21, 0x00);
    fdk_outb(0xA1, 0x00);
}

static void pic_eoi(u32 irq) {
    if (irq >= 8) fdk_outb(0xA0, 0x20);
    fdk_outb(0x20, 0x20);
}

void fdk_irq_install_handler(int irq, fdk_irq_handler_t h) {
    if (irq < 0 || irq > 15) return;
    if (!pic_remapped) { pic_remap(); pic_remapped = 1; }
    irq_handlers[irq] = h;
}

void fdk_irq_uninstall_handler(int irq) {
    if (irq < 0 || irq > 15) return;
    irq_handlers[irq] = 0;
}

void fdk_irq_handler(u32 *frame) {
    /* frame layout (from stub):
       0  gs, 1 fs, 2 es, 3 ds,
       4..11 pusha (edi, esi, ebp, esp, ebx, edx, ecx, eax),
       12 int_no, 13 err_code, 14 eip, 15 cs, 16 eflags */
    u32 int_no = frame[12];
    u32 irq    = int_no - 32;
    if (irq < 16 && irq_handlers[irq]) {
        irq_handlers[irq](irq, frame);
    }
    pic_eoi(irq);
}
