difficulty_fire_delay:
 lda difficulty
 beq normal_fire_delay
 lda #32
 rts
normal_fire_delay:
 lda #48
 rts
difficulty_patrol:
 lda difficulty
 bne difficulty_patrol_done
 lda frame
 and #3
difficulty_patrol_done:
 rts
