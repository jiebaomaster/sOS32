[global switch_to]

; 具体的线程切换操作，重点在于寄存器的保存与恢复
switch_to:
    mov eax, [esp+4]  ; [esp+4]为第一个参数，prev的上下文
    ; 保存prev上下文
    mov [eax+0], esp
    mov [eax+4], ebp
    mov [eax+8], ebx
    mov [eax+12], esi
    mov [eax+16], edi
    pushf             ; eflags 入栈
    pop ecx           ; eflags=>ecx
    mov [eax+20], ecx ; ecx=>context.eflags

    mov eax, [esp+8]  ; [esp+8]为第二个参数，next的上下文
    ; 恢复next上下文
    mov esp, [eax+0]
    mov ebp, [eax+4]
    mov ebx, [eax+8]
    mov esi, [eax+12]
    mov edi, [eax+16]
    mov eax, [eax+20] ; context.eflags=>eax
    push eax          ; eflags 入栈
    popf              ; 栈顶 => eflags
    ; 自动从栈中弹出下一条指令的地址，并跳转过去执行
    ret