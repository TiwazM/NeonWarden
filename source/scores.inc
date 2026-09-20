; 1991 session records. No disk writes during gameplay.
score_reset:
 jsr tape_new_run
 jsr cheat_begin
 lda #0
 sta credit_points
 sta credit_points+1
 sta time_points
 sta time_points+1
 sta score_saved
 lda #255
 sta score_rank
 rts
score_credit:
 pha
 phx
 phy
 ldx run_cheat
 bne score_credit_done
 ldx difficulty
 beq score_credit_done
 tax
score_credit_loop:
 clc
 lda credit_points
 adc #10
 sta credit_points
 lda credit_points+1
 adc #0
 sta credit_points+1
 bcc score_credit_next
 lda #255
 sta credit_points
 sta credit_points+1
score_credit_next:
 dex
 bne score_credit_loop
score_credit_done:
 ply
 plx
 pla
 rts
score_clear_bonus:
 lda run_cheat
 bne score_bonus_done
 lda difficulty
 beq score_bonus_done
 ldx time_left
 beq score_bonus_done
score_bonus_loop:
 clc
 lda time_points
 adc #5
 sta time_points
 lda time_points+1
 adc #0
 sta time_points+1
 bcc score_bonus_next
 lda #255
 sta time_points
 sta time_points+1
score_bonus_next:
 dex
 bne score_bonus_loop
score_bonus_done:
 rts
score_total:
 lda run_cheat
 beq score_total_regular
 lda #0
 sta run_score
 sta run_score+1
 rts
score_total_regular:
 clc
 lda credit_points
 adc time_points
 sta run_score
 lda credit_points+1
 adc time_points+1
 sta run_score+1
 bcc score_total_done
 lda #255
 sta run_score
 sta run_score+1
score_total_done:
 rts
score_death:
 lda state
 cmp #5
 bne score_submit_done
score_submit:
 lda run_cheat
 bne score_submit_done
 lda difficulty
 beq score_submit_done
 lda score_saved
 bne score_submit_done
 inc score_saved
 jsr score_total
 ldx #0
score_find:
 txa
 lsr
 cmp score_count
 bcs score_insert
 lda run_score+1
 cmp score_table+1,x
 bcc score_find_next
 bne score_insert
 lda run_score
 cmp score_table,x
 bcc score_find_next
 bne score_insert
score_find_next:
 inx
 inx
 cpx #10
 bcc score_find
 rts
score_insert:
 stx score_slot
 txa
 lsr
 sta score_rank
 ldx #8
score_shift:
 cpx score_slot
 beq score_store
 lda score_table-2,x
 sta score_table,x
 lda score_table-1,x
 sta score_table+1,x
 dex
 dex
 bra score_shift
score_store:
 lda run_score
 sta score_table,x
 lda run_score+1
 sta score_table+1,x
 lda score_count
 cmp #5
 bcs score_submit_done
 inc score_count
score_submit_done:
 rts
score_run_exit:
 lda difficulty
 bne score_open
 jsr new_game
 lda #0
 sta state
 jmp menu_done
score_open:
 lda #6
 sta state
 jmp menu_done
score_dispatch:
 lda state
 cmp #7
 bne tape_tick_other
 jmp tape_tick
tape_tick_other:
 lda state
 cmp #6
 bne score_normal_tick
 lda keys
 and #8
 beq score_escape
 lda prev
 and #8
 beq score_close
score_escape:
 lda event
 cmp #27
 bne score_tick_done
score_close:
 lda #0
 sta state
 sta title_help
score_tick_done:
 jmp menu_done
score_normal_tick:
 jmp score_dispatch_return
score_render_dispatch:
 lda state
 cmp #7
 bne tape_render_other
 jmp tape_render
tape_render_other:
 lda state
 cmp #6
 bne score_normal_render
 lda #0
 sta $d020
 lda back
 sta pix+2
 lda #0
 sta pix+3
 jsr score_screen
 jmp render_swap
score_normal_render:
 jmp score_render_return
score_menu:
 jsr ui_text
 .byte 1,80,168,10
 .word score_menu_text
 rts
score_screen:
 jsr ui_panel
 .byte 0,0,0,192,0
 jsr ui_panel
 .byte 16,16,224,160,1
 jsr ui_text
 .byte 2,68,24,11
 .word score_heading
 jsr ui_text
 .byte 1,77,43,19
 .word score_subhead
 lda #0
 sta score_row
score_draw_row:
 ldx score_row
 txa
 clc
 adc #49
 sta score_row_text
 txa
 asl
 tax
 lda score_table,x
 ldy score_table+1,x
 jsr score_decimal
 ldx score_row
 cpx score_count
 bcc score_row_ready
 ldx #4
 lda #45
score_empty_digits:
 sta score_digits,x
 dex
 bpl score_empty_digits
score_row_ready:
 lda #6
 ldx score_row
 cpx score_rank
 bne score_row_ink
 lda #19
score_row_ink:
 sta score_row_color
 jsr ui_text
 .byte 1,66,60,6
score_row_y = *-2
score_row_color = *-1
 .word score_row_text
 ; ui_text uses inline y patched before each subsequent frame/row.
 inc score_row
 lda score_row
 cmp #5
 beq score_draw_footer
 clc
 lda score_row_y
 adc #12
 sta score_row_y
 bra score_draw_row
score_draw_footer:
 jsr score_total
 lda run_score
 ldy run_score+1
 jsr score_decimal
 jsr ui_text
 .byte 1,32,122,11
 .word score_total_label
 jsr ui_text
 .byte 1,188,122,11
 .word score_digits
 lda #60
 sta score_row_y
 jsr ui_text
 .byte 1,32,136,6
 .word score_credit_label
 lda credit_points
 ldy credit_points+1
 jsr score_decimal
 jsr ui_text
 .byte 1,188,136,19
 .word score_digits
 jsr ui_text
 .byte 1,32,147,6
 .word score_time_label
 lda time_points
 ldy time_points+1
 jsr score_decimal
 jsr ui_text
 .byte 1,188,147,19
 .word score_digits
 jsr cheat_score_status
 jsr ui_text
 .byte 1,53,181,6
 .word score_back_text
 rts
; Convert unsigned A/Y to five decimal digits without touching game scratch.
score_decimal:
 sta score_work
 sty score_work+1
 ldx #0
score_digit_next:
 lda #48
 sta score_digits,x
score_digit_sub:
 lda score_work+1
 cmp score_divhi,x
 bcc score_digit_finished
 bne score_digit_take
 lda score_work
 cmp score_divlo,x
 bcc score_digit_finished
score_digit_take:
 sec
 lda score_work
 sbc score_divlo,x
 sta score_work
 lda score_work+1
 sbc score_divhi,x
 sta score_work+1
 inc score_digits,x
 bra score_digit_sub
score_digit_finished:
 inx
 cpx #5
 bne score_digit_next
 rts
credit_points: .word 0
time_points: .word 0
run_score: .word 0
score_saved: .byte 0
score_rank: .byte 255
score_count: .byte 0
score_slot: .byte 0
score_row: .byte 0
score_work: .word 0
score_table: .fill 10,0
score_divlo: .byte <10000,<1000,<100,<10,<1
score_divhi: .byte >10000,>1000,>100,>10,>1
score_row_text: .text "1       "
score_digits: .text "00000"
 .byte 0
score_menu_text: .text "1991 HIGH SCORES"
 .byte 0
score_heading: .text "1991 SCORES"
 .byte 0
score_subhead: .text "TOP FIVE RUNS"
 .byte 0
score_credit_label: .text "LAST RUN: CREDITS X10"
 .byte 0
score_time_label: .text "CLEAR-TIME BONUS X5"
 .byte 0
score_session_text: .text "RECORDS LAST UNTIL RESET"
 .byte 0
score_back_text: .text "FIRE / ESC: TITLE SCREEN"
 .byte 0
score_total_label: .text "YOUR RUN SCORE"
 .byte 0
