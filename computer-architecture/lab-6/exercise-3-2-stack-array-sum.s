addi sp, sp, -12        #store the array {1,2,3} in the stack
addi t0, x0,   1
sw   t0, 8(sp)
addi t0, x0,   2
sw   t0, 4(sp)
addi t0, x0,   3
sw   t0, 0(sp)
addi a1, x0,   3        #the size of the array
addi a2, x0,   0
j    loop
loop:                   #compute the sum of the array
bge a2, a1, end
lw   t0, 0(sp)
addi sp, sp,   4
add a0, a0, t0
addi a2, a2,   1
j    loop
end:
