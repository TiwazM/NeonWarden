; Horizontal ferry platforms, used only by district 6 (internal stage 6).
; Original districts keep their existing terrain, enemies and pit lasers.
ferry_phase: .byte 0
ferry_ride: .byte 255
ferry_index: .byte 0
ferry_xlo: .byte 0,0,0,0
ferry_xhi: .byte 0,0,0,0
ferry_prev: .byte 0,0,0,0
ferry_delta: .byte 0
ferry_sign: .byte 0
ferry_base_lo: .byte <328,<584,<680,<776
ferry_base_hi: .byte >328,>584,>680,>776
ferry_setup:
 lda stage
 cmp #6
 bne ferry_done
 lda #0
 sta ferry_phase
 lda #255
 sta ferry_ride
 jsr ferry_positions
 ; Ground guards only on the safe approach, central pier and final landing.
 ldx #7
 lda #0
ferry_clear_enemies:
 sta ehp,x
 dex
 bpl ferry_clear_enemies
 ldx #3
ferry_spawn:
 ldy ferry_slots,x
 lda ferry_enemylo,x
 sta ex,y
 sta home,y
 lda ferry_enemyhi,x
 sta exhi,y
 sta homehi,y
 lda #0
 sta etype,y
 lda #136
 sta ey,y
 lda #8
 sta ehp,y
 dex
 bpl ferry_spawn
 lda #2
 sta etype+7
 lda #12
 sta ehp+7
 lda #132
 sta ey+7
 lda #6
 sta ehp+6
 jsr ferry_guard
ferry_done:
 rts
ferry_slots: .byte 0,3,6,7
ferry_enemylo: .byte <180,<592,<888,<928
ferry_enemyhi: .byte >180,>592,>888,>928
ferry_positions:
 ldx #0
ferry_position_loop:
 lda ferry_xlo,x
 sta ferry_prev,x
 lda ferry_phase
 cpx #0
 beq ferry_phase_ready
 clc
 adc #100
 bcc ferry_phase_no_carry
 ; phase+100 is at most 299, modulo 200.
 adc #55
 bra ferry_phase_ready
ferry_phase_no_carry:
 cmp #200
 bcc ferry_phase_ready
 sbc #200
ferry_phase_ready:
 tay
 lda ferry_offsets,y
 cpx #0
 beq ferry_offset_ready
 lda ferry_short_offsets,y
ferry_offset_ready:
 clc
 adc ferry_base_lo,x
 sta ferry_xlo,x
 lda ferry_base_hi,x
 adc #0
 sta ferry_xhi,x
 inx
 cpx #4
 bne ferry_position_loop
 rts
ferry_tick:
 lda stage
 cmp #6
 bne ferry_done
 inc ferry_phase
 lda ferry_phase
 cmp #200
 bcc ferry_clock_ready
 lda #0
 sta ferry_phase
ferry_clock_ready:
 jsr ferry_positions
 jsr ferry_guard
 lda grounded
 beq ferry_done
 lda playery
 cmp #120
 bne ferry_done
 ldx ferry_ride
 cpx #4
 bcs ferry_done
 sec
 lda ferry_xlo,x
 sbc ferry_prev,x
 sta ferry_delta
 lda #0
 bit ferry_delta
 bpl ferry_positive
 lda #255
ferry_positive:
 sta ferry_sign
 clc
 lda playerx
 adc ferry_delta
 sta playerx
 lda playerx+1
 adc ferry_sign
 sta playerx+1
 rts
ferry_landing:
 lda stage
 cmp #6
 bne ferry_done
 lda #255
 sta ferry_ride
 lda oldy
 clc
 adc #24
 cmp #145
 bcs ferry_done
 lda playery
 clc
 adc #24
 cmp #144
 bcc ferry_done
 ldx #3
ferry_overlap:
 clc
 lda playerx
 adc #8
 sta temp
 lda playerx+1
 adc #0
 sta temp2
 sec
 lda temp
 sbc ferry_xlo,x
 sta temp
 lda temp2
 sbc ferry_xhi,x
 bne ferry_overlap_next
 lda temp
 cmp #64
 bcs ferry_overlap_next
 lda #144
 sta floor_y
 stx ferry_ride
 rts
ferry_overlap_next:
 dex
 bpl ferry_overlap
 rts
ferry_draw:
 lda stage
 cmp #6
 bne ferry_done
 lda #0
 sta ferry_index
ferry_draw_next:
 ldx ferry_index
 sec
 lda ferry_xlo,x
 sbc camera
 sta ox
 lda ferry_xhi,x
 sbc camera+1
 sta ox+1
 beq ferry_visible
 cmp #255
 bne ferry_draw_skip
 lda ox
 cmp #193
 bcc ferry_draw_skip
ferry_visible:
 lda #144
 sta oy
 lda #<ferry_sprite
 sta src
 lda #>ferry_sprite
 sta src+1
 ; Bounds already checked for this 64-pixel-wide platform.
 jsr sprite_visible
ferry_draw_skip:
 inc ferry_index
 lda ferry_index
 cmp #4
 bne ferry_draw_next
 rts
ferry_offsets:
 .for k := 0, k < 200, k += 1
 .if k < 16
 .byte 0
 .elsif k < 100
 .byte (k-15)*2
 .elsif k < 116
 .byte 168
 .else
 .byte (199-k)*2
 .endif
 .endfor
ferry_guard:
 lda ehp+6
 beq ferry_guard_dead
 clc
 lda ferry_xlo+2
 adc #40
 sta ex+6
 sta home+6
 lda ferry_xhi+2
 adc #0
 sta exhi+6
 sta homehi+6
 lda #120
 sta ey+6
 lda #1
 sta edir+6
 rts
ferry_guard_dead:
 lda ehit+6
 cmp #255
 bne ferry_guard_return
 ; Keep the defeated guard's credit on its moving deck.
 clc
 lda ferry_xlo+2
 adc #40
 sta ex+6
 lda ferry_xhi+2
 adc #0
 sta exhi+6
 lda #128
 sta ey+6
 lda #$80
 sta ecool+6
ferry_guard_return:
 rts
ferry_patrol:
 lda stage
 cmp #6
 bne ferry_patrol_regular
 cpx #6
 beq ferry_guard_return
ferry_patrol_regular:
 jmp pursue_enemy
ferry_short_offsets:
 .for k := 0, k < 200, k += 1
 .if k < 16
 .byte 0
 .elsif k < 100
 .byte ((k-15)*2)/3
 .elsif k < 116
 .byte 56
 .else
 .byte ((199-k)*2)/3
 .endif
 .endfor
ferry_sprite:
 .byte 64,8
 .for row := 0, row < 8, row += 1
 .for ferry_art_col := 0, ferry_art_col < 64, ferry_art_col += 1
 .if row == 0
 .byte 10
 .elsif row == 1
 .byte 6
 .elsif row == 7
 .byte 1
 .elsif ferry_art_col < 4 || ferry_art_col > 59
 .byte 18
 .elsif row == 4 && (ferry_art_col & 7) == 2
 .byte 19
 .else
 .byte 3
 .endif
 .endfor
 .endfor


; Palette red border warning, independent of the music/SFX mix.
countdown_border:
 lda #0
 sta $d020
 lda difficulty
 beq countdown_border_done
 lda state
 cmp #1
 bne countdown_border_done
 lda paused
 bne countdown_border_done
 lda time_left
 beq countdown_border_done
 cmp #11
 bcs countdown_border_done
 cmp #6
 bcc countdown_border_fast
 lda frame
 and #16
 bra countdown_border_phase
countdown_border_fast:
 lda frame
 and #8
countdown_border_phase:
 beq countdown_border_done
 lda #30
 sta $d020
countdown_border_done:
 rts

; Credit pling: short high partial, then a clean triangle bell body.
credit_sid_start:
 cpy #9
 bne credit_sid_other
 lda #$07
 sta $d413
 lda #$08
 sta $d414
 ldy #67
 jsr credit_sid_note
 lda #$11
 sta $d412
 rts
credit_sid_other:
 jmp effects_sid_regular
credit_sid_note:
 lda videoticks
 cmp #60
 beq credit_sid_ntsc
 lda freqlo_pal,y
 sta $d40e
 lda freqhi_pal,y
 sta $d40f
 rts
credit_sid_ntsc:
 lda freqlo_ntsc,y
 sta $d40e
 lda freqhi_ntsc,y
 sta $d40f
 rts
credit_decay_dispatch:
 lda fx_active
 cmp #9
 bne credit_decay_other
 lda fx_mode
 beq credit_decay_other
 lda soundtime
 beq credit_fx_return
 dec soundtime
 beq credit_decay_off
 lda soundtime
 cmp #11
 bne credit_fx_return
 ldy #60
 jmp credit_sid_note
credit_decay_off:
 jmp effects_off
credit_decay_other:
 jmp effects_decay_regular
credit_fm_start:
 cpy #9
 bne credit_fx_return
 ldx #$c0
 lda #$30
 jsr opl_write
 ldx #$43
 lda #12
 jsr opl_write
 ldx #$60
 lda #$f5
 jsr opl_write
 ldx #$63
 lda #$f7
 jsr opl_write
 ldx #$80
 lda #$08
 jsr opl_write
 ldx #$83
 lda #$08
 jmp opl_write
credit_fx_return:
 rts
