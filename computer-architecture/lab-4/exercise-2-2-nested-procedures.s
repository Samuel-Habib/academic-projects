.text
.globl main
main:
li x5, 2       # input x = 4
mv x10, x5        # copy x to argument register
jal exp        # call exp procedure
mv x11, x10        # move result to print register
li x10, 1      # print int syscall
ecall
li x10, 10         # exit syscall
ecall
exp:
addi sp, sp, -16      # adjust stack
sw ra, 12(sp)        # save return address
sw x5, 8(sp)         # save x
li x6, 4          # n = 4 for x^4
jal power           # calculate x^4
mv x7, x10           # save x^4 result
lw x5, 8(sp)        # restore x
li x6, 5          # m = 5 for 5x
jal times          # calculate 5x
mv x8, x10           # save 5x result
sub x10, x7, x8       # x^4 - 5x
addi x10, x10, 10      # add 10
lw ra, 12(sp)         # restore return address
addi sp, sp, 16        # restore stack
ret
power:                # calculates x^n
li x10, 1          # result = 1
power_loop:
beq x6, x0, skip_pow # if n == 0, done
mul x10, x10, x5        # result *= x
addi x6, x6, -1       # n--
j power_loop
skip_pow:
ret
times:               # calculates m*x
mul x10, x5, x6        # result = x * m
ret
