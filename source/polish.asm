; Compact inline UI commands keep the native program within stock RAM.
ui_ptr=$bc
title_help=$be
title_choice=$bf
trace_index=$c0
trace_step=$c1
trace_base=$c2
logo_phase=$c3
logo_depth=$c4
heart_index=$c5
heart_units=$c6
heart_row=$c7
heart_col=$c8
heart_bits=$c9
enemy_buffer=$5d40
ui_text:
 pla
 sta ui_ptr
 pla
 sta ui_ptr+1
 ldy #1
 lda (ui_ptr),y
 sta textscale
 iny
 lda (ui_ptr),y
 sta tx
 iny
 lda (ui_ptr),y
 sta ty
 iny
 lda (ui_ptr),y
 sta ink
 iny
 lda (ui_ptr),y
 sta txt
 iny
 lda (ui_ptr),y
 sta txt+1
 lda #6
 jsr ui_advance
 lda ui_ptr+1
 pha
 lda ui_ptr
 pha
 jmp draw_text
ui_panel:
 pla
 sta ui_ptr
 pla
 sta ui_ptr+1
 ldy #1
 lda (ui_ptr),y
 sta tx
 iny
 lda (ui_ptr),y
 sta ty
 iny
 lda (ui_ptr),y
 sta sw
 iny
 lda (ui_ptr),y
 sta sh
 iny
 lda (ui_ptr),y
 sta ink
 lda #5
 jsr ui_advance
 lda ui_ptr+1
 pha
 lda ui_ptr
 pha
 jmp rectangle
ui_advance:
 clc
 adc ui_ptr
 sta ui_ptr
 bcc ui_advance_done
 inc ui_ptr+1
ui_advance_done:
 rts

; One RLE decoder shared by enemy animation frames and the clinic vendor.
unpack_actor:
 lda #<enemy_buffer
 sta src2
 lda #>enemy_buffer
 sta src2+1
 ldy #0
 jsr actor_read
 jsr actor_write
 jsr actor_read
 jsr actor_write
actor_run:
 jsr actor_read
 beq actor_ready
 tax
 jsr actor_read
actor_repeat:
 jsr actor_write
 dex
 bne actor_repeat
 bra actor_run
actor_ready:
 lda #<enemy_buffer
 sta src
 lda #>enemy_buffer
 sta src+1
 rts
actor_read:
 lda (src),y
 inc src
 bne actor_read_done
 inc src+1
actor_read_done:
 cmp #0
 rts
actor_write:
 sta (src2),y
 inc src2
 bne actor_write_done
 inc src2+1
actor_write_done:
 rts
animate_enemy:
 ldx obj
 lda etype,x
 asl
 sta temp
 lda stage
 cmp #5
 bne actor_phase
 lda #6
 sta temp
actor_phase:
 lda frame
 lsr
 lsr
 lsr
 clc
 adc obj
 and #1
 ora temp
 tay
 sta temp
 asl
 clc
 adc temp
 adc #$40
 sta actor_job+6
 jsr actor_copy
 lda #<enemy_buffer
 sta src
 lda #>enemy_buffer
 sta src+1
 rts

; Cached HUD pixels keep busy districts at their previous cadence.
draw_hearts:
 lda health
 asl
 clc
 adc #$60
 sta heart_job+6
 lda back
 sta heart_job+10
heart_copy:
 lda #0
 sta $d702
 sta $d704
 lda #>heart_job
 sta $d701
 lda #<heart_job
 sta $d705
 rts
