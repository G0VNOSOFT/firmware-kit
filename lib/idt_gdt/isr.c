#include "fdk/idt.h"

#define VGA ((volatile u16 *)0xB8000)

static const char *exc_name(u32 v) {
    static const char *names[32] = {
        "Divide Error",         "Debug",              "NMI",
        "Breakpoint",           "Overflow",           "Bound Range",
        "Invalid Opcode",       "Device Not Available",
        "Double Fault",         "Coprocessor Seg",    "Invalid TSS",
        "Segment Not Present",  "Stack Fault",        "General Protection",
        "Page Fault",           "Reserved",
        "x87 FP",               "Alignment Check",    "Machine Check",
        "SIMD FP",              "Virtualization",     "Control Protection",
        "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
        "Hypervisor Injected",  "VMM Communication",  "Security", "Reserved"
    };
    return (v < 32) ? names[v] : "Unknown";
}

static void vga_puts_at(const char *s, int row, int col) {
    volatile u16 *p = VGA + row * 80 + col;
    while (*s) *p++ = (u16)(0x0F00 | (u8)*s++);
}

static void vga_puthex32(u32 v, int row, int col) {
    static const char hex[] = "0123456789ABCDEF";
    volatile u16 *p = VGA + row * 80 + col;
    *p++ = '0'; *p++ = 'x';
    for (int i = 28; i >= 0; i -= 4)
        *p++ = (u16)(0x0F00 | (u8)hex[(v >> i) & 0xF]);
}

void fdk_isr_handler(u32 *frame) {
    /* frame: [0]=gs [1]=fs [2]=es [3]=ds [4..11]=pusha [12]=int_no
       [13]=err [14]=eip [15]=cs [16]=eflags */
    u32 int_no = frame[12];
    u32 err    = frame[13];
    u32 eip    = frame[14];
    u32 cs     = frame[15];
    u32 eflags = frame[16];

    vga_puts_at("!! CPU EXCEPTION !!", 8, 0);
    vga_puts_at("Type:   ",  9, 0); vga_puts_at(exc_name(int_no), 9, 8);
    vga_puts_at("Vector: ", 10, 0); vga_puthex32(int_no, 10, 8);
    vga_puts_at("Err:    ", 11, 0); vga_puthex32(err, 11, 8);
    vga_puts_at("EIP:    ", 12, 0); vga_puthex32(eip, 12, 8);
    vga_puts_at("CS:     ", 13, 0); vga_puthex32(cs, 13, 8);
    vga_puts_at("EFLAGS: ", 14, 0); vga_puthex32(eflags, 14, 8);
    vga_puts_at("System halted.", 16, 0);

    for (;;) __asm__ volatile ("cli; hlt");
}
