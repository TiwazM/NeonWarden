; One blue force-field heart; normal hearts use the cached pink HUD.
draw_field:
 lda armour
 beq field_return
 clc
 adc #18
 tay
 lda health
 pha
 sty health
 lda #3
 sta heart_index
 lda #4
 sta heart_limit
 lda #10
 sta heart_color
 lda #9
 sta heart_empty
 jsr heart_next
 pla
 sta health
field_return:
 rts

; Charge absorbs all damage first; remainder reaches red health.
absorb_field:
 lda armour
 sec
 sbc temp2
 bcc field_broken
 sta armour
 lda #0
 sta temp2
 rts
field_broken:
 eor #255
 ina
 sta temp2
 lda #0
 sta armour
 rts

; Back off after a blocked pursuit instead of repeatedly running into a wall.
pursuit_ready:
 lda difficulty
 beq pursuit_ready_done
 lda $60f0,x
 beq pursuit_ready_yes
 dec $60f0,x
 lda #0
 rts
pursuit_ready_yes:
 lda #1
pursuit_ready_done:
 rts
pursue_step:
 jsr patrol_enemy
 lda ex,x
 cmp patrolx
 beq pursuit_ready_done
 lda #24
 sta $60f0,x
 rts

pit_damage:
 lda difficulty
 bne pit_lethal
 lda #3
 jmp hurt_player
pit_lethal:
 jmp player_dead
