; Two visible emitters per late district; 25-Hz clock freezes with gameplay.
; 88 safe ticks, 24 charge ticks, 16 beam ticks, staggered by 64 ticks.
laser_clock: .byte 0
laser_index: .byte 0
laser_phase: .byte 0
laser_low: .byte <500,<852,<516,<820
laser_high: .byte >500,>852,>516,>820
laser_begin:
 lda stage
 cmp #3
 bcc laser_no_stage
 cmp #5
 bcs laser_no_stage
 sec
 sbc #3
 asl
 sta laser_index
 sec
 rts
laser_no_stage:
 clc
 rts
laser_get_phase:
 lda laser_index
 and #1
 asl
 asl
 asl
 asl
 asl
 asl
 clc
 adc laser_clock
 and #127
 sta laser_phase
 ldx laser_index
 rts
laser_tick:
 lda state
 cmp #1
 bne laser_return
 jsr laser_begin
 bcc laser_return
 inc laser_clock
laser_hit_loop:
 jsr laser_get_phase
 cmp #112
 bcc laser_hit_next
 ; Beam spans y=40..175. Ignore players above it or beyond its sides.
 lda playery
 cmp #176
 bcs laser_hit_next
 clc
 adc #23
 cmp #40
 bcc laser_hit_next
 sec
 lda laser_low,x
 sbc playerx
 sta temp
 lda laser_high,x
 sbc playerx+1
 bne laser_hit_next
 lda temp
 cmp #16
 bcs laser_hit_next
 lda #3
 jsr hurt_player
laser_hit_next:
 inc laser_index
 lda laser_index
 and #1
 bne laser_hit_loop
laser_return:
 rts
draw_lasers:
 jsr laser_begin
 bcc laser_return
laser_draw_loop:
 jsr laser_get_phase
 sec
 lda laser_low,x
 sbc camera
 sta tx
 lda laser_high,x
 sbc camera+1
 bne laser_draw_next
 lda tx
 cmp #252
 bcs laser_draw_next
 lda #176
 sta ty
 lda #19
 sta ink
 lda #<laser_emitter
 sta src
 lda #>laser_emitter
 sta src+1
 jsr hud_icon
 lda laser_phase
 cmp #88
 bcc laser_draw_next
 lda #40
 sta ty
 lda #19
 sta ink
 lda laser_phase
 cmp #112
 bcc laser_column
 lda #40
 sta ty
 lda #15
 sta ink
laser_column:
 ldx tx
 ldy ty
 jsr plot
 inx
 jsr plot
 lda laser_phase
 cmp #112
 bcs laser_solid_step
 lda ty
 clc
 adc #7
 sta ty
laser_solid_step:
 inc ty
 lda ty
 cmp #176
 bne laser_column
laser_draw_next:
 inc laser_index
 lda laser_index
 and #1
 bne laser_draw_loop
 rts
laser_emitter: .byte $c0,$e0,$e0,$f0,$00,$00,$00

