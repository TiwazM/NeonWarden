; Secret session unlock; no disk access and no gameplay updates during state 7.
tape_unlock:
 lda difficulty
 cmp #1
 bne tape_return
 lda run_cheat
 bne tape_return
 lda #1
 sta tape_unlocked
 rts
tape_mode_cycle:
 inc difficulty
 lda tape_unlocked
 beq tape_cycle_locked
 lda difficulty
 cmp #3
 bcc tape_return
 bra tape_cycle_zero
tape_cycle_locked:
 lda difficulty
 cmp #2
 bcc tape_return
tape_cycle_zero:
 lda #0
 sta difficulty
tape_return:
 rts
tape_check_selection:
 lda tape_unlocked
 bne tape_return
 lda difficulty
 cmp #2
 bne tape_return
 lda #1
 sta difficulty
 rts
tape_mode_label:
 lda difficulty
 cmp #2
 bne tape_label_regular
 jsr ui_text
 .byte 1,83,159,19
 .word tape_mode_text
 rts
tape_label_regular:
 jmp difficulty_menu_regular
tape_new_run:
 lda #0
 sta tape_count
 sta tape_clock
 sta tape_joke
 lda frame
 eor $d012
tape_random_reduce:
 cmp #7
 bcc tape_random_ready
 sec
 sbc #7
 bra tape_random_reduce
tape_random_ready:
 ina
 sta tape_joke_load
 rts
tape_load:
 lda difficulty
 cmp #2
 beq tape_start
 jmp load_stage
tape_start:
 ; stage already points to the next district; current state is frozen.
 lda #0
 sta tape_clock
 sta tape_joke
 inc tape_count
 lda tape_count
 cmp tape_joke_load
 bne tape_no_joke
 lda #1
 sta tape_joke
tape_no_joke:
 ldx stage
 lda tape_district_numbers,x
 sta tape_district_digit
 sei
 lda #0
 sta fx_pending
 sta sword_pending
 sta soundtime
 sta sword_time
 jsr effects_off
 jsr sword_off
 lda #0
 sta $d438
 sta $d478
 lda #7
 sta state
 cli
 rts
tape_tick:
 inc tape_clock
 lda tape_joke
 beq tape_short
 lda tape_clock
 cmp #140
 bra tape_finished_test
tape_short:
 lda tape_clock
 cmp #115
tape_finished_test:
 bcc tape_tick_done
 sei
 lda #0
 sta $d404
 sta $d40b
 sta $d412
 sta $d418
 sta $d020
 jsr tape_edge_fill
 lda #192
 sta tape_irq_line
 sta $d012
 lda #1
 sta $d019
 ; Force music instrument/song setup on return to gameplay.
 lda #255
 sta audio_song
 jsr load_stage
 cli
tape_tick_done:
 jmp menu_done
; Raster bands around the physical border. Music keeps one IRQ step at line 192.
tape_raster:
 lda state
 cmp #7
 beq tape_raster_active
 lda tape_irq_line
 cmp #192
 beq tape_raster_audio
 lda #192
 sta tape_irq_line
 sta $d012
 clc
 rts
tape_raster_audio:
 sec
 rts
tape_raster_active:
 lda tape_irq_line
 lsr
 lsr
 lsr
 lsr
 lsr
 clc
 adc tape_clock
 and #7
 tax
 lda tape_colors,x
 sta $d020
 jsr tape_edge_fill
 lda tape_irq_line
 pha
 clc
 adc #32
 bne tape_next_line
 lda #32
tape_next_line:
 sta tape_irq_line
 sta $d012
 pla
 cmp #192
 beq tape_raster_audio
 clc
 rts
; Quiet SID-only cassette parody, respecting mute. No samples or actual IO.
tape_audio:
 lda state
 cmp #7
 beq tape_audio_active
 jmp audio_tick_normal
tape_audio_active:
 inc audio_frames
 lda #0
 sta $d438
 sta $d478
 sta $d40b
 sta $d412
 lda muted
 bne tape_audio_silence
 lda tape_clock
 cmp #12
 bcc tape_audio_silence
 lda tape_joke
 beq tape_audio_blocks
 lda tape_clock
 cmp #85
 bcc tape_audio_blocks
 cmp #110
 bcc tape_audio_silence
tape_audio_blocks:
 lda tape_clock
 and #15
 cmp #13
 bcs tape_audio_silence
 lda #5
 sta $d418
 lda #$00
 sta $d405
 lda #$80
 sta $d406
 lda #$80
 sta $d400
 lda tape_clock
 and #7
 tax
 lda tape_pitches,x
 sta $d401
 lda tape_clock
 and #3
 beq tape_audio_noise
 lda #$11
 bra tape_audio_gate
tape_audio_noise:
 lda #$81
tape_audio_gate:
 sta $d404
 rts
tape_audio_silence:
 lda #0
 sta $d404
 sta $d418
 rts
tape_render:
 lda back
 sta pix+2
 lda #0
 sta pix+3
 sta tape_row
 sta tape_band_y
 jsr tape_visual_phase
tape_band_loop:
 lda tape_row
 clc
 adc tape_clock
 and #7
 tax
 lda tape_colors,x
 sta tape_band_color
 jsr ui_panel
 .byte 0,0,0,16,10
tape_band_y = *-4
tape_band_color = *-1
 clc
 lda tape_band_y
 adc #16
 sta tape_band_y
 inc tape_row
 lda tape_row
 cmp #12
 bcc tape_band_loop
 jsr ui_panel
 .byte 16,24,224,144,2
 jsr ui_text
 .byte 1,35,36,6
 .word tape_header
 jsr tape_loading_heading
 ldx tape_phase
 lda stage
 beq tape_first_phase
 lda tape_phase_lo,x
 sta txt
 lda tape_phase_hi,x
 bra tape_phase_pointer
tape_first_phase:
 lda tape_first_lo,x
 sta txt
 lda tape_first_hi,x
tape_phase_pointer:
 sta txt+1
 lda #35
 sta tx
 lda #103
 sta ty
 lda #7
 sta ink
 lda #1
 sta textscale
 jsr draw_text
 jsr ui_text
 .byte 1,35,128,6
 .word tape_progress_text
 jsr ui_text
 .byte 1,35,151,6
 .word tape_bottom
 jmp render_swap
tape_visual_phase:
 lda tape_clock
 ldx tape_joke
 beq tape_phase_regular
 cmp #85
 bcc tape_phase_regular
 cmp #110
 bcs tape_after_joke
 ldx #5
 cmp #97
 bcc tape_joke_phase
 inx
tape_joke_phase:
 stx tape_phase
 lda #57
 sta tape_percent
 sta tape_percent+1
 rts
tape_after_joke:
 sec
 sbc #25
tape_phase_regular:
 ldx #0
tape_phase_divide:
 cmp #23
 bcc tape_phase_ready
 sec
 sbc #23
 inx
 bra tape_phase_divide
tape_phase_ready:
 stx tape_phase
 lda tape_percent_tens,x
 sta tape_percent
 lda tape_percent_ones,x
 sta tape_percent+1
 rts
tape_unlocked: .byte 0
tape_count: .byte 0
tape_clock: .byte 0
tape_joke: .byte 0
tape_joke_load: .byte 1
tape_irq_line: .byte 192
tape_row: .byte 0
tape_phase: .byte 0
tape_colors: .byte 10,0,19,8,15,0,7,9
tape_pitches: .byte 12,24,13,27,17,23,14,29
tape_district_numbers: .text "1234576"
tape_percent_tens: .text "02479"
tape_percent_ones: .text "00869"
tape_mode_text: .text "MODE: 1991 TAPE"
 .byte 0
tape_header: .text "*** 1991 TAPE SYSTEM ***"
 .byte 0
tape_loading: .text "LOADING DISTRICT "
tape_district_digit: .text "2..."
 .byte 0
tape_found: .text "FOUND NEON WARDEN"
 .byte 0
tape_progress_text: .text "TAPE COUNTER: "
tape_percent: .text "00%"
 .byte 0
tape_bottom: .text "PLEASE DO NOT REWIND."
 .byte 0
tape_phase0: .text "SEARCHING..."
 .byte 0
tape_phase1: .text "LOADING BLOCK 01"
 .byte 0
tape_phase2: .text "LOADING BLOCK 02"
 .byte 0
tape_phase3: .text "LOADING BLOCK 03"
 .byte 0
tape_phase4: .text "VERIFYING..."
 .byte 0
tape_phase5: .text "?LOAD ERROR"
 .byte 0
tape_phase6: .text "JUST KIDDING..."
 .byte 0
tape_phase_lo: .byte <tape_phase0,<tape_phase1,<tape_phase2,<tape_phase3,<tape_phase4,<tape_phase5,<tape_phase6
tape_phase_hi: .byte >tape_phase0,>tape_phase1,>tape_phase2,>tape_phase3,>tape_phase4,>tape_phase5,>tape_phase6

tape_title_unlock:
 lda tape_unlocked
 beq tape_title_done
 jsr ui_text
 .byte 1,77,4,19
 .word tape_unlocked_text
tape_title_done:
 rts
tape_unlocked_text: .text "1991 TAPE UNLOCKED"
 .byte 0

; The centered framebuffer leaves 4 tile columns each side and a bottom row.
; They all reference the dedicated solid FCM tile at $6800.
; Match it to the physical raster border only while loading; restore zero on exit.
tape_edge_fill:
 ldx #63
tape_edge_pixel:
 sta $6800,x
 dex
 bpl tape_edge_pixel
 rts

tape_launch:
 jsr new_game
 lda difficulty
 cmp #2
 bne tape_launch_done
 jmp tape_start
tape_launch_done:
 rts

tape_loading_heading:
 lda stage
 beq tape_first_heading
 jsr ui_text
 .byte 1,35,58,7
 .word tape_loading
 jsr ui_text
 .byte 1,35,79,7
 .word tape_found
 rts
tape_first_heading:
 jsr ui_text
 .byte 1,35,58,7
 .word tape_found
 rts
tape_press_play: .text "PRESS PLAY ON TAPE"
 .byte 0
tape_found_first: .text "FOUND DISTRICT 1"
 .byte 0
tape_first_lo: .byte <tape_press_play,<tape_phase0,<tape_found_first,<tape_phase1,<tape_phase2,<tape_phase5,<tape_phase6
tape_first_hi: .byte >tape_press_play,>tape_phase0,>tape_found_first,>tape_phase1,>tape_phase2,>tape_phase5,>tape_phase6
