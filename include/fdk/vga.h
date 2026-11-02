#ifndef FDK_VGA_H
#define FDK_VGA_H

#include "types.h"

/*
 * VGA text-mode output (80x25, color).
 *
 * Two output styles are provided:
 *   - positional  (fdk_vga_putc_at / puts_at / puthex_at)  -- for UIs
 *   - terminal    (fdk_vga_putc / puts / puthex / putdec)   -- for logs,
 *                   with automatic scrolling and line-wrap.
 *
 * The module keeps its own cursor and attribute state.
 * It only depends on <fdk/types.h> and <fdk/io.h>.
 */

#define FDK_VGA_COLS 80
#define FDK_VGA_ROWS 25

/* Standard VGA palette, indexed 0..15 */
enum fdk_vga_color {
    FDK_VGA_BLACK         = 0,
    FDK_VGA_BLUE          = 1,
    FDK_VGA_GREEN         = 2,
    FDK_VGA_CYAN          = 3,
    FDK_VGA_RED           = 4,
    FDK_VGA_MAGENTA       = 5,
    FDK_VGA_BROWN         = 6,
    FDK_VGA_LIGHT_GREY    = 7,
    FDK_VGA_DARK_GREY     = 8,
    FDK_VGA_LIGHT_BLUE    = 9,
    FDK_VGA_LIGHT_GREEN   = 10,
    FDK_VGA_LIGHT_CYAN    = 11,
    FDK_VGA_LIGHT_RED     = 12,
    FDK_VGA_LIGHT_MAGENTA = 13,
    FDK_VGA_YELLOW        = 14,
    FDK_VGA_WHITE         = 15
};

/* ---- lifecycle ---- */
void fdk_vga_init(void);            /* reset state, clear screen */
void fdk_vga_clear(void);           /* clear with current attribute */

/* ---- attributes ---- */
void fdk_vga_set_color(u8 fg, u8 bg); /* e.g. FDK_VGA_LIGHT_GREY, FDK_VGA_BLACK */
u8   fdk_vga_get_color(void);

/* ---- cursor ---- */
void fdk_vga_set_cursor(int row, int col);   /* clamps to 0..COLS-1 / 0..ROWS-1 */
void fdk_vga_get_cursor(int *row, int *col);

/* ---- positional output (does not move the cursor) ---- */
void fdk_vga_putc_at(char c, int row, int col);
void fdk_vga_puts_at(const char *s, int row, int col);
void fdk_vga_puthex_at(u32 v, int row, int col);   /* "0x1234ABCD" */

/* ---- terminal-style output (advances cursor, wraps, scrolls) ---- */
void fdk_vga_putc(char c);          /* \n, \r, \t, \b handled */
void fdk_vga_puts(const char *s);
void fdk_vga_puthex(u32 v);         /* with "0x" prefix */
void fdk_vga_putdec(u32 v);
void fdk_vga_newline(void);
void fdk_vga_scroll(void);          /* shift content up one line */

#endif /* FDK_VGA_H */
