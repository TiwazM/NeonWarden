; Final room uses the first 256 pixels of its map, with no camera scrolling.
arena_setup:
 lda stage
 cmp #5
 bne arena_return
 ldx #6
 lda #0
arena_clear:
 sta ehp,x
 dex
 bpl arena_clear
 lda #176
 sta ex+7
 sta home+7
 lda #0
 sta exhi+7
 sta homehi+7
 sta message_timer
 lda #132
 sta ey+7
 lda #60
 sta ehp+7
arena_return:
 rts
arena_bounds:
 lda stage
 cmp #5
 bne arena_return
 lda playerx+1
 bne arena_right_bound
 lda playerx
 cmp #8
 bcs arena_right_bound
 lda #8
 sta playerx
arena_right_bound:
 lda playerx+1
 bne arena_set_right
 lda playerx
 cmp #233
 bcc arena_return
arena_set_right:
 lda #232
 sta playerx
 lda #0
 sta playerx+1
 rts
arena_camera:
 lda #0
 sta camera
 sta camera+1
 rts
arena_interact:
 lda keys
 and #16
 beq arena_camera
 lda prev
 and #16
 bne arena_camera
 lda playerx
 cmp #212
 bcc arena_camera
 lda relay
 beq arena_camera
 jmp win_game
; Telegraph, charge, a volley, then a recovery window (96 simulation ticks).
arena_boss:
 lda stage
 cmp #5
 bne arena_return
 lda ehp+7
 bne arena_alive
 ldx #7
 lda #0
arena_clear_shots:
 sta bulletlife,x
 dex
 bpl arena_clear_shots
 rts
arena_alive:
 inc bossphase
 lda bossphase
 cmp #160
 bcc arena_phase
 lda #0
 sta bossphase
arena_phase:
 lda bossphase
 cmp #96
 bcs arena_jump
 lda #9
 sta ecool+7
 lda bossphase
 cmp #24
 bcc arena_return
 bne arena_direction_done
 lda #0
 sta edir+7
 lda playerx
 cmp ex+7
 bcs arena_direction_done
 inc edir+7
arena_direction_done:
 lda bossphase
 cmp #48
 bcs arena_volley
 lda #40
 sta ecool+7
 lda edir+7
 bne arena_charge_left
 lda ex+7
 cmp #214
 bcs arena_return
 inc ex+7
 inc ex+7
 rts
arena_charge_left:
 lda ex+7
 cmp #26
 bcc arena_return
 dec ex+7
 dec ex+7
 rts
arena_volley:
 lda #40
 sta ecool+7
 lda bossphase
 cmp #48
 beq arena_fire
 cmp #56
 beq arena_fire
 cmp #64
 bne arena_return
arena_fire:
 ldx #0
 lda bossphase
 cmp #56
 bne arena_slot2
 inx
arena_slot2:
 cmp #64
 bne arena_shoot
 ldx #2
arena_shoot:
 lda ex+7
 sta ex,x
 lda #0
 sta exhi,x
 lda #132
 sta ey,x
 jsr fire_bullet
 lda #8
 jsr sfx
 rts

; 96..111 warning, 112..143 aimed leap, 144..159 landing recovery.
; Snapshot the player at takeoff: moving out of the landing zone is a counter.
arena_jump:
 lda #9
 sta ecool+7
 lda bossphase
 cmp #112
 bcc arena_jump_return
 bne arena_jump_flight
 lda playerx
 cmp #24
 bcs arena_target_right
 lda #24
arena_target_right:
 cmp #213
 bcc arena_target_set
 lda #212
arena_target_set:
 sta home+7
 lda #8
 jsr sfx
arena_jump_flight:
 lda #40
 sta ecool+7
 lda bossphase
 cmp #144
 bcs arena_landed
 sec
 sbc #112
 tay
 lda arena_jump_y,y
 sta ey+7
 ; Three one-pixel steps, stopping exactly at the locked target.
 ldy #3
arena_air_step:
 lda ex+7
 cmp home+7
 beq arena_jump_return
 bcc arena_air_right
 dec ex+7
 bra arena_air_next
arena_air_right:
 inc ex+7
arena_air_next:
 dey
 bne arena_air_step
arena_jump_return:
 rts
arena_landed:
 lda #132
 sta ey+7
 lda bossphase
 cmp #144
 bne arena_jump_return
 lda #3
 jsr sfx
 rts
arena_jump_y:
 .byte 122,112,103,95,87,80,74,68,63,59,55,52,50,48,47,46
 .byte 46,47,48,50,52,55,59,63,68,74,80,87,95,103,112,122
