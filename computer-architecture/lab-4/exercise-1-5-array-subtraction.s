.data
source: .word 2, 4, 6, 8, 10, 12, 0       # array values
.text
.globl main
main:
li x5, 0           #k=0
li x6, 100           # rem = 100
la x7, source          # load array address
loop:
slli x8, x5, 2       # k * 4 for offset
add x8, x8, x7          # get source[k] address
lw x9, 0(x8)          # load source[k] value
beq x9, x0, end_loop       # exit if source[k] = 0
sub x6, x6, x9          # rem = rem - source[k]
addi x5, x5, 1         # k++
bne x9, x0, loop       # loop if not zero
end_loop:
li x10, 1          # print syscall
mv x11, x6           # move rem to x11
ecall
li x10, 10         # exit syscall
ecall
