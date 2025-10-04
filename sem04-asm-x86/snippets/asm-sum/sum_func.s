.intel_syntax noprefix
.global sum

sum:
    mov rax, 0
    add rax, rdi
    add rax, rsi
    ret
