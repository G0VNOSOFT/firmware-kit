#include "fdk/kbd.h"
#include "fdk/idt.h"
#include "fdk/io.h"

#define KBD_DATA   0x60
#define KBD_STATUS 0x64
#define KBD_BUF_SZ 64

static char              buf[KBD_BUF_SZ];
static volatile int      head = 0;
static volatile int      tail = 0;

static volatile int      shift_l = 0, shift_r = 0;
static volatile int      ctrl = 0, alt = 0, caps = 0;

/* Scan code set 1, lower row (no shift). */
static const char map_lo[128] = {
    0,   27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,   'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,   '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0,   ' ',
};

/* Shifted row. */
static const char map_hi[128] = {
    0,   27, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,   'A','S','D','F','G','H','J','K','L',':','"','~',
    0,   '|','Z','X','C','V','B','N','M','<','>','?', 0,
    '*', 0,   ' ',
};

static void push(char c) {
    int next = (head + 1) % KBD_BUF_SZ;
    if (next == tail) return;       /* buffer full — drop */
    buf[head] = c;
    head = next;
}

static char pop(void) {
    if (head == tail) return 0;
    char c = buf[tail];
    tail = (tail + 1) % KBD_BUF_SZ;
    return c;
}

static int is_letter(u8 code) {
    return (code >= 0x10 && code <= 0x19) ||
           (code >= 0x1E && code <= 0x26) ||
           (code >= 0x2C && code <= 0x32);
}

static void on_irq1(u32 irq, u32 *frame) {
    (void)irq; (void)frame;
    u8 sc = fdk_inb(KBD_DATA);
    int release = (sc & 0x80) != 0;
    u8 code = sc & 0x7F;

    switch (code) {
        case 0x2A: shift_l = !release; return;
        case 0x36: shift_r = !release; return;
        case 0x1D: ctrl    = !release; return;
        case 0x38: alt     = !release; return;
        case 0x3A: if (!release) caps = !caps; return;
    }

    if (release || code >= 128) return;

    int shift = (shift_l || shift_r);
    char c;
    if (is_letter(code)) c = (shift ^ caps) ? map_hi[code] : map_lo[code];
    else                 c = shift         ? map_hi[code] : map_lo[code];

    if (c) push(c);
}

void fdk_kbd_init(void) {
    head = tail = 0;
    shift_l = shift_r = ctrl = alt = caps = 0;

    /* Drain any pending bytes from BIOS/shell before we take over. */
    while (fdk_inb(KBD_STATUS) & 0x01) (void)fdk_inb(KBD_DATA);

    fdk_irq_install_handler(1, on_irq1);
}

char fdk_kbd_poll(void) { return pop(); }

char fdk_kbd_getchar(void) {
    char c;
    while ((c = pop()) == 0) {
        __asm__ volatile ("sti; hlt");
    }
    return c;
}

int fdk_kbd_shift(void)    { return shift_l || shift_r; }
int fdk_kbd_ctrl(void)     { return ctrl; }
int fdk_kbd_alt(void)      { return alt; }
int fdk_kbd_capslock(void) { return caps; }
