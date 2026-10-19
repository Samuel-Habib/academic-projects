.data
result: .word 0
.text
.globl main
main:
addi x10, x0, 3 # a = 3
addi x11, x0, 8 # b = 8
addi x12, x0, 13 # c = 13
addi sp, sp, -12 # allocate space for 3 integers
sw x10, 0(sp) # push A
sw x11, 4(sp) # push B
sw x12, 8(sp) # push C
lw x11, 4(sp) # load B from stack
lw x12, 8(sp) # load C from stack
add x10, x11, x12 # b+c and store in x10
addi sp, sp, 8 # deallocate B and C
lw x10, 0(sp) # load A from stack
mul x11, x10, x10 # (B+C)*A
addi sp, sp, -4 # allocate space for the result
sw x11, 0(sp) # push the result
lw x11, 0(sp) # load the result from stack
addi sp, sp, 4    # deallocate the result
li a0, 1
ecall
li a0, 10
ecall
