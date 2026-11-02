#include "fdk/types.h"
#include "fdk/io.h"
#include "fdk/gdt.h"
#include "fdk/idt.h"
#include "fdk/pit.h"
#include "fdk/vga.h"

static volatile u32 ticks = 0;

static void on_irq0(u32 irq, u32 *frame) {
    (void)irq; (void)frame;
    ticks++;
}

/* Scancode set 1, US layout, lower-case. */
static const char kbd_map[128] = {
    0,   27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,   'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,   '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0,   ' ',
};

static void on_irq1(u32 irq, u32 *frame) {
    (void)irq; (void)frame;
    u8 sc = fdk_inb(0x60);
    if (sc & 0x80) return;            /* release */
    if (sc >= 128) return;
    char c = kbd_map[sc];
    if (!c) return;

    /* Echo typed characters at row 7, column 0.. */
    static int kbd_col = 0;
    int row, col;
    fdk_vga_get_cursor(&row, &col);
    if (c == '\b') {
        if (kbd_col > 0) {
            kbd_col--;
            fdk_vga_putc_at(' ', 7, kbd_col);
        }
    } else if (c == '\n') {
        kbd_col = 0;
    } else {
        if (kbd_col >= FDK_VGA_COLS) kbd_col = 0;
        fdk_vga_putc_at(c, 7, kbd_col++);
    }
    (void)row; (void)col;
}

void kernel_main(void) {
    fdk_vga_init();

    fdk_vga_set_color(FDK_VGA_YELLOW, FDK_VGA_BLACK);
    fdk_vga_puts_at("Firmware Kit - VGA library demo", 0, 0);

    fdk_vga_set_color(FDK_VGA_LIGHT_GREY, FDK_VGA_BLACK);
    fdk_gdt_init();
    fdk_vga_puts_at("GDT: OK", 1, 0);

    fdk_idt_init();
    fdk_vga_puts_at("IDT: OK", 1, 10);

    fdk_irq_install_handler(0, on_irq0);
    fdk_irq_install_handler(1, on_irq1);
    fdk_pit_init(100);
    fdk_vga_puts_at("PIT: 100 Hz   KBD: IRQ1", 2, 0);

    fdk_vga_puts_at("Ticks: ",  4, 0);
    fdk_vga_puts_at("Type: ",   6, 0);

    fdk_sti();

    for (;;) {
        fdk_vga_puthex_at(ticks, 4, 7);
        fdk_hlt();
    }
}

__attribute__((section(".text.entry"), noinline))
void _start(void) {
    kernel_main();
    for (;;) __asm__ volatile ("cli; hlt");
}
