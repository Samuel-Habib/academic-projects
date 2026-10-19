.data
x: .word 9         # initial value of x
y: .word 10         # initial value of y
msg1: .string "x = " # output message for x
msg2: .string " y = " # output message for y
.text
.globl main
main:
la x5, x        # load address of x
lw x6, 0(x5)       # load value of x
la x7, y        # load address of y
lw x8, 0(x7)       # load value of y
jal branch_1        # call branch_1
sw x6, 0(x5)       # store updated x
jal branch_2        # call branch_2
sw x8, 0(x7)       # store updated y
li x10, 4        # print string syscall
la x11, msg1
ecall
li x10, 1        # print int syscall
mv x11, x6
ecall
li x10, 4        # print string syscall
la x11, msg2
ecall
li x10, 1        # print int syscall
mv x11, x8
ecall
li x10, 10       # exit syscall
ecall
branch_1:
li x9, 20          # load 20 for comparison
bge x6, x9, skip1      # if x >= 20, skip
li x9, 5          # load 5 for multiplication
mul x6, x6, x9        #x=5*x
skip1:
ret              # return to main
branch_2:
bge x8, x0, skip2      # if y >= 0, skip
sub x8, x0, x8        # y = 0 - y (make positive)
skip2:
ret              # return to main
