.intel_syntax noprefix
.section .data
    hello: .asciz "Hello, world!\n"

.section .text
    .global _start

_start:
    # Write system call
    mov rax, 1          # syscall number for write (1)
    mov rdi, 1          # file descriptor stdout (1)
    lea rsi, [hello]    # pointer to string (use lea for address loading)
    mov rdx, 14         # string length
    syscall

    # Exit system call
    mov rax, 60         # syscall number for exit (60)
    mov rdi, 0          # exit status 0
    syscall
