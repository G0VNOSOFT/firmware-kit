bits 16
org 0x7C00

KERNEL_SEG  equ 0x1000          ; load at 0x10000
KERNEL_LBA  equ 1               ; start right after MBR
KERNEL_SECT equ 64              ; 32 KiB budget

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    mov [boot_drive], dl

    ; --- read kernel via LBA (int 13h AH=42h) ---
    mov word [dap_count], KERNEL_SECT
    mov dword [dap_lba],   KERNEL_LBA
    mov si, dap
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13
    jc  .err

    ; --- enable A20 (fast gate) ---
    in  al, 0x92
    or  al, 2
    and al, 0xFE
    out 0x92, al

    ; --- enter protected mode ---
    cli
    lgdt [gdt_desc]
    mov eax, cr0
    or  eax, 1
    mov cr0, eax
    jmp 0x08:pm_start

.err:
    mov si, msg_err
.put:
    lodsb
    test al, al
    jz  .hang
    mov ah, 0x0E
    int 0x10
    jmp .put
.hang:
    cli
    hlt
    jmp .hang

bits 32
pm_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    jmp 0x08:0x10000

bits 16
boot_drive db 0
msg_err    db "Boot error", 0

align 8
dap:
    db 0x10                     ; size of DAP
    db 0
dap_count:
    dw 0                        ; sectors to read
    dw 0x0000                   ; offset
    dw KERNEL_SEG               ; segment
dap_lba:
    dq 0                        ; starting LBA

align 8
gdt:
    dq 0
    dw 0xFFFF, 0x0000
    db 0x00, 0x9A, 0xCF, 0x00
    dw 0xFFFF, 0x0000
    db 0x00, 0x92, 0xCF, 0x00
gdt_end:

gdt_desc:
    dw gdt_end - gdt - 1
    dd gdt

times 510-($-$$) db 0
dw 0xAA55
