# Getting Started

## Prerequisites

- **BIOS modules:** NASM, GCC (cross-capable, e.g. `x86_64-w64-mingw32-gcc -m32`),
  GNU Make, QEMU.
- **UEFI modules (later):** EDK II (TianoCore) or OpenCorePkg, Python 3,
  QEMU + OVMF.

On Windows the recommended environment is **MSYS2 UCRT64**:
Repository layout

    include/fdk/ — public headers.

    lib/ — shared libraries (IDT/GDT, PCI, port I/O).

    drivers/bios/ — drivers for legacy BIOS.

    drivers/uefi/ — DXE drivers for UEFI.

    examples/ — minimal runnable demos.

    build/ — build scripts (Makefiles).

Quick start
BIOS example
bash

cd examples/bios-hello
make
qemu-system-i386 -drive format=raw,file=hello.img

UEFI example (later)
bash

cd examples/uefi-hello
make
qemu-system-x86_64 -bios /usr/share/OVMF/OVMF.fd \
    -drive format=raw,file=fat:rw:esp

Toolchain notes

    BIOS code uses 32-bit freestanding C (-m32 -ffreestanding -nostdlib) plus NASM.

    UEFI code will use the EDK II build system (or an alternative cross toolchain).

    MSYS2 UCRT64 paths are POSIX-like; QEMU from the same prefix works out of the box.
