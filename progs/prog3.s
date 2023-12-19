lw $t0, 0x0000($s0)
addi $t0, $t0, 0x0002
add $s0, $t0, $t1
and $s0, $t0, $t1
slt $s0, $t0, $t1
sub $s0, $t0, $t1
nor $s0, $t0, $t1
or $s0, $t0, $t1
sw $s0, 0x0000($zero)