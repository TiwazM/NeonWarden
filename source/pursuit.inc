; 1991 guards pursue a nearby courier on the same level, respecting terrain.
pursue_enemy:
 jsr pursuit_ready
 beq pursue_walk
 lda distance
 cmp #112
 bcs pursue_walk
 lda ey,x
 sec
 sbc playery
 clc
 adc #8
 cmp #17
 bcs pursue_walk
 lda ex,x
 sta home,x
 lda exhi,x
 sta homehi,x
 lda playerx
 cmp ex,x
 lda playerx+1
 sbc exhi,x
 lda #0
 bcs pursue_direction
 ina
pursue_direction:
 sta edir,x
 jmp pursue_step
pursue_walk:
 jmp patrol_enemy
