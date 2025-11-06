    .text
    .global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov eax, 10
    mov DWORD PTR [rbp-4], eax
    mov eax, 20
    mov DWORD PTR [rbp-8], eax
    mov DWORD PTR [rbp-12], 0
    mov eax, DWORD PTR [rbp-4]
    push rax
    mov eax, DWORD PTR [rbp-8]
    push rax
    mov eax, 2
    mov ebx, eax
    pop rax
    imul eax, ebx
    mov ebx, eax
    pop rax
    add eax, ebx
    mov DWORD PTR [rbp-12], eax
    mov eax, DWORD PTR [rbp-12]
    push rax
    mov eax, 40
    mov ebx, eax
    pop rax
    cmp eax, ebx
    setg al
    movzx eax, al
    cmp eax, 0
    je .L0
    mov eax, DWORD PTR [rbp-12]
    push rax
    mov eax, 10
    mov ebx, eax
    pop rax
    sub eax, ebx
    mov DWORD PTR [rbp-12], eax
    jmp .L1
.L0:
    mov eax, DWORD PTR [rbp-12]
    push rax
    mov eax, 5
    mov ebx, eax
    pop rax
    add eax, ebx
    mov DWORD PTR [rbp-12], eax
.L1:
.L2:
    mov eax, DWORD PTR [rbp-4]
    push rax
    mov eax, 20
    mov ebx, eax
    pop rax
    cmp eax, ebx
    setl al
    movzx eax, al
    cmp eax, 0
    je .L3
    mov eax, DWORD PTR [rbp-4]
    push rax
    mov eax, 1
    mov ebx, eax
    pop rax
    add eax, ebx
    mov DWORD PTR [rbp-4], eax
    jmp .L2
.L3:
    mov eax, DWORD PTR [rbp-12]
    mov ebx, eax
    mov eax, ebx
    jmp .LRETURN
    mov eax, 0
    add rsp, 16
    pop rbp
    ret

