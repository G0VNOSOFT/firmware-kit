#include "fdk/types.h"
#include "fdk/io.h"
#include "fdk/gdt.h"
#include "fdk/idt.h"
#include "fdk/pit.h"

#define VGA ((volatile u16 *)0xB8000)

static volatile u32 ticks = 0;
static volatile int kbd_col = 0;

static void vga_clear(void) {
    for (int i = 0; i < 80 * 25; i++) VGA[i] = 0x0F20;
}

static void vga_set_cursor(int row, int col) {
    u16 pos = (u16)(row * 80 + col);
    fdk_outb(0x3D4, 0x0F); fdk_outb(0x3D5, (u8)(pos & 0xFF));
    fdk_outb(0x3D4, 0x0E); fdk_outb(0x3D5, (u8)((pos >> 8) & 0xFF));
}

static void vga_puts_at(const char *s, int row, int col) {
    volatile u16 *p = VGA + row * 80 + col;
    while (*s) *p++ = (u16)(0x0F00 | (u8)*s++);
}

static void vga_puthex32(u32 v, int row, int col) {
    static const char hex[] = "0123456789ABCDEF";
    volatile u16 *p = VGA + row * 80 + col;
    for (int i = 28; i >= 0; i -= 4)
        *p++ = (u16)(0x0F00 | (u8)hex[(v >> i) & 0xF]);
}

static void on_irq0(u32 irq, u32 *frame) {
    (void)irq; (void)frame;
    ticks++;
}

/* Scancode set 1, US layout, нижний регистр. */
static const char kbd_map[128] = {
    0,   27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,   'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,   '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0,   ' ',
    /* 58..127 — нули (F-клавиши, стрелки, numpad и т.п.) */
};

static void on_irq1(u32 irq, u32 *frame) {
    (void)irq; (void)frame;
    u8 sc = fdk_inb(0x60);
    if (sc & 0x80) return;            /* release */
    if (sc >= 128) return;
    char c = kbd_map[sc];
    if (!c) return;

    if (c == '\b') {
        if (kbd_col > 0) kbd_col--;
        VGA[6 * 80 + kbd_col] = 0x0F20;
        return;
    }
    if (c == '\n') { kbd_col = 0; return; }

    if (kbd_col >= 80) kbd_col = 0;
    VGA[6 * 80 + kbd_col] = (u16)(0x0F00 | (u8)c);
    kbd_col++;
}

void kernel_main(void) {
    vga_clear();
    vga_set_cursor(7, 0);

    vga_puts_at("Firmware Kit - GDT / IDT / IRQ0 / IRQ1 demo", 0, 0);

    fdk_gdt_init();
    vga_puts_at("GDT: OK", 1, 0);

    fdk_idt_init();
    vga_puts_at("IDT: OK", 1, 10);

    fdk_irq_install_handler(0, on_irq0);
    fdk_irq_install_handler(1, on_irq1);
    fdk_pit_init(100);
    vga_puts_at("PIT: 100 Hz  KBD: IRQ1", 2, 0);

    vga_puts_at("Ticks: ", 4, 0);
    vga_puts_at("Type something:", 5, 0);
    vga_puts_at("_", 6, 0);

    __asm__ volatile ("int $0x03");
    fdk_sti();

    for (;;) {
        vga_puthex32(ticks, 4, 7);
        fdk_hlt();
    }
}

__attribute__((section(".text.entry"), noinline))
void _start(void) {
    kernel_main();
    for (;;) __asm__ volatile ("cli; hlt");
}
