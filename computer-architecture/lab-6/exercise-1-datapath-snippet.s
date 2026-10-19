lw x5, 0(x10)
lw x6, 4(x10)
add x7, x6,   x5
sw x7, 8(x10)
beq x7, x0, Lab6
or x7, x5,   x6
...
Lab6:
