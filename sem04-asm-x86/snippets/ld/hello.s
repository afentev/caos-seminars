.intel_syntax noprefix
.section .data
    hello: .asciz "Hello, world!\n"

.section .text
    .global _start

_start:
    lea rdi, [hello]
    call printf
    
    # Exit properly
    mov rax, 60                # sys_exit
    xor rdi, rdi               # exit status 0
    syscall


