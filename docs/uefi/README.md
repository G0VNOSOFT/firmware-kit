UEFI Environment

Target: UEFI DXE drivers (x86_64 and IA32).
Typical use cases

    DXE drivers loaded by firmware or by a UEFI shell.

    Bootloaders and runtime services consumers.

Constraints

    Built against EDK II headers (or an equivalent SDK).

    Boot Services are available during DXE; Runtime Services persist.

    Protocol-based architecture: drivers bind to protocols (e.g. Block I/O,
    Simple Network Protocol).

    File layout of the resulting .efi binary must follow the PE/COFF format
    expected by the firmware.