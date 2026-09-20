; Normal sprite scale makes the courier fit the rooftop skyline.
cinema_hero:
 lda #<hero_buffer
 sta src
 lda #>hero_buffer
 sta src+1
 jmp draw_sprite

difficulty_retry:
 jmp cheat_retry

difficulty_menu:
 jmp tape_mode_label
difficulty_menu_regular:
 lda difficulty
 bne difficulty_1991
 jsr ui_text
 .byte 1,92,159,19
 .word difficulty_normal_text
 rts
difficulty_1991:
 jsr ui_text
 .byte 1,98,159,15
 .word difficulty_1991_text
 rts
difficulty_normal_text: .text "MODE: NORMAL"
 .byte 0
difficulty_1991_text: .text "MODE: 1991"
 .byte 0
death_mode_text:
 jmp cheat_death_text
cheat_death_original:
 lda difficulty
 beq death_normal
 jsr ui_text
 .byte 1,89,98,15
 .word death_1991_text
 rts
death_normal:
 jsr ui_text
 .byte 1,50,98,6
 .word death_normal_text
 rts
