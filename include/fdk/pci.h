#ifndef FDK_PCI_H
#define FDK_PCI_H

#include "types.h"

/*
 * PCI configuration-space access via the legacy 0xCF8/0xCFC mechanism.
 *
 * Supports:
 *   - raw 8/16/32-bit reads and 32-bit writes
 *   - device probing (vendor/device/class/header_type)
 *   - enumeration of all present functions (with multi-function awareness)
 *   - search by class/subclass or vendor/device
 *   - BAR reading and size detection (MMIO or I/O BARs)
 *
 * Only depends on <fdk/types.h> and <fdk/io.h>.
 */

#define FDK_PCI_ADDR_PORT 0xCF8
#define FDK_PCI_DATA_PORT 0xCFC

typedef struct {
    u8  bus;
    u8  slot;          /* device number 0..31 */
    u8  func;          /* function 0..7 */
    u16 vendor;
    u16 device;
    u8  revision;
    u8  prog_if;
    u8  subclass;
    u8  class_code;
    u8  header_type;
} fdk_pci_dev_t;

/* ---- raw config-space access ---- */
u32  fdk_pci_read32 (u8 bus, u8 slot, u8 func, u8 offset);
u16  fdk_pci_read16 (u8 bus, u8 slot, u8 func, u8 offset);
u8   fdk_pci_read8  (u8 bus, u8 slot, u8 func, u8 offset);
void fdk_pci_write32(u8 bus, u8 slot, u8 func, u8 offset, u32 val);

/* ---- device probing ---- */
fdk_status fdk_pci_get_device(u8 bus, u8 slot, u8 func, fdk_pci_dev_t *out);

/* ---- enumeration ----
 * Callback returns 0 to continue, nonzero to stop.
 */
typedef int (*fdk_pci_enum_cb)(const fdk_pci_dev_t *dev, void *user);
void fdk_pci_enumerate(fdk_pci_enum_cb cb, void *user);

/* ---- search ---- */
fdk_status fdk_pci_find_by_class(u8 class_code, u8 subclass, fdk_pci_dev_t *out);
fdk_status fdk_pci_find_by_id   (u16 vendor, u16 device,      fdk_pci_dev_t *out);

/* ---- BAR helpers (only for header_type 0x00 / 0x80) ---- */
u32 fdk_pci_read_bar (const fdk_pci_dev_t *dev, int bar_index); /* 0..5 */
u32 fdk_pci_bar_size (const fdk_pci_dev_t *dev, int bar_index);

/* ---- human-readable class name (e.g. "storage", "display") ---- */
const char *fdk_pci_class_name(u8 class_code);

#endif /* FDK_PCI_H */
