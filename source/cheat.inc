; Title-only code. Session toggle; each run latches its eligibility.
cheat_dispatch:
 lda state
 beq cheat_title_tick
 lda #0
 sta cheat_progress
 jmp cheat_dispatch_return
cheat_title_tick:
 lda cheat_notice
 beq cheat_read
 dec cheat_notice
cheat_read:
 lda event
 beq cheat_no_event
 ; Normalize alphabetic ASCII/PETSCII without corrupting digits.
 and #$7f
 cmp #$61
 bcc cheat_match
 cmp #$7b
 bcs cheat_match
 and #$df
cheat_match:
 ldx cheat_progress
 cmp cheat_code,x
 beq cheat_matched
 ldx #0
 stx cheat_progress
 cmp #$54
 beq cheat_matched
 jmp cheat_dispatch_return
cheat_matched:
 inc cheat_progress
 lda cheat_progress
 cmp #10
 bne cheat_consume
 lda #0
 sta cheat_progress
 lda gear_cheat
 eor #1
 sta gear_cheat
 jsr tape_check_selection
 lda #100
 sta cheat_notice
cheat_consume:
 lda #0
 sta event
 sta keys
 jmp menu_done
cheat_no_event:
 lda cheat_progress
 bne cheat_consume
 jmp cheat_dispatch_return
cheat_begin:
 lda #0
 sta cheat_progress
 sta run_cheat
 lda difficulty
 beq cheat_return
 lda gear_cheat
 sta run_cheat
cheat_return:
 rts
cheat_retry:
 lda run_cheat
 bne cheat_return
 jmp cheat_retry_original
cheat_retry_original:
 lda difficulty
 beq difficulty_retry_done
 lda #0
 sta blade
 sta armour
 sta gun
 sta ammo
 lda #1
 sta level
difficulty_retry_done:
 rts

cheat_death_text:
 lda run_cheat
 beq cheat_death_standard
 jsr ui_text
 .byte 1,86,98,11
 .word cheat_retained_text
 rts
cheat_death_standard:
 jmp cheat_death_original
cheat_title_status:
 lda gear_cheat
 bne cheat_status_on
 lda cheat_notice
 bne cheat_status_off
 jmp tape_title_unlock
cheat_status_off:
 jsr ui_text
 .byte 1,74,4,19
 .word cheat_off_text
 rts
cheat_status_on:
 jsr ui_text
 .byte 1,56,4,19
 .word cheat_on_text
 rts
cheat_score_status:
 lda run_cheat
 beq cheat_score_normal
 jsr ui_text
 .byte 1,41,162,19
 .word cheat_unranked_text
 rts
cheat_score_normal:
 jsr ui_text
 .byte 1,41,162,5
 .word score_session_text
 rts
gear_cheat: .byte 0
run_cheat: .byte 0
cheat_progress: .byte 0
cheat_notice: .byte 0
cheat_code: .text "TINWIM2077"
; The code is ten characters: T I N W I M 2 0 7 7.
cheat_retained_text: .text "GEAR RETAINED."
 .byte 0
cheat_on_text: .text "1991 GEAR SAFE / NO SCORE"
 .byte 0
cheat_off_text: .text "GEAR LOSS RESTORED"
 .byte 0
cheat_unranked_text: .text "CHEAT RUN / SCORE ZERO"
 .byte 0
