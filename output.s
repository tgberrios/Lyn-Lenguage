.intel_syntax noprefix
.data
fmt: .asciz "Result: %ld\n"

arr: .quad 0
p: .quad 0
doble: .quad 0
resultado: .quad 0
z: .quad 0
y: .quad 0
x: .quad 0

.text
.global main
main:
    ; ---- Inicio Sentencia ----
    mov rax, 10    ; cargar literal numérico
    mov [x], rax ; inicializar varDecl
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    mov rax, 20    ; cargar literal numérico
    mov [y], rax ; inicializar varDecl
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    mov rax, [x]    ; cargar variable global
    push rax          ; stack = L
    mov rax, [y]    ; cargar variable global
    pop rbx           ; rbx = L, rax = R
    add rax, rbx    ; rax = L + R
    push rax          ; stack = L
    mov rax, 2    ; cargar literal numérico
    pop rbx           ; rbx = L, rax = R
    imul rax, rbx   ; rax = L * R
    mov [z], rax ; inicializar varDecl
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    mov rax, [z]    ; cargar variable global
    mov rsi, rax    ; param arg
    lea rdi, [rip+fmt] ; "Result: %ld\n"
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----

.global suma
suma:
    ; ---- Inicio Sentencia ----
    mov rax, [a]    ; cargar variable global
    push rax          ; stack = L
    mov rax, [b]    ; cargar variable global
    pop rbx           ; rbx = L, rax = R
    add rax, rbx    ; rax = L + R
    ret
    ; ---- Fin Sentencia ----

    ret    ; fin de funcion suma
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    mov rax, [x]    ; cargar variable global
    push rax    ; argumento 0
    mov rax, [y]    ; cargar variable global
    push rax    ; argumento 1
    call suma
    mov [resultado], rax ; inicializar varDecl
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Resultado: ]    ; cargar literal string
    push rax          ; stack = L
    mov rax, [resultado]    ; cargar variable global
    push rax    ; argumento 0
    call to_str
    pop rbx           ; rbx = L, rax = R
    add rax, rbx    ; rax = L + R
    mov rsi, rax    ; param arg
    lea rdi, [rip+fmt] ; "Result: %ld\n"
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; (lambda expresion) => STUB, no implementado
    mov [doble], rax    ; asignación
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Doble de 5: ]    ; cargar literal string
    push rax          ; stack = L
    mov rax, 5    ; cargar literal numérico
    push rax    ; argumento 0
    call doble
    push rax    ; argumento 0
    call to_str
    pop rbx           ; rbx = L, rax = R
    add rax, rbx    ; rax = L + R
    mov rsi, rax    ; param arg
    lea rdi, [rip+fmt] ; "Result: %ld\n"
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; (class Punto) => pendiente de implementación real
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    mov rax, 3    ; cargar literal numérico
    push rax    ; argumento 0
    mov rax, 4    ; cargar literal numérico
    push rax    ; argumento 1
    call Punto
    mov [p], rax ; inicializar varDecl
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Distancia de punto: ]    ; cargar literal string
    push rax          ; stack = L
    mov rax, [p]    ; cargar variable global
    push rax    ; argumento 0
    call distancia
    push rax    ; argumento 0
    call to_str
    pop rbx           ; rbx = L, rax = R
    add rax, rbx    ; rax = L + R
    mov rsi, rax    ; param arg
    lea rdi, [rip+fmt] ; "Result: %ld\n"
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    mov rax, [resultado]    ; cargar variable global
    push rax          ; stack = L
    mov rax, 25    ; cargar literal numérico
    pop rbx           ; rbx = L, rax = R
    cmp rbx, rax    ; compara L con R
    setg al         ; al=1 si L>R, 0 si no
    movzb rax, al   ; rax = 0/1
    cmp rax, 0
    je .ELSE_0
    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Resultado mayor a 25]    ; cargar literal string
    mov rsi, rax    ; param arg
    lea rdi, [rip+fmt] ; "Result: %ld\n"
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    jmp .ENDIF_1
.ELSE_0:
    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Resultado menor o igual a 25]    ; cargar literal string
    mov rsi, rax    ; param arg
    lea rdi, [rip+fmt] ; "Result: %ld\n"
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

.ENDIF_1:
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    mov rax, 0    ; cargar literal numérico
    mov [i], rax
.LOOP_2:
    mov rax, 3    ; cargar literal numérico
    mov rbx, rax    ; rbx = end
    mov rax, [i]   ; rax = i
    cmp rax, rbx
    jge .LOOPEND_3
    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Iteración ]    ; cargar literal string
    push rax          ; stack = L
    mov rax, [i]    ; cargar variable global
    push rax    ; argumento 0
    call to_str
    pop rbx           ; rbx = L, rax = R
    add rax, rbx    ; rax = L + R
    mov rsi, rax    ; param arg
    lea rdi, [rip+fmt] ; "Result: %ld\n"
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    mov rax, [i]
    add rax, 1
    mov [i], rax
    jmp .LOOP_2
.LOOPEND_3:
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; (import python numpy) => sin efecto real
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; (array literal) => no implementado (stub)
    mov [arr], rax ; inicializar varDecl
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Suma de arreglo: ]    ; cargar literal string
    push rax          ; stack = L
    mov rax, [arr]    ; cargar variable global
    push rax    ; argumento 0
    call suma_numpy
    push rax    ; argumento 0
    call to_str
    pop rbx           ; rbx = L, rax = R
    add rax, rbx    ; rax = L + R
    mov rsi, rax    ; param arg
    lea rdi, [rip+fmt] ; "Result: %ld\n"
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    mov rax, 60    ; exit
    xor rdi, rdi   ; status=0
    syscall
