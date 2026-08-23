.intel_syntax noprefix
.global main

dummy:
    push rbp
    mov rbp, rsp
    sub rsp, 64
entry:
    mov eax, 0
    mov rsp, rbp
    pop rbp
    ret
