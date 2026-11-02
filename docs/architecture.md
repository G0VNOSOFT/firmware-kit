Architecture

Firmware Kit is split into three layers:

    Primitives (include/fdk/, lib/)
    Environment-agnostic low-level code: types, port I/O, memory helpers,
    GDT/IDT/IRQ, timers. Designed to compile for both BIOS (32-bit) and UEFI.

    BIOS drivers (drivers/bios/)
    Target real mode / 32-bit protected mode. No reliance on any OS.

    UEFI drivers (drivers/uefi/)
    DXE drivers. Built against EDK II (or compatible) headers.

Design principles

    No hidden runtime. No libc dependency in primitives; all helpers are explicit.

    Freestanding C. Compiles with -ffreestanding -nostdlib -fno-builtin.

    Portable types. u8/u16/u32/u64 everywhere; fdk_status for results.

    Small, composable modules. One feature per directory, clear public header.

    QEMU-first testing. Every module must be runnable via QEMU.

Directory roles

    include/fdk/ — public API. Stable, documented.

    lib/ — implementations of primitives.

    drivers/ — target-specific code, depends on lib/ only.

    examples/ — minimal runnable consumers of the API.
