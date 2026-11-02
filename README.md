# Firmware Kit

**A construction kit for BIOS and UEFI firmware development.**

Firmware Kit collects ready-to-use, well-documented low-level primitives and
drivers for **legacy BIOS** and **UEFI** environments. Every module is
standalone, freestanding-C friendly, and exposes a clean public API so that
other operating systems, bootloaders, and firmware payloads can integrate
without dragging in dependencies.

## Why

Frameworks like EDK II, SeaBIOS, and coreboot are excellent, but reusable,
isolated building blocks are still scarce. Firmware Kit fills that gap:
pick the module you need, drop it in, skip reinventing the wheel.

## Status

| Module                | BIOS | UEFI | What works today                              |
|-----------------------|:----:|:----:|-----------------------------------------------|
| GDT / IDT / IRQ       | yes  |  -   | Protected mode setup, PIC remap, IRQ dispatch |
| CPU exception handler | yes  |  -   | Vector, error code, EIP/CS/EFLAGS dump        |
| PIT timer             | yes  |  -   | Programmable frequency (100 Hz in the demo)   |
| VGA text mode         | yes  |  -   | 80x25, colors, scroll, positional + terminal  |
| PS/2 keyboard         | yes  |  -   | Scan set 1, shift / caps / ctrl / alt         |
| PCI enumeration       | yes  |  -   | 0xCF8/0xCFC, find_by_class/id, BAR size       |
| ATA PIO               | yes  |  -   | LBA28 read, primary/secondary x master/slave  |
| FAT32 (read-only)     | yes  |  -   | 8.3 names, root dir, cluster chain            |
| FAT32 LFN + subdirs   | plan |  -   | Planned                                       |
| NTFS (read-only)      | plan | plan | Planned                                       |
| USB UHCI / xHCI       | plan | plan | Planned                                       |
| NE2000 / e1000 + ARP  | plan | plan | Planned                                       |

Legend: **yes** = working, **plan** = planned, **-** = not planned.

## Design principles

- **No hidden runtime.** Primitives depend only on `<fdk/types.h>` and
  `<fdk/io.h>`. Freestanding C, no libc, no stack protector.
- **Pluggable dependencies.** Filesystems take a block-read callback, so
  FAT32 works over ATA, USB, or anything else you plug in.
- **Bare-metal first.** Every module runs in QEMU with a single `make run`.
- **Prefixed API.** Every public symbol starts with `fdk_` -- no collisions
  when you link Firmware Kit into your own OS or firmware.

## Repository layout

include/fdk/ public headers
lib/idt_gdt/ GDT, IDT, IRQ, CPU exceptions
lib/common/ shared helpers (VGA, memory, port I/O)
lib/pci/ PCI configuration-space access
lib/timer/ PIT timer
lib/kbd/ PS/2 keyboard
lib/ata/ ATA PIO block device
lib/fs/ filesystems (FAT32)
examples/ runnable QEMU demos
docs/ architecture notes, per-environment guides
## Quick start (Windows / MSYS2 UCRT64)
```bash
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-nasm mingw-w64-ucrt-x86_64-make \
    mingw-w64-ucrt-x86_64-qemu mingw-w64-ucrt-x86_64-binutils mtools

git clone git@github.com:G0VNOSOFT/firmware-kit.git
cd firmware-kit/examples/bios-hello
make run
The example boots in QEMU, enumerates PCI, reads README.TXT from a
generated FAT32 disk image, and accepts keyboard input.
Quick start (Linux)

Same packages via your distro. The Makefile is toolchain-agnostic: override
CC/LD/OBJCOPY on the command line if needed:
make CC=gcc-12 LD=ld.bfd OBJCOPY=objcopy run
License

MIT -- see LICENSE.
