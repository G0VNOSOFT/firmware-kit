#ifndef FDK_IO_H
#define FDK_IO_H

#include "types.h"

static inline void fdk_outb(u16 port, u8 val) {
    __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}
static inline u8 fdk_inb(u16 port) {
    u8 r;
    __asm__ volatile ("inb %1, %0" : "=a"(r) : "Nd"(port));
    return r;
}
static inline void fdk_outw(u16 port, u16 val) {
    __asm__ volatile ("outw %0, %1" :: "a"(val), "Nd"(port));
}
static inline u16 fdk_inw(u16 port) {
    u16 r;
    __asm__ volatile ("inw %1, %0" : "=a"(r) : "Nd"(port));
    return r;
}
static inline void fdk_outl(u16 port, u32 val) {
    __asm__ volatile ("outl %0, %1" :: "a"(val), "Nd"(port));
}
static inline u32 fdk_inl(u16 port) {
    u32 r;
    __asm__ volatile ("inl %1, %0" : "=a"(r) : "Nd"(port));
    return r;
}

static inline void fdk_io_wait(void) {
    __asm__ volatile ("outb %%al, $0x80" :: "a"(0));
}
static inline void fdk_cli(void) { __asm__ volatile ("cli" ::: "memory"); }
static inline void fdk_sti(void) { __asm__ volatile ("sti" ::: "memory"); }
static inline void fdk_hlt(void) { __asm__ volatile ("hlt"); }

#endif /* FDK_IO_H */
