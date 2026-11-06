.data
A:          .word 495, 10, 864, 22, 0, 24
datasize:   .word 6
threshold:  .word 40
num_studs:  .word 4

msg_data:    .asciiz "data[] = {"
msg_comma:   .asciiz ", "
msg_end_arr: .asciiz "}\n"
msg_count:   .asciiz "count_suspicious (threshold=40) = "
msg_newline: .asciiz "\n"
msg_bonus:   .asciiz "Bonus: Cheaters IDs: "
msg_and:     .asciiz " and "

.text
.globl main

main:
    addi $sp, $sp, -4
    sw $ra, 0($sp)

    la $t0, A               # Base Address
    lw $t1, datasize        # n
    li $t2, 0               # i = 0

decrypt_loop:
    beq $t2, $t1, part2_prep
    
    sll $t3, $t2, 2         # i * 4
    add $t4, $t0, $t3       # address of A[i]
    lw $t5, 0($t4)          # t5 = data[i]

    andi $t6, $t2, 1        # if(i%=2)
    beq $t6, $zero, even_pos

odd_pos:
    # data[i] = data[i] * 3 -> (data[i] << 1) + data[i]
    sll $t7, $t5, 1
    add $t5, $t7, $t5
    sw $t5, 0($t4)
    j next_iter

even_pos:
    li $t7, 0         # Quotient
    move $t8, $t5     # Remainder

div_loop:
    blt $t8, 9, div_done
    addi $t8, $t8, -9
    addi $t7, $t7, 1
    j div_loop

div_done:
    sw $t7, 0($t4)

next_iter:
    addi $t2, $t2, 1
    j decrypt_loop

part2_prep:
    li $v0, 4
    la $a0, msg_data
    syscall
    li $t2, 0

print_loop:
    lw $t1, datasize
    beq $t2, $t1, end_print
    sll $t3, $t2, 2
    la $t0, A
    add $t4, $t0, $t3
    lw $a0, 0($t4)
    li $v0, 1
    syscall
    addi $t2, $t2, 1
    beq $t2, $t1, end_print
    li $v0, 4
    la $a0, msg_comma
    syscall
    j print_loop

end_print:
    li $v0, 4
    la $a0, msg_end_arr
    syscall

    la $a0, A               # data*
    lw $a1, datasize        # n
    lw $a2, threshold       # threshold
    jal count_suspicious
    move $s0, $v1      

    li $v0, 4
    la $a0, msg_count
    syscall
    move $a0, $s0
    li $v0, 1
    syscall
    li $v0, 4
    la $a0, msg_newline
    syscall
    la $t0, A
    lw $t1, datasize
    li $t2, 0               # index

bonus_loop:
    beq $t2, $t1, exit_prog
    sll $t3, $t2, 2
    add $t4, $t0, $t3
    lw $t5, 0($t4)
    bge $t5, 80, found_cheaters
    addi $t2, $t2, 1
    j bonus_loop

found_cheaters:
    li $t6, 2

row_calc:
    li $t7, 1

col_calc:
    addi $t8, $t6, -1       # r-1
    addi $t9, $t6, -2       # r-2
    mul $t8, $t8, $t9       # (r-1)(r-2)
    sra $t8, $t8, 1         # /2
    addi $t9, $t7, -1       # c-1
    add $t8, $t8, $t9       # Total index
    
    beq $t8, $t2, print_bonus
    
    addi $t7, $t7, 1
    blt $t7, $t6, col_calc
    addi $t6, $t6, 1
    j row_calc

print_bonus:
    move $s6, $t7           # Student 1 ID
    move $s7, $t6           # Student 2 ID
    
    li $v0, 4
    la $a0, msg_bonus
    syscall
    move $a0, $s6
    li $v0, 1
    syscall
    li $v0, 4
    la $a0, msg_and
    syscall
    move $a0, $s7
    li $v0, 1
    syscall
    li $v0, 4
    la $a0, msg_newline
    syscall

exit_prog:
    lw $ra, 0($sp)
    addi $sp, $sp, 4
    li $v0, 10              # Exit
    syscall

count_suspicious:
    # Stack Frame
    addi $sp, $sp, -12
    sw $ra, 8($sp)
    sw $a1, 4($sp)          # save n
 
    bne $a1, $zero, recurse
    li $v1, 0
    addi $sp, $sp, 12
    jr $ra

recurse:
    addi $a1, $a1, -1       # n - 1
    jal count_suspicious    # count_suspicious(data, n-1, threshold)

    lw $a1, 4($sp)
    sll $t0, $a1, 2
    addi $t0, $t0, -4       # (n-1)*4
    add $t0, $a0, $t0       # address of data[n-1]
    lw $t1, 0($t0)          # data[n-1] value
    
    blt $t1, $a2, skip_inc  # if (data[n-1] >= threshold)
    addi $v1, $v1, 1        # count + 1

skip_inc:
    lw $ra, 8($sp)
    addi $sp, $sp, 12
    jr $ra




