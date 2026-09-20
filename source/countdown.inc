; District time is measured in the existing 25-Hz simulation clock.
; Menus, pause, death and endings do not call countdown_tick.
reset_countdown:
 ldx stage
 lda district_seconds,x
 sta time_left
 lda #25
 sta time_fraction
 lda #0
 sta time_expired
 rts
countdown_tick:
 lda difficulty
 beq countdown_done
 dec time_fraction
 bne countdown_done
 lda #25
 sta time_fraction
 lda time_left
 beq countdown_expire
 dec time_left
 bne countdown_done
countdown_expire:
 lda #1
 sta time_expired
 lda #0
 sta lives
 jmp player_dead
countdown_done:
 rts
draw_countdown:
 lda difficulty
 beq countdown_done
 lda time_left
 ldx #48
 cmp #100
 bcc countdown_digits
 sec
 sbc #100
 inx
countdown_digits:
 stx time_text
 jsr digits
 stx time_text+1
 sta time_text+2
 lda #19
 ldx time_left
 cpx #16
 bcs countdown_color
 lda #15
countdown_color:
 sta ink
 lda #34
 sta tx
 lda #9
 sta ty
 lda #<clock_icon
 sta src
 lda #>clock_icon
 sta src+1
 jsr hud_icon
 lda #44
 sta tx
 lda #<time_text
 sta txt
 lda #>time_text
 sta txt+1
 jmp draw_text
prison_reason:
 lda time_expired
 bne prison_timeout
 jsr ui_text
 .byte 1,86,162,15
 .word prison_text6
 rts
prison_timeout:
 jsr ui_text
 .byte 1,92,162,15
 .word timeout_text
 rts
district_seconds: .byte 60,75,65,60,70,100,80
time_left: .byte 0
time_fraction: .byte 25
time_expired: .byte 0
time_text: .text "000"
 .byte 0
timeout_text: .text "TIME EXPIRED"
 .byte 0
clock_icon: .byte $38,$44,$92,$9a,$82,$44,$38

