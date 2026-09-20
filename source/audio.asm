; Original dual-SID score + two-operator OPL effects.
; SID 2 ($D420) and SID 4 ($D460) follow the ION DRIFT stereo layout.
; OPL registers: $FE00000-$FE000FF, directly addressed (not SFX cart ports).
; Mapping verified in MEGA65 src/vhdl/slow_devices.vhdl and Xemu's mapper;
; implementation and all music/patches here are original.
; Everything below runs in the raster IRQ except init and the atomic sfx queue.
audio_song=$a5
audio_section=$a6
audio_voice=$a7
audio_index=$a8
audio_note=$a9
audio_ptr=$aa                 ; two bytes, never shared with renderer
opl_ptr=$ac                   ; four-byte physical pointer
opl_value=$b0
fx_pending=$b1
fx_active=$b2
fx_mode=$b3                   ; 0 OPL, 1 SID compatibility (F key)
fx_mode_last=$b4
fx_pitch=$b5
fx_high=$b6
audio_mixgain=$d0
audio_frames=$b7
sword_pending=$b9
sword_time=$ba
sword_pitch=$bb
audio_gain=$b8                ; music gain, also permits isolated audio tests

init_audio:
 ; SID works on release cores; F opts into FM where supported.
 lda #1
 sta fx_mode
 ldx #24
 lda #0
audio_zero:
 sta $d400,x
 sta $d420,x
 sta $d440,x
 sta $d460,x
 dex
 bpl audio_zero
 sta opl_ptr
 lda #0
 sta opl_ptr+1
 lda #$e0
 sta opl_ptr+2
 lda #$ff
 sta audio_song
 lda #$0f
 sta opl_ptr+3
 lda #6
 sta audio_gain
 sta audio_mixgain
 ldy #5
audio_instrument:
 ldx sid_offsets,y
 lda #8
 sta $d423,x
 lda sid_attack,y
 sta $d425,x
 lda sid_release,y
 sta $d426,x
 dey
 bpl audio_instrument
 ; Silence every OPL channel, enable waveform selection and stereo output.
 ldx #$b0
audio_opl_clear:
 lda #0
 jsr opl_write
 inx
 cpx #$b9
 bne audio_opl_clear
 ldx #1
 lda #$20
 jsr opl_write
 ldx #$bd
 lda #0
 jsr opl_write
 ; OPL mixer input 14, speakers and headphones. Other inputs untouched.
 ldx #7
audio_mix:
 lda opl_mixer_regs,x
 sta $d6f4
 jsr opl_wait
 lda #$ff
 sta $d6f5
 jsr opl_wait
 dex
 bpl audio_mix
 jmp sword_init

; Preserve all caller registers; one-byte handoff cannot tear across an IRQ.
sfx:
 cmp #2
 bne sfx_general
 sta sword_pending
 rts
sfx_general:
 sta fx_pending
 rts

; X = OPL register, A = value. 40 us settling at 40.5 MHz, longer at slow speed.
opl_write:
 sta opl_value
 txa
 taz
 lda opl_value
 nop
 sta (opl_ptr),z
opl_wait:
 phy
 ldy #255
opl_wait_a:
 dey
 bne opl_wait_a
 ldy #80
opl_wait_b:
 dey
 bne opl_wait_b
 ply
 rts

audio_tick:
 jmp tape_audio
audio_tick_normal:
 inc audio_frames
 lda muted
 beq audio_live
 lda #0
 sta $d418
 sta $d438
 sta $d478
 sta $d412
 sta fx_pending
 sta soundtime
 sta sword_pending
 sta sword_time
 sta $d40b
 ldx #$b1
 jsr opl_write
 lda #0
 ldx #$b0
 jsr opl_write
 lda #$3f
 ldx #$43
 jsr opl_write
 lda #$3f
 ldx #$44
 jmp opl_write
audio_live:
 lda fx_pending
 ora sword_pending
 ora soundtime
 ora sword_time
 beq music_full_gain
 lda audio_gain
 sec
 sbc #2
 bcs music_gain_target
 lda #0
 bra music_gain_target
music_full_gain:
 lda audio_gain
music_gain_target:
 cmp audio_mixgain
 beq music_gain_ready
 bcc music_gain_down
 inc audio_mixgain
 bra music_gain_ready
music_gain_down:
 dec audio_mixgain
music_gain_ready:
 lda audio_mixgain
 sta $d438
 sta $d478
 lda #10
 sta $d418
 lda fx_mode
 cmp fx_mode_last
 beq audio_fx
 sta fx_mode_last
 lda #0
 sta $d412
 sta soundtime
 sta sword_time
 sta $d40b
 ldx #$b1
 jsr opl_write
 lda #0
 ldx #$b0
 jsr opl_write
audio_fx:
 jsr sword_tick
 jsr effects_tick
 lda stage
 cmp audio_song
 beq music_tick
 sta audio_song
 tax
 lda songlo,x
 sta audio_ptr
 lda songhi,x
 sta audio_ptr+1
 lda #0
 sta musicstep
 sta musicdelay
 sta audio_section
music_tick:
 lda musicdelay
 beq music_note
 dec musicdelay
 bne audio_return
 ; Gate off one full 25 Hz tick before the next note, except tied pads.
 ldy #5
music_release:
 cpy #3
 beq music_release_next
 ldx sid_offsets,y
 lda sid_wave,y
 and #$fe
 sta $d424,x
music_release_next:
 dey
 bpl music_release
audio_return:
 rts
music_note:
 ldx audio_song
 lda songtempo,x
 dea
 sta musicdelay
 lda musicstep
 asl
 sta audio_index
 asl
 clc
 adc audio_index
 sta audio_index
 lda #0
 sta audio_voice
music_voice_loop:
 ldy audio_index
 lda (audio_ptr),y
 cmp #255
 beq music_next_voice
 ldy audio_voice
 ldx sid_offsets,y
 cmp #0
 beq music_rest
 cpy #2                       ; noise percussion has no harmonic shift
 beq music_pitch
 ldy audio_section
 clc
 adc songshift,y
music_pitch:
 sec
 sbc #24
 tay
 lda videoticks
 cmp #60
 beq music_ntsc
 lda freqlo_pal,y
 sta $d420,x
 lda freqhi_pal,y
 bra music_high
music_ntsc:
 lda freqlo_ntsc,y
 sta $d420,x
 lda freqhi_ntsc,y
music_high:
 sta $d421,x
 ldy audio_voice
 lda sid_wave,y
 bra music_gate
music_rest:
 lda #0
music_gate:
 sta $d424,x
music_next_voice:
 inc audio_index
 inc audio_voice
 lda audio_voice
 cmp #6
 bne music_voice_loop
 inc musicstep
 lda musicstep
 cmp #32
 bne audio_return
 lda #0
 sta musicstep
 inc audio_section
 lda audio_section
 and #3
 sta audio_section
 rts

effects_tick:
 lda fx_pending
 beq effects_decay
 sta fx_active
 lda #0
 sta fx_pending
 sta $d412
 ldx #$b0
 jsr opl_write
 ldy fx_active
 lda fx_duration,y
 sta soundtime
 lda fx_freq,y
 sta fx_pitch
 lda fx_block,y
 sta fx_high
 lda fx_mode
 bne effects_sid
 ; Two operators: ratio/modulation and envelope vary for every effect.
 ldx #$20
 lda fx_ratio,y
 jsr opl_write
 ldx #$23
 lda #1
 jsr opl_write
 ldx #$40
 lda fx_mod,y
 jsr opl_write
 ldx #$43
 lda #0                       ; leave transient effects clear above the score
 jsr opl_write
 ldx #$60
 lda #$f4
 jsr opl_write
 ldx #$63
 lda #$f3
 jsr opl_write
 ldx #$80
 lda #$26
 jsr opl_write
 ldx #$83
 lda #$26
 jsr opl_write
 ldx #$e0
 lda #0
 jsr opl_write
 ldx #$e3
 jsr opl_write
 ldx #$c0
 lda #$fe
 jsr opl_write
 jsr credit_fm_start
 jmp effects_opl_pitch
effects_sid:
 jmp credit_sid_start
effects_sid_regular:
 lda fx_pitch
 sta $d40e
 lda fx_high
 and #$1f
 sta $d40f
 lda #$02
 sta $d413
 lda #$68
 sta $d414
 lda fx_sid_wave,y
 sta $d412
 rts
effects_decay:
 jmp credit_decay_dispatch
effects_decay_regular:
 lda soundtime
 beq effects_return
 dec soundtime
 beq effects_off
 ldy fx_active
 clc
 lda fx_pitch
 adc fx_sweep,y
 sta fx_pitch
 lda fx_mode
 beq effects_opl_pitch
 lda fx_pitch
 sta $d40e
 rts
effects_opl_pitch:
 ldx #$a0
 lda fx_pitch
 jsr opl_write
 ldx #$b0
 lda fx_high
 ora #$20
 jmp opl_write
effects_off:
 lda #0
 sta $d412
 ldx #$b0
 jmp opl_write
effects_return:
 rts

; Sword has its own FM channel / SID fallback voice, so impacts cannot steal it.
sword_init:
 ldy #11
sword_init_loop:
 ldx sword_regs,y
 lda sword_vals,y
 jsr opl_write
 dey
 bpl sword_init_loop
 rts
sword_tick:
 lda sword_pending
 beq sword_decay
 lda #0
 sta sword_pending
 sta $d40b
 ldx #$b1
 jsr opl_write
 lda #4
 sta sword_time
 lda #220
 sta sword_pitch
 lda fx_mode
 beq sword_fm_start
 lda #220
 sta $d407
 lda #30
 sta $d408
 lda #2
 sta $d40c
 lda #$68
 sta $d40d
 lda #$81
 sta $d40b
 rts
sword_fm_start:
 lda #0
 ldx #$44
 jsr opl_write
 ldx #$a1
 lda sword_pitch
 jsr opl_write
 ldx #$b1
 lda #$2e
 jmp opl_write
sword_decay:
 lda sword_time
 beq sword_return
 dec sword_time
 beq sword_off
 lda sword_pitch
 sec
 sbc #30
 sta sword_pitch
 lda fx_mode
 beq sword_fm_pitch
 lda sword_pitch
 sta $d407
 rts
sword_fm_pitch:
 lda sword_pitch
 ldx #$a1
 jmp opl_write
sword_off:
 lda #0
 sta $d40b
 ldx #$b1
 jmp opl_write
sword_return:
 rts
sword_regs: .byte $21,$24,$41,$44,$61,$64,$81,$84,$e1,$e4,$c1,$b1
sword_vals: .byte 14,1,3,0,$f2,$f2,$26,$16,0,0,$fe,0

sid_offsets: .byte 0,7,14,64,71,78
sid_wave: .byte $41,$41,$81,$11,$21,$11
sid_attack: .byte $02,$04,$00,$38,$02,$04
sid_release: .byte $84,$65,$03,$a9,$44,$64
opl_mixer_regs: .byte $1c,$1d,$3c,$3d,$dc,$dd,$fc,$fd
audio_end:
.cerror audio_end > $6700, "Audio overlaps border character"
