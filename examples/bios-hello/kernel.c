#include "fdk/types.h"
#include "fdk/io.h"
#include "fdk/gdt.h"
#include "fdk/idt.h"
#include "fdk/pit.h"
#include "fdk/vga.h"
#include "fdk/pci.h"

static volatile u32 ticks = 0;

static void on_irq0(u32 irq, u32 *frame) {
    (void)irq; (void)frame;
    ticks++;
}

static const char kbd_map[128] = {
    0,   27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,   'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,   '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0,   ' ',
};

static int kbd_col = 0;
static void on_irq1(u32 irq, u32 *frame) {
    (void)irq; (void)frame;
    u8 sc = fdk_inb(0x60);
    if (sc & 0x80) return;
    if (sc >= 128) return;
    char c = kbd_map[sc];
    if (!c) return;
    if (c == '\b') {
        if (kbd_col > 0) { kbd_col--; fdk_vga_putc_at(' ', 22, kbd_col); }
    } else if (c == '\n') {
        kbd_col = 0;
    } else {
        if (kbd_col >= FDK_VGA_COLS) kbd_col = 0;
        fdk_vga_putc_at(c, 22, kbd_col++);
    }
}

static void putdec_at(u32 v, int row, int col) {
    char buf[11]; int i = 10; buf[10] = 0;
    if (v == 0) buf[--i] = '0';
    else while (v > 0 && i > 0) { buf[--i] = (char)('0' + (v % 10)); v /= 10; }
    fdk_vga_puts_at(&buf[i], row, col);
}

struct enum_ctx {
    int row;
    int shown;
    int total;
};

static int enum_print(const fdk_pci_dev_t *d, void *user) {
    struct enum_ctx *ctx = user;
    ctx->total++;
    if (ctx->row >= 21) return 0;
    static const char hex[] = "0123456789ABCDEF";
    char buf[32]; int p = 0;
    buf[p++] = hex[(d->bus  >> 4) & 0xF];
    buf[p++] = hex[ d->bus        & 0xF];
    buf[p++] = ':';
    buf[p++] = hex[(d->slot >> 4) & 0xF];
    buf[p++] = hex[ d->slot       & 0xF];
    buf[p++] = '.';
    buf[p++] = hex[ d->func       & 0x7];
    buf[p++] = ' ';
    buf[p++] = hex[(d->vendor >> 12) & 0xF];
    buf[p++] = hex[(d->vendor >>  8) & 0xF];
    buf[p++] = hex[(d->vendor >>  4) & 0xF];
    buf[p++] = hex[ d->vendor        & 0xF];
    buf[p++] = ':';
    buf[p++] = hex[(d->device >> 12) & 0xF];
    buf[p++] = hex[(d->device >>  8) & 0xF];
    buf[p++] = hex[(d->device >>  4) & 0xF];
    buf[p++] = hex[ d->device        & 0xF];
    buf[p++] = ' ';
    buf[p++] = hex[(d->class_code >> 4) & 0xF];
    buf[p++] = hex[ d->class_code       & 0xF];
    buf[p++] = ':';
    buf[p++] = hex[(d->subclass >> 4) & 0xF];
    buf[p++] = hex[ d->subclass       & 0xF];
    buf[p++] = ' ';
    const char *cn = fdk_pci_class_name(d->class_code);
    while (*cn && p < 31) buf[p++] = *cn++;
    buf[p] = 0;
    fdk_vga_puts_at(buf, ctx->row++, 0);
    ctx->shown++;
    return 0;
}

void kernel_main(void) {
    fdk_vga_init();

    fdk_vga_set_color(FDK_VGA_YELLOW, FDK_VGA_BLACK);
    fdk_vga_puts_at("Firmware Kit - PCI enumeration demo", 0, 0);
    fdk_vga_set_color(FDK_VGA_LIGHT_GREY, FDK_VGA_BLACK);

    fdk_gdt_init();
    fdk_idt_init();
    fdk_irq_install_handler(0, on_irq0);
    fdk_irq_install_handler(1, on_irq1);
    fdk_pit_init(100);

    fdk_vga_puts_at("GDT: OK   IDT: OK   PIT: 100 Hz   KBD: IRQ1", 1, 0);
    fdk_vga_puts_at("Ticks: ", 2, 0);
    fdk_vga_puts_at("PCI devices found: ", 3, 0);

    fdk_sti();

    struct enum_ctx ctx = { .row = 5, .shown = 0, .total = 0 };
    fdk_pci_enumerate(enum_print, &ctx);
    putdec_at((u32)ctx.total, 3, 19);

    /* Showcase search: find the first VGA (class 03:00) */
    fdk_pci_dev_t vga;
    if (fdk_pci_find_by_class(0x03, 0x00, &vga) == FDK_OK) {
        static const char hex[] = "0123456789ABCDEF";
        char line[40]; int p = 0;
        const char *s = "First VGA: ";
        while (*s) line[p++] = *s++;
        line[p++] = hex[(vga.bus  >> 4) & 0xF];
        line[p++] = hex[ vga.bus        & 0xF];
        line[p++] = ':';
        line[p++] = hex[(vga.slot >> 4) & 0xF];
        line[p++] = hex[ vga.slot       & 0xF];
        line[p++] = '.';
        line[p++] = hex[ vga.func       & 0xF];
        line[p++] = ' ';
        line[p++] = hex[(vga.vendor >> 12) & 0xF];
        line[p++] = hex[(vga.vendor >>  8) & 0xF];
        line[p++] = hex[(vga.vendor >>  4) & 0xF];
        line[p++] = hex[ vga.vendor        & 0xF];
        line[p++] = ':';
        line[p++] = hex[(vga.device >> 12) & 0xF];
        line[p++] = hex[(vga.device >>  8) & 0xF];
        line[p++] = hex[(vga.device >>  4) & 0xF];
        line[p++] = hex[ vga.device        & 0xF];
        line[p] = 0;
        fdk_vga_puts_at(line, 21, 0);
    }

    fdk_vga_puts_at("Type: ", 22, 0);

    for (;;) {
        fdk_vga_puthex_at(ticks, 2, 7);
        fdk_hlt();
    }
}

__attribute__((section(".text.entry"), noinline))
void _start(void) {
    kernel_main();
    for (;;) __asm__ volatile ("cli; hlt");
}
