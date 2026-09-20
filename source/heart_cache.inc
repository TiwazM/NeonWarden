; Nineteen health states, 384 bytes each in 512-byte slots at $16000.
init_heart_cache:
 lda #0
 sta health
 sta pix+3
 lda #4
 sta pix+2
 jsr ui_panel
 .byte 0,0,48,8,0
heart_cache_next:
 jsr build_hearts
 jsr heart_copy
 inc heart_job+9
 inc heart_job+9
 inc health
 lda health
 cmp #19
 bne heart_cache_next
 lda #$60
 sta heart_job+6
 lda #1
 sta heart_job+7
 lda #0
 sta heart_job+9
 rts
heart_job:
 .byte $0a,0,0
 .word 384
 .word 0
 .byte 4
 .word $6000
 .byte 1
 .word 0
