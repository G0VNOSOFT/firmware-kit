# Firmware Kit

**Firmware Kit** is an open collection of ready-to-use low-level primitives
and drivers for **legacy BIOS** and **UEFI** environments. Not just
"another driver pack" — a construction kit for building firmware and bootloaders.

## 🎯 Motivation

Frameworks like EDK II, SeaBIOS and coreboot exist, but portable,
well-documented, drop-in "building blocks" — IDT/GDT, IRQ handling, port I/O,
filesystems, USB stack — are still scarce. **Firmware Kit** fills that gap:
take the module you need, embed it, skip reinventing the wheel.

## 📦 What's inside (roadmap)

### Low-level primitives (shared by BIOS & UEFI)
- GDT / IDT setup and management
- IRQ and exception handling (PIC, APIC)
- Port I/O (`inb` / `outb` / `inw` / `outw` / ...)
- Memory primitives (memset, memcpy, alignment)
- Timers (PIT, TSC)

### Drivers for **BIOS** (real / protected mode)
- FAT32 (read/write)
- USB UHCI
- Minimal network stack (NE2000 / RTL8139)

### Drivers for **UEFI** (DXE)
- FAT32 (wrapper over EDK II EnhancedFat + improvements)
- NTFS (port/adaptation of `ntfs-3g`)
- USB xHCI
- Network stack (wrapper over SNP)

## 🚀 Getting started
