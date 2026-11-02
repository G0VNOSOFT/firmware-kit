Firmware Kit - FAT32 demo
=========================

This file lives on a FAT32 disk image and is read at boot.

If you can see this, the following all work:
  1. ATA PIO sector reads
  2. FAT32 boot sector parsing
  3. FAT cluster chain traversal
  4. Directory entry lookup (8.3)
  5. File data read

The whole fs stack is pluggable - swap ATA for USB or anything
else and the fs module keeps working.
