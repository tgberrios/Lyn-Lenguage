.intel_syntax noprefix
.data
fmt: .asciz "Result: %ld\n"
arr: .quad 0
p: .quad 0
doble: .quad 0
resultado: .quad 0
z: .quad 0
y: .quad 20
x: .quad 10
.text
.global main
main:
    mov rax, [z]
    mov rsi, rax
    lea rdi, [rip+fmt]
    xor eax, eax
    call printf
    lea rax, [rip+str_Resultado: ]
    push rax
    ; ERROR: Node type 4 not supported in codegen
    pop rbx
    add rax, rbx
    mov rsi, rax
    lea rdi, [rip+fmt]
    xor eax, eax
    call printf
    lea rax, [rip+str_Doble de 5: ]
    push rax
    ; ERROR: Node type 4 not supported in codegen
    pop rbx
    add rax, rbx
    mov rsi, rax
    lea rdi, [rip+fmt]
    xor eax, eax
    call printf
    ; ERROR: Node type 8 not supported in codegen
    ; ERROR: Node type 9 not supported in codegen
    ; ERROR: Node type 10 not supported in codegen
    ; ERROR: Node type 11 not supported in codegen
    lea rax, [rip+str_Suma numpy: ]
    push rax
    ; ERROR: Node type 4 not supported in codegen
    pop rbx
    add rax, rbx
    mov rsi, rax
    lea rdi, [rip+fmt]
    xor eax, eax
    call printf
    mov rax, 60
    xor rdi, rdi
    syscall
