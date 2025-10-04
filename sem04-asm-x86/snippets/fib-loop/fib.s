.intel_syntax noprefix
.global fib

fib:
    mov rax, 0
    mov r8, 1

.loop:
    cmp rdi, 0
    jle .exit
    dec rdi

    mov r9, rax 
    mov rax, r8 
    add r8, r9

    jmp .loop

.exit:
    ret 

