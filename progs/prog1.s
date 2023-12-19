add $t0, $t0, $t0
add $zero, $zero, $zero
sub $t0, $t1, $t2
beq $t0, $t1, 0x0003
lw $t0, 0x0000($s0)
addi $t2, $t2, 0x0100
j 0x0000004