#include "fdk/types.h"
#include "fdk/io.h"
#include "fdk/gdt.h"
#include "fdk/idt.h"
#include "fdk/pit.h"
#include "fdk/vga.h"
#include "fdk/pci.h"
#include "fdk/kbd.h"
#include "fdk/ata.h"
#include "fdk/fat32.h"

static volatile u32 ticks = 0;
static void on_irq0(u32 irq, u32 *f) { (void)irq; (void)f; ticks++; }

static void putdec_at(u32 v, int r, int c) {
    char b[11]; int i = 10; b[10] = 0;
    if (v == 0) b[--i] = '0';
    else while (v && i) { b[--i] = (char)('0' + v % 10); v /= 10; }
    fdk_vga_puts_at(&b[i], r, c);
}

struct enum_ctx { int row; int total; };
static int enum_print(const fdk_pci_dev_t *d, void *u) {
    struct enum_ctx *ctx = u;
    ctx->total++;
    if (ctx->row >= 12) return 0;
    static const char h[] = "0123456789ABCDEF";
    char b[40]; int p = 0;
    b[p++] = h[(d->bus >> 4) & 0xF]; b[p++] = h[d->bus & 0xF]; b[p++] = ':';
    b[p++] = h[(d->slot >> 4) & 0xF]; b[p++] = h[d->slot & 0xF]; b[p++] = '.';
    b[p++] = h[d->func & 0x7]; b[p++] = ' ';
    b[p++] = h[(d->vendor >> 12) & 0xF]; b[p++] = h[(d->vendor >> 8) & 0xF];
    b[p++] = h[(d->vendor >> 4) & 0xF]; b[p++] = h[d->vendor & 0xF]; b[p++] = ':';
    b[p++] = h[(d->device >> 12) & 0xF]; b[p++] = h[(d->device >> 8) & 0xF];
    b[p++] = h[(d->device >> 4) & 0xF]; b[p++] = h[d->device & 0xF]; b[p++] = ' ';
    const char *cn = fdk_pci_class_name(d->class_code);
    while (*cn && p < 39) b[p++] = *cn++;
    b[p] = 0;
    fdk_vga_puts_at(b, ctx->row++, 0);
    return 0;
}

/* Adapter: fdk_ata_dev_t -> fdk_fat32_read_fn */
static fdk_status ata_read_cb(void *ctx, u32 lba, u32 count, void *buf) {
    return fdk_ata_read((const fdk_ata_dev_t *)ctx, lba, count, buf);
}

static void print_file(fdk_fat32_fs_t *fs, fdk_fat32_entry_t *e, int row) {
    u8 buf[1024];
    u32 got = 0;
    if (fdk_fat32_read(fs, e, buf, sizeof(buf), &got) != FDK_OK) {
        fdk_vga_puts_at("read error", row, 0);
        return;
    }
    int col = 0;
    for (u32 i = 0; i < got && row < 22; i++) {
        char c = (char)buf[i];
        if (c == '\r') continue;
        if (c == '\n') { row++; col = 0; continue; }
        if (col >= FDK_VGA_COLS) { row++; col = 0; }
        if (row >= 22) break;
        fdk_vga_putc_at(c, row, col++);
    }
}

void kernel_main(void) {
    fdk_vga_init();
    fdk_vga_set_color(FDK_VGA_YELLOW, FDK_VGA_BLACK);
    fdk_vga_puts_at("Firmware Kit - ATA + FAT32 demo", 0, 0);
    fdk_vga_set_color(FDK_VGA_LIGHT_GREY, FDK_VGA_BLACK);

    fdk_gdt_init();
    fdk_idt_init();
    fdk_irq_install_handler(0, on_irq0);
    fdk_pit_init(100);
    fdk_kbd_init();

    fdk_vga_puts_at("GDT/IDT/PIT/KBD: OK", 1, 0);
    fdk_vga_puts_at("Ticks: ",            2, 0);
    fdk_vga_puts_at("PCI: ",              3, 0);
    fdk_vga_puts_at("ATA pri slave: ",    4, 0);
    fdk_vga_puts_at("FAT32: ",            5, 0);

    fdk_sti();

    struct enum_ctx ctx = { .row = 7, .total = 0 };
    fdk_pci_enumerate(enum_print, &ctx);
    putdec_at((u32)ctx.total, 3, 5);

    /* --- ATA: probe primary slave (QEMU's second -drive) --- */
    fdk_ata_dev_t disk;
    fdk_status as = fdk_ata_probe(0, 1, &disk);
    if (as == FDK_OK) {
        fdk_vga_puts_at(disk.model, 4, 15);
        putdec_at(disk.sectors / 2048, 4, 55);
        fdk_vga_puts_at("MB", 4, 62);
    } else {
        fdk_vga_puts_at("not found", 4, 15);
    }

    /* --- FAT32 --- */
    if (as == FDK_OK) {
        fdk_fat32_fs_t fs;
        if (fdk_fat32_mount(&fs, &disk, ata_read_cb) == FDK_OK) {
            fdk_vga_puts_at("mounted, root cluster=", 5, 7);
            putdec_at(fs.root_cluster, 5, 30);

            fdk_fat32_entry_t e;
            if (fdk_fat32_lookup(&fs, "README.TXT", &e) == FDK_OK) {
                fdk_vga_set_color(FDK_VGA_LIGHT_CYAN, FDK_VGA_BLACK);
                fdk_vga_puts_at("---- README.TXT ----", 13, 0);
                fdk_vga_set_color(FDK_VGA_LIGHT_GREY, FDK_VGA_BLACK);
                print_file(&fs, &e, 14);
            } else {
                fdk_vga_puts_at("README.TXT not found", 5, 40);
            }
        } else {
            fdk_vga_puts_at("mount failed (no FAT32?)", 5, 7);
        }
    }

    fdk_vga_puts_at("Type: ", 23, 0);
    int col = 0;
    for (;;) {
        fdk_vga_puthex_at(ticks, 2, 7);
        char c = fdk_kbd_poll();
        if (c) {
            if (c == '\b') { if (col > 0) fdk_vga_putc_at(' ', 23, --col + 6); }
            else if (c == '\n') { col = 0; }
            else if (col < 73) fdk_vga_putc_at(c, 23, 6 + col++);
        }
        fdk_hlt();
    }
}

__attribute__((section(".text.entry"), noinline))
void _start(void) {
    kernel_main();
    for (;;) __asm__ volatile ("cli; hlt");
}
