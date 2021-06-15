[GLOBAL gdt_flush]

gdt_flush:
    mov eax, [esp+4]    ; 参数存入 eax 寄存器
    lgdt [eax]          ; 设置GRUB

    mov ax, 0x10        ; 0x10 为数据段在GDT中的偏移地址
                        ; x86里不能把立即数直接往DS里送，得通过AX中转一下
    mov ds, ax          ; 更新所有可以更新的段寄存器
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush     ; 0x08 是代码段描述符在GDT中的偏移
                        ; 远跳目的是清空流水线并串行化处理器

.flush:
    ret