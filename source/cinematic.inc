; Native-size courier on a foreground rooftop, skyline behind.
cine_time=$cb
cine_particle=$cc
cine_distance=$cd
cinematic:
 jsr draw_background
 jsr draw_clouds
 jsr ui_panel
 .byte 0,0,0,32,0
 jsr ui_panel
 .byte 0,152,0,8,5
 jsr ui_panel
 .byte 0,160,0,16,2
 jsr ui_panel
 .byte 0,176,0,16,0
 jsr ui_panel
 .byte 32,168,32,8,3
 jsr ui_panel
 .byte 192,168,32,8,3
 jsr ui_panel
 .byte 152,128,16,24,2
 jsr ui_panel
 .byte 152,128,16,8,10
 lda cine_time
 cmp #160
 bcc cinema_caption_start
 jsr ui_text
 .byte 2,51,8,11
 .word cinema_names
 bra cinema_caption_done
cinema_caption_start:
 jsr ui_text
 .byte 1,83,12,6
 .word cinema_delivery
cinema_caption_done:
 jsr ui_text
 .byte 1,98,180,6
 .word cinema_return
 lda #0
 sta facing
 lda #128
 sta oy
 lda cine_time
 cmp #96
 bcs cinema_at_terminal
 clc
 adc #32
 sta ox
 lda cine_time
 lsr
 lsr
 lsr
 and #3
 ina
 bra cinema_pose
cinema_at_terminal:
 lda #128
 sta ox
 lda #7
 ldx cine_time
 cpx #160
 bcc cinema_pose
 lda #0
cinema_pose:
 jsr pose_selected
 jsr cinema_hero
 jmp cinema_particles
