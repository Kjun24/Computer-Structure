        .data
data:   .word 4
data2:  .word 0x00000000
        .word 0x00000400
        .word 0x00000001
        .text
main:
        addiu $2, $0, 0x4
        addiu $1, $2, -3
        la $3, data
        addu $4, $3, $2
        la $5, data2
        lw $6, 4($4)
        srl $7, $6, 8
        sw $7, 0($5)
        lb $8, 0($5)
        sll $9, $7, 20
        bne $4, $5, check
        jal check

sub:
        addiu $11, $11, 5
        srl $11, $11, 1
        sb $11, 11($5)
        j check

check:
        addiu $10, $10, 1
        lb $11, 11($4)
        sltu $20, $11, $2
        sltu $21, $2, $11
        beq $20, $21, exit
        ori $9, $9, 0x30
        jr $31

exit:
