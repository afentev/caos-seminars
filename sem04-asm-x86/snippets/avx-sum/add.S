    .global very_important_function
    .intel_syntax noprefix
    .text

# (N=rdi, *A=rsi, *B=rdx, *R=rcx)
very_important_function:
    
    mov r8, rdi

    .Loop:
        # Вычитаем 8, потому что векторно за 1 итерацию обрабатываем 8 значений
        sub rdi, 8
        
        vmovaps ymm0, [rsi + rdi * 4] #  *4, так как sizeof(float) = 4
        vmovaps ymm1, [rdx + rdi * 4]
        vaddps ymm0, ymm0, ymm1
        vmovaps [rcx + rdi * 4], ymm0
        
        cmp rdi, 0
        jg .Loop

    ret
