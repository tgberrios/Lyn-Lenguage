.intel_syntax noprefix
.data
fmt: .asciz "Result: %ld\n"
.text
.global main
main:
    ; ---- Inicio Sentencia ----
    ; ERROR: Nodo tipo 2 no soportado en generación de código
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; ERROR: Nodo tipo 2 no soportado en generación de código
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; ERROR: Nodo tipo 2 no soportado en generación de código
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    mov rax, [z]    ; cargar valor de variable
    mov rsi, rax    ; prepara argumento para printf
    lea rdi, [rip+fmt]    ; formato de impresión
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; ERROR: Nodo tipo 2 no soportado en generación de código
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Resultado: ]    ; cargar literal de cadena
    push rax          ; guardar operando izquierdo
    ; ERROR: Nodo tipo 4 no soportado en generación de código
    pop rbx           ; recuperar operando izquierdo
    add rax, rbx    ; suma
    mov rsi, rax    ; prepara argumento para printf
    lea rdi, [rip+fmt]    ; formato de impresión
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; ERROR: Nodo tipo 7 no soportado en generación de código
    mov [doble], rax    ; asignación de variable
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Doble de 5: ]    ; cargar literal de cadena
    push rax          ; guardar operando izquierdo
    ; ERROR: Nodo tipo 4 no soportado en generación de código
    pop rbx           ; recuperar operando izquierdo
    add rax, rbx    ; suma
    mov rsi, rax    ; prepara argumento para printf
    lea rdi, [rip+fmt]    ; formato de impresión
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; ERROR: Nodo tipo 8 no soportado en generación de código
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; ERROR: Nodo tipo 2 no soportado en generación de código
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Distancia de punto: ]    ; cargar literal de cadena
    push rax          ; guardar operando izquierdo
    ; ERROR: Nodo tipo 4 no soportado en generación de código
    pop rbx           ; recuperar operando izquierdo
    add rax, rbx    ; suma
    mov rsi, rax    ; prepara argumento para printf
    lea rdi, [rip+fmt]    ; formato de impresión
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; ERROR: Nodo tipo 9 no soportado en generación de código
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; ERROR: Nodo tipo 10 no soportado en generación de código
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; ERROR: Nodo tipo 11 no soportado en generación de código
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; ERROR: Nodo tipo 2 no soportado en generación de código
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Suma de arreglo: ]    ; cargar literal de cadena
    push rax          ; guardar operando izquierdo
    ; ERROR: Nodo tipo 4 no soportado en generación de código
    pop rbx           ; recuperar operando izquierdo
    add rax, rbx    ; suma
    mov rsi, rax    ; prepara argumento para printf
    lea rdi, [rip+fmt]    ; formato de impresión
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    mov rax, 60    ; syscall: exit
    xor rdi, rdi   ; status 0
    syscall
