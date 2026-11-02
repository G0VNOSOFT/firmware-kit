#include "fdk/vga.h"
#include "fdk/io.h"

#define VGA_MEM       ((volatile u16 *)0xB8000)
#define VGA_PORT_CTRL 0x3D4
#define VGA_PORT_DATA 0x3D5

static u8  cur_attr = 0x07;   /* light grey on black */
static int cur_row  = 0;
static int cur_col  = 0;

static inline u16 cell(char c, u8 attr) {
    return (u16)(((u16)attr << 8) | (u8)c);
}

static void hw_set_cursor(int row, int col) {
    u16 pos = (u16)(row * FDK_VGA_COLS + col);
    fdk_outb(VGA_PORT_CTRL, 0x0F); fdk_outb(VGA_PORT_DATA, (u8)(pos & 0xFF));
    fdk_outb(VGA_PORT_CTRL, 0x0E); fdk_outb(VGA_PORT_DATA, (u8)((pos >> 8) & 0xFF));
}

void fdk_vga_init(void) {
    cur_attr = 0x07;
    cur_row = cur_col = 0;
    fdk_vga_clear();
}

void fdk_vga_clear(void) {
    for (int i = 0; i < FDK_VGA_COLS * FDK_VGA_ROWS; i++)
        VGA_MEM[i] = cell(' ', cur_attr);
    cur_row = cur_col = 0;
    hw_set_cursor(0, 0);
}

void fdk_vga_set_color(u8 fg, u8 bg) {
    cur_attr = (u8)(((bg & 0x0F) << 4) | (fg & 0x0F));
}

u8 fdk_vga_get_color(void) { return cur_attr; }

void fdk_vga_set_cursor(int row, int col) {
    if (row < 0) row = 0;
    if (row >= FDK_VGA_ROWS) row = FDK_VGA_ROWS - 1;
    if (col < 0) col = 0;
    if (col >= FDK_VGA_COLS) col = FDK_VGA_COLS - 1;
    cur_row = row; cur_col = col;
    hw_set_cursor(row, col);
}

void fdk_vga_get_cursor(int *row, int *col) {
    if (row) *row = cur_row;
    if (col) *col = cur_col;
}

void fdk_vga_putc_at(char c, int row, int col) {
    if (row < 0 || row >= FDK_VGA_ROWS) return;
    if (col < 0 || col >= FDK_VGA_COLS) return;
    VGA_MEM[row * FDK_VGA_COLS + col] = cell(c, cur_attr);
}

void fdk_vga_puts_at(const char *s, int row, int col) {
    if (!s) return;
    for (int i = 0; s[i] && col + i < FDK_VGA_COLS; i++)
        fdk_vga_putc_at(s[i], row, col + i);
}

void fdk_vga_puthex_at(u32 v, int row, int col) {
    static const char hex[] = "0123456789ABCDEF";
    char buf[11];
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 0; i < 8; i++)
        buf[2 + i] = hex[(v >> (28 - i * 4)) & 0xF];
    buf[10] = 0;
    fdk_vga_puts_at(buf, row, col);
}

void fdk_vga_scroll(void) {
    for (int row = 1; row < FDK_VGA_ROWS; row++)
        for (int col = 0; col < FDK_VGA_COLS; col++)
            VGA_MEM[(row - 1) * FDK_VGA_COLS + col] = VGA_MEM[row * FDK_VGA_COLS + col];
    for (int col = 0; col < FDK_VGA_COLS; col++)
        VGA_MEM[(FDK_VGA_ROWS - 1) * FDK_VGA_COLS + col] = cell(' ', cur_attr);
}

void fdk_vga_newline(void) {
    cur_col = 0;
    if (++cur_row >= FDK_VGA_ROWS) {
        fdk_vga_scroll();
        cur_row = FDK_VGA_ROWS - 1;
    }
    hw_set_cursor(cur_row, cur_col);
}

void fdk_vga_putc(char c) {
    if (c == '\n') { fdk_vga_newline(); return; }
    if (c == '\r') { cur_col = 0; hw_set_cursor(cur_row, cur_col); return; }
    if (c == '\t') {
        int next = (cur_col + 8) & ~7;
        while (cur_col < next && cur_col < FDK_VGA_COLS) fdk_vga_putc(' ');
        return;
    }
    if (c == '\b') {
        if (cur_col > 0) {
            cur_col--;
            VGA_MEM[cur_row * FDK_VGA_COLS + cur_col] = cell(' ', cur_attr);
            hw_set_cursor(cur_row, cur_col);
        }
        return;
    }
    VGA_MEM[cur_row * FDK_VGA_COLS + cur_col] = cell(c, cur_attr);
    if (++cur_col >= FDK_VGA_COLS) {
        cur_col = 0;
        if (++cur_row >= FDK_VGA_ROWS) {
            fdk_vga_scroll();
            cur_row = FDK_VGA_ROWS - 1;
        }
    }
    hw_set_cursor(cur_row, cur_col);
}

void fdk_vga_puts(const char *s) {
    if (!s) return;
    while (*s) fdk_vga_putc(*s++);
}

void fdk_vga_puthex(u32 v) {
    static const char hex[] = "0123456789ABCDEF";
    char buf[11];
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 0; i < 8; i++)
        buf[2 + i] = hex[(v >> (28 - i * 4)) & 0xF];
    buf[10] = 0;
    fdk_vga_puts(buf);
}

void fdk_vga_putdec(u32 v) {
    char buf[11];
    int i = 10;
    buf[i] = 0;
    if (v == 0) buf[--i] = '0';
    else while (v && i > 0) { buf[--i] = (char)('0' + (v % 10)); v /= 10; }
    fdk_vga_puts(&buf[i]);
}
