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
    i32.const 10
    mov [x], rax ; inicializar varDecl
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    i32.const 20
    mov [y], rax ; inicializar varDecl
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    global.get $x
    push rax          ; stack = L
    global.get $y
    pop rbx           ; rbx = L, rax = R
    i32.add
    push rax          ; stack = L
    i32.const 2
    pop rbx           ; rbx = L, rax = R
    i32.mul
    mov [z], rax ; inicializar varDecl
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    global.get $z
    mov rsi, rax
    lea rdi, [rip+fmt]
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----

.global suma
suma:
    ; ---- Inicio Sentencia ----
    global.get $a
    push rax          ; stack = L
    global.get $b
    pop rbx           ; rbx = L, rax = R
    i32.add
    ret
    ; ---- Fin Sentencia ----

    ret    ; fin de función suma
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    global.get $x
    push rax    ; argumento 0
    global.get $y
    push rax    ; argumento 1
    call suma
    mov [resultado], rax ; inicializar varDecl
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Resultado: ]    ; cargar literal string
    push rax          ; stack = L
    global.get $resultado
    push rax    ; argumento 0
    call to_str
    pop rbx           ; rbx = L, rax = R
    i32.add
    mov rsi, rax
    lea rdi, [rip+fmt]
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; (lambda expresion) => STUB, no implementado
    global.set $doble
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Doble de 5: ]    ; cargar literal string
    push rax          ; stack = L
    i32.const 5
    push rax    ; argumento 0
    call doble
    push rax    ; argumento 0
    call to_str
    pop rbx           ; rbx = L, rax = R
    i32.add
    mov rsi, rax
    lea rdi, [rip+fmt]
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; (class Punto) => pendiente de implementación real
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    i32.const 3
    push rax    ; argumento 0
    i32.const 4
    push rax    ; argumento 1
    call Punto
    mov [p], rax ; inicializar varDecl
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Distancia de punto: ]    ; cargar literal string
    push rax          ; stack = L
    global.get $p
    push rax    ; argumento 0
    call distancia
    push rax    ; argumento 0
    call to_str
    pop rbx           ; rbx = L, rax = R
    i32.add
    mov rsi, rax
    lea rdi, [rip+fmt]
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    global.get $resultado
    push rax          ; stack = L
    i32.const 25
    pop rbx           ; rbx = L, rax = R
    i32.gt_s
    cmp rax, 0
    i32.eqz
    br_if .ELSE_16
    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Resultado mayor a 25]    ; cargar literal string
    mov rsi, rax
    lea rdi, [rip+fmt]
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    br .ENDIF_17
.ELSE_16:
    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Resultado menor o igual a 25]    ; cargar literal string
    mov rsi, rax
    lea rdi, [rip+fmt]
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

.ENDIF_17:
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    i32.const 0
    mov [i], rax
.LOOP_18:
    i32.const 3
    mov rbx, rax    ; rbx = end
    mov rax, [i]   ; rax = i
    cmp rax, rbx
    i32.eqz
    br_if .LOOPEND_19
    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Iteración ]    ; cargar literal string
    push rax          ; stack = L
    global.get $i
    push rax    ; argumento 0
    call to_str
    pop rbx           ; rbx = L, rax = R
    i32.add
    mov rsi, rax
    lea rdi, [rip+fmt]
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    mov rax, [i]
    add rax, 1
    mov [i], rax
    br .LOOP_18
.LOOPEND_19:
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; (import python numpy) => sin efecto real
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    ; (array literal) => pendiente de implementación
    mov [arr], rax ; inicializar varDecl
    ; ---- Fin Sentencia ----

    ; ---- Inicio Sentencia ----
    lea rax, [rip+str_Suma de arreglo: ]    ; cargar literal string
    push rax          ; stack = L
    global.get $arr
    push rax    ; argumento 0
    call suma_numpy
    push rax    ; argumento 0
    call to_str
    pop rbx           ; rbx = L, rax = R
    i32.add
    mov rsi, rax
    lea rdi, [rip+fmt]
    xor eax, eax
    call printf
    ; ---- Fin Sentencia ----

    mov rax, 60    ; exit
    xor rdi, rdi   ; status=0
    syscall
