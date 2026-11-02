bits 32
section .text
global fdk_idt_load
fdk_idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret
