intro_sound_status:
 lda fx_mode
 bne intro_sid_status
 jsr ui_text
 .byte 1,59,159,11
 .word text_67
 rts
intro_sid_status:
 jsr ui_text
 .byte 1,59,159,19
 .word text_68
 rts
