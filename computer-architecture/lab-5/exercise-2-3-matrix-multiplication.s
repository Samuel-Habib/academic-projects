.data
matrixA: .word 1, 2 # First row
.word 3, 4   # Second row
rowsA: .word 2
colsA: .word 2
matrixB: .word 5, 6 # First row
.word 7, 8   # Second row
rowsB: .word 2
colsB: .word 2
matrixC: .space 16      # 4 elements * 4 bytes each
newline: .string "\n"
space: .string " "
.text
.globl main
main:
la x5, rowsA
lw x20, 0(x5)        # x20 = rows of A
la x5, colsA
lw x21, 0(x5)        # x21 = cols of A
la x5, rowsB
lw x22, 0(x5)        # x22 = rows of B
la x5, colsB
lw x23, 0(x5)        # x23 = cols of B
li x24, 0       # i = 0 (row counter for A)
outer_loop:
bge x24, x20, done # if i >= rowsA, done
li x25, 0       # j = 0 (col counter for B)
middle_loop:
bge x25, x23, outer_next # if j >= colsB, next row
li x26, 0       # Sum for this element
li x27, 0       # k = 0 (element counter)
inner_loop:
bge x27, x21, middle_next # if k >= colsA, store result
# Calculate indices
# A[i][k] = matrixA + 4*(i*colsA + k)
mul x5, x24, x21      # i * colsA
add x5, x5, x27      #+k
slli x5, x5, 2     #*4
la x6, matrixA
add x6, x6, x5
lw x7, 0(x6)       # Load A[i][k]
# B[k][j] = matrixB + 4*(k*colsB + j)
mul x5, x27, x23      # k * colsB
add x5, x5, x25      #+j
slli x5, x5, 2     #*4
la x6, matrixB
add x6, x6, x5
lw x8, 0(x6)       # Load B[k][j]
# Multiply and add
mul x9, x7, x8
add x26, x26, x9      # sum += A[i][k] * B[k][j]
addi x27, x27, 1     # k++
beq zero, zero, inner_loop
middle_next:
# Store result in C[i][j]
mul x5, x24, x23      # i * colsC
add x5, x5, x25      #+j
slli x5, x5, 2     #*4
la x6, matrixC
add x6, x6, x5
sw x26, 0(x6)       # Store result
addi x25, x25, 1     # j++
beq zero, zero, middle_loop
outer_next:
addi x24, x24, 1     # i++
beq zero, zero, outer_loop
done:
li x24, 0         # Row counter
print_outer:
bge x24, x20, exit # if row >= rowsA, exit
li x25, 0         # Column counter
print_inner:
bge x25, x23, print_next # if col >= colsB, next row
mul x5, x24, x23
add x5, x5, x25
slli x5, x5, 2
la x6, matrixC
add x6, x6, x5
lw x11, 0(x6)
li x10, 1
ecall
li x10, 4
la x11, space
ecall
addi x25, x25, 1     # col++
beq zero, zero, print_inner
print_next:
li x10, 4
la x11, newline
ecall
addi x24, x24, 1     # row++
beq zero, zero, print_outer
exit:
li x10, 10
ecall
