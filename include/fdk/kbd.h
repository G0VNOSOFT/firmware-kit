#ifndef FDK_KBD_H
#define FDK_KBD_H

#include "types.h"

/*
 * PS/2 keyboard driver (scan code set 1).
 *
 * fdk_kbd_init() installs an IRQ1 handler and drains any pending bytes.
 * Characters are delivered via a small internal ring buffer, so the IRQ
 * handler stays fast and the caller can pull at any pace.
 *
 * Only depends on <fdk/types.h>, <fdk/io.h> and <fdk/idt.h>.
 */

void fdk_kbd_init(void);

/* Non-blocking. Returns 0 if no character is pending. */
char fdk_kbd_poll(void);

/* Blocking. Halts the CPU (sti; hlt) until a character arrives. */
char fdk_kbd_getchar(void);

/* Modifier state — useful for shortcuts in higher layers. */
int fdk_kbd_shift(void);
int fdk_kbd_ctrl(void);
int fdk_kbd_alt(void);
int fdk_kbd_capslock(void);

#endif /* FDK_KBD_H */
