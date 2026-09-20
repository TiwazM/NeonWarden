intro_screen:
 jsr ui_panel
 .byte 0,0,0,192,32
 lda #0
 sta trace_index
 lda #4
 sta trace_base
trace_next:
 lda #0
 sta trace_step
trace_point:
 lda frame
 lsr
 lsr
 lsr
 and #1
 clc
 adc #8
 sta ink
 lda trace_index
 asl
 asl
 clc
 adc frame
 and #63
 cmp trace_step
 bne trace_dim
 lda #11
 sta ink
trace_dim:
 lda trace_step
 sec
 sbc #32
 bcs trace_diagonal
 lda #0
trace_diagonal:
 cmp #13
 bcc trace_x
 lda #12
trace_x:
 clc
 adc trace_base
 tax
 lda trace_step
 clc
 adc #8
 tay
 jsr plot
 txa
 eor #255
 tax
 lda #191
 sec
 sbc trace_step
 sec
 sbc #8
 tay
 jsr plot
 inc trace_step
 lda trace_step
 cmp #64
 bne trace_point
 clc
 lda trace_base
 adc #28
 sta trace_base
 inc trace_index
 lda trace_index
 cmp #8
 bne trace_next
 lda title_help
 bne intro_menu_jump
 jsr ui_panel
 .byte 16,16,224,16,32
 jsr ui_panel
 .byte 48,128,160,48,32
 lda #0
 sta trace_index
logo_pin:
 lda #9
 sta ink
 lda trace_index
 asl
 asl
 asl
 clc
 adc #48
 tax
 ldy #53
 jsr plot
 iny
 jsr plot
 iny
 jsr plot
 ldy #104
 jsr plot
 iny
 jsr plot
 iny
 jsr plot
 inc trace_index
 lda trace_index
 cmp #20
 bne logo_pin
 jsr ui_panel
 .byte 40,56,192,56,0
 jsr ui_panel
 .byte 32,48,192,56,9
 jsr ui_panel
 .byte 40,56,176,40,3
 jsr ui_panel
 .byte 48,64,160,32,1
 lda frame
 lsr
 lsr
 and #15
 sta logo_phase
 lda #5
 sta logo_depth
logo_layer:
 ldx logo_phase
 lda logo_wave,x
 clc
 adc #63
 sta tx
 lda logo_depth
 cpx #8
 bcc logo_right
 eor #255
 ina
logo_right:
 clc
 adc tx
 sta tx
 txa
 clc
 adc #4
 and #15
 tax
 lda logo_wave,x
 clc
 adc #73
 clc
 adc logo_depth
 sta ty
 ldx logo_depth
 lda logo_colors,x
 cpx #0
 bne logo_ink
 lda frame
 and #8
 beq logo_mint
 lda #7
 bra logo_ink
logo_mint:
 lda #11
logo_ink:
 sta ink
 lda #2
 sta textscale
 lda #<logo_text
 sta txt
 lda #>logo_text
 sta txt+1
 jsr draw_text
 dec logo_depth
 bpl logo_layer
intro_menu_jump:
 jmp intro_menu
logo_wave: .byte 0,2,4,5,5,5,4,2,0,254,252,251,251,251,252,254
logo_colors: .byte 11,10,9,8,3,2
logo_text: .text "NEON WARDEN"
 .byte 0

intro_input:
 lda event
 and #$df
 cmp #$48
 bne intro_not_h
 lda title_help
 eor #1
 sta title_help
 jmp menu_done
intro_not_h:
 lda title_help
 beq intro_choose
 lda keys
 and #24
 beq intro_input_done
 eor prev
 and #24
 beq intro_input_done
 lda #0
 sta title_help
 jmp menu_done
intro_choose:
 lda keys
 eor prev
 and keys
 and #20
 beq intro_fire
 and #4
 bne intro_up
 inc title_choice
 lda title_choice
 cmp #4
 bcc intro_fire
 lda #0
 sta title_choice
 bra intro_fire
intro_up:
 dec title_choice
 bpl intro_fire
 lda #3
 sta title_choice
intro_fire:
 lda keys
 eor prev
 and keys
 and #8
 beq intro_input_done
 lda title_choice
 cmp #3
 bne score_title_choice
 jmp score_open
score_title_choice:
 lda title_choice
 beq intro_start
 cmp #1
 beq intro_help
 jsr tape_mode_cycle
 bra intro_input_done
intro_help:
 lda #1
 sta title_help
 bra intro_input_done
intro_start:
 jsr tape_launch
intro_input_done:
 jmp menu_done

; The large stationary shopkeeper streams straight to the framebuffer.
draw_vendor:
 ldy #0
 jsr actor_read
 sta sw
 jsr actor_read
 sta sh
 lda #0
 sta sx
 sta sy
vendor_run:
 ldy #0
 jsr actor_read
 beq vendor_done
 sta lzcount
 jsr actor_read
 sta ink
vendor_pixel:
 lda ink
 beq vendor_advance
 lda sx
 clc
 adc ox
 tax
 lda sy
 clc
 adc oy
 tay
 jsr plot
vendor_advance:
 inc sx
 lda sx
 cmp sw
 bne vendor_column
 lda #0
 sta sx
 inc sy
vendor_column:
 dec lzcount
 bne vendor_pixel
 bra vendor_run
vendor_done:
 rts

heart_shape: .word $39c,$7fe,$fff,$7fe,$3fc,$1f8,$060

heart_edge: .word 0
heart_outline: .word $39c,$462,$801,$402,$204,$198,$060


