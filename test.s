.intel_syntax noprefix
.global main

main:
    push rbp
    mov rbp, rsp
    sub rsp, 64
entry:
    mov DWORD PTR [rbp-4], 10
    mov eax, DWORD PTR [rbp-4]
    mov DWORD PTR [rbp-8], eax
    mov eax, DWORD PTR [rbp-8]
    add eax, 5
    mov DWORD PTR [rbp-12], eax
    mov eax, DWORD PTR [rbp-12]
    mov rsp, rbp
    pop rbp
    ret
