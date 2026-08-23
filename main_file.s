.intel_syntax noprefix
.global main

main:
    push rbp
    mov rbp, rsp
    sub rsp, 64
entry:
    mov eax, 42
    mov rsp, rbp
    pop rbp
    ret
