.data
result: .word 0
.text
.globl main
main:
addi x10, x0, 9
addi x11, x0, 1          # result = 1
loop:
beq x10, x0, exit         # if x == 0, exit
mul x11, x11, x10          # result = result * x
addi x10, x10, -1         #x=x-1
j loop               # jump back to the start of the loop
exit:
addi sp, sp, -4         # allocate space on stack for the result
sw x11, 0(sp)            # store the result on the stack
lw x11, 0(sp)           # load the result back into x11
addi sp, sp, 4          # deallocate the stack space
li a0, 1
ecall
li a0, 10
ecall
