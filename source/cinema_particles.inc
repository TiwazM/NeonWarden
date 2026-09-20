cinema_particles:
 lda cine_time
 cmp #96
 bcc cinema_particles_done
 lda #0
 sta cine_particle
cinema_particle_loop:
 lda cine_particle
 asl
 asl
 asl
 clc
 adc frame
 and #63
 sta cine_distance
 lda #128
 sec
 sbc cine_distance
 tay
 lda cine_particle
 and #1
 beq cinema_particle_left
 lda #160
 clc
 adc cine_distance
 bra cinema_particle_x
cinema_particle_left:
 lda #160
 sec
 sbc cine_distance
cinema_particle_x:
 tax
 lda #11
 sta ink
 jsr plot
 inx
 jsr plot
 inc cine_particle
 lda cine_particle
 cmp #8
 bne cinema_particle_loop
cinema_particles_done:
 rts
