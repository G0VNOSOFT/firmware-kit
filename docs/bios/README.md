BIOS Environment

Target: legacy x86 BIOS, real mode and 32-bit protected mode.
Typical use cases

    Boot sectors and bootloaders.

    Option ROMs (OpROM).

    Firmware payloads (SeaBIOS / coreboot payloads).

Constraints

    No standard library. -ffreestanding -nostdlib -m32.

    Segment registers matter: DS/ES/SS setup first.

    A20 line must be enabled before accessing memory > 1 MiB.

    Real mode uses int 10h/int 13h/int 16h for basic services.

    Protected mode requires GDT + IDT set up before external interrupts.
