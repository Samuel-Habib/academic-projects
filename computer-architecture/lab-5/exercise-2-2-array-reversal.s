.data
arr: .word 2, 6, 10, 14, 20, 16
newline: .string "\n"
space: .string " "
.text
.globl main
input:
li x10, 0x10000040       # destination address
la x11, arr          # source address
li x12, 6         # counter
input_loop:
beq x12, zero, input_done
lw x20, 0(x11)
sw x20, 0(x10)
addi x10, x10, 4
addi x11, x11, 4
addi x12, x12, -1
beq zero, zero, input_loop
input_done:
jalr x0, x1, 0
reorder:
mul x20, x11, x8       # x8 is 4
add x21, x10, x20       # End address
addi x21, x21, -4      # Last element address
reorder_loop:
bge x10, x21, reorder_done
lw x22, 0(x10)        # load first element
lw x23, 0(x21)        # load last element
sw x23, 0(x10)         # store last at first
sw x22, 0(x21)         # store first at last
addi x10, x10, 4       # move front pointer
addi x21, x21, -4      # move back pointer
beq zero, zero, reorder_loop
reorder_done:
jalr x0, x1, 0
output:
addi x20, x10, 0       # save start address
addi x21, x11, 0       # save size
output_loop:
beq x21, zero, output_done
lw x11, 0(x20)        # load number in a1 for print
li x10, 1         # print integer syscall
ecall
la x11, space         # print space
li x10, 4
ecall
addi x20, x20, 4
addi x21, x21, -1
beq zero, zero, output_loop
output_done:
la x11, newline
li x10, 4
ecall
jalr x0, x1, 0
main:
li x8, 4       # constant 4 for array element size
jal input
li x10, 0x10000040   # array address
li x11, 6      # array size
jal reorder
li x10, 0x10000040   # array address
li x11, 6      # array size
jal output
li x10, 10
ecall
