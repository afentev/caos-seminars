.intel_syntax noprefix
.global fib

fib:
    push r12
    push r13

    mov rax, 0
    cmp rdi, 2 
    jl .return
    je .return1

    dec rdi
    mov r12, rdi

    call fib 
    mov r13, rax 
    
    mov rdi, r12
    dec rdi
    call fib 
    add rax, r13

.return:
    pop r13
    pop r12
    ret 

.return1:
    pop r13
    pop r12
    mov rax, 1
    ret
