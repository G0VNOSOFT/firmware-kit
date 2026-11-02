# Firmware Kit — BIOS hello example
# Toolchain: MSYS2 UCRT64

CC      := x86_64-w64-mingw32-gcc
LD      := x86_64-w64-mingw32-ld
AS      := nasm
OBJCOPY := x86_64-w64-mingw32-objcopy
QEMU    := qemu-system-i386

ROOT    := ../..

CFLAGS  := -m32 -ffreestanding -nostdlib -fno-builtin -fno-pic \
           -fno-stack-protector -mno-stack-arg-probe \
           -fno-asynchronous-unwind-tables -fno-unwind-tables \
           -fno-ident -fno-common -I$(ROOT)/include
ASFLAGS := -f win32 --prefix _

LIBDIR  := $(ROOT)/lib/idt_gdt

CFILES  := kernel.c $(LIBDIR)/gdt.c $(LIBDIR)/idt.c $(LIBDIR)/irq.c \
           $(LIBDIR)/pit.c $(LIBDIR)/isr.c
AFILES  := $(LIBDIR)/gdt_flush.s $(LIBDIR)/idt_load.s $(LIBDIR)/isr_stubs.s

OBJS    := kernel.o gdt.o idt.o irq.o pit.o isr.o \
           gdt_flush.o idt_load.o isr_stubs.o

vpath %.c $(LIBDIR)
vpath %.s $(LIBDIR)

all: os.img

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

boot.bin: boot.s
	$(AS) -f bin $< -o $@

LDFLAGS := -m i386pe --image-base 0x0 -T link.ld

kernel.pe: $(OBJS) link.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

kernel.bin: kernel.pe
	$(OBJCOPY) -O binary \
	    -R .pdata -R .xdata -R .eh_frame -R .comment \
	    --set-section-flags .bss=alloc,load,contents \
	    $< $@

os.img: boot.bin kernel.bin
	cat boot.bin kernel.bin > $@

run: os.img
	$(QEMU) -drive format=raw,file=os.img

clean:
	rm -f *.o *.bin *.pe *.img

.PHONY: all run clean
