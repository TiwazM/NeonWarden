start:
 sei
 cld
 lda #0
 tab
 tax
 tay
 taz
 map
 eom
 see
 ldx #$ff
 txs
 ; Make banking pins outputs before selecting RAM under BASIC/KERNAL.
 lda #$2f
 sta $00
 lda #$35
 sta $01
 lda #$47
 sta $d02f
 lda #$53
 sta $d02f
 lda #0
 sta $d030
 lda #$7f
 sta $dc0d
 sta $dd0d
 lda $dc0d
 lda $dd0d
 lda #0
 sta $d01a
 sta $d015
 sta $d011
 ; Base VIC-II geometry first, then turn off hot-register derivation.
 lda #8
 sta $d016
 lda #$18
 sta $d018
 lda #$60
 sta $d031
 lda #$1b
 sta $d011
 lda $d05d
 and #$7f
 sta $d05d
 lda #$47             ; VFAST + FCLRHI + FCLRLO + CHR16
 sta $d054
 lda #5              ; RAM palette + expose full 2 KB color RAM
 sta $d030
 lda #0
 sta $d020
 sta $d021
 sta $d070
 sta $d059
 sta $d062
 sta $d063
 sta $d064
 sta $d065
 lda #80
 sta $d058
 lda #40
 sta $d05e
 lda #25
 sta $d07b
 ; Disable extended sprites and bitplanes; preserve video sync polarity.
 lda #0
 sta $d06b
 sta $d071
 ldx #0
init_palette:
 cpx #palette_count
 bcc palette_defined
 lda #0
 sta $d100,x
 sta $d200,x
 sta $d300,x
 bra palette_clear_ram
palette_defined:
 lda palette_r,x
 sta $d100,x
 lda palette_g,x
 sta $d200,x
 lda palette_b,x
 sta $d300,x
palette_clear_ram:
 lda #0
 sta $d800,x
 sta $d900,x
 sta $da00,x
 sta $db00,x
 sta $dc00,x
 sta $dd00,x
 sta $de00,x
 sta $df00,x
 cpx #64
 bcs palette_border_done
 sta $6800,x
palette_border_done:
 inx
 bne init_palette
 ; Two 40x25 screen maps point into separate 256x192 FCM buffers.
 lda #0
 sta pix
 lda #$70
 sta pix+1
 ; Generate the same 40x25 map once, instead of storing a 2 KB template.
 lda #0
 sta src
 sta temp
 sta ty
 lda #$70
 sta src+1
 lda #$10
 sta temp2
map_row:
 lda #0
 sta tx
map_cell:
 lda #$a0
 ldx #1
 ldy ty
 cpy #24
 bcs map_store
 ldy tx
 cpy #4
 bcc map_store
 cpy #36
 bcs map_store
 lda temp
 ldx temp2
 inc temp
 bne map_store
 inc temp2
map_store:
 ldy #0
 sta (src),y
 iny
 txa
 sta (src),y
 clc
 lda src
 adc #2
 sta src
 bcc map_pointer
 inc src+1
map_pointer:
 inc tx
 lda tx
 cmp #40
 bne map_cell
 inc ty
 lda ty
 cmp #25
 bne map_row
 lda #0
 ldx #47
map_padding:
 sta $77d0,x
 dex
 bpl map_padding
 ; Buffer B char numbers are buffer A + $400 for inner cells.
 ldx #0
copy_screens:
 lda $7000,x
 sta $7800,x
 lda $7100,x
 sta $7900,x
 lda $7200,x
 sta $7a00,x
 lda $7300,x
 sta $7b00,x
 lda $7400,x
 sta $7c00,x
 lda $7500,x
 sta $7d00,x
 lda $7600,x
 sta $7e00,x
 lda $7700,x
 sta $7f00,x
 inx
 bne copy_screens
 lda #$78
 sta pix+1
 lda #1
 sta pix
 ldx #8
 ldy #0
fix_map:
 lda (pix),y
 cmp #$10
 bcc fix_next
 clc
 adc #4
 sta (pix),y
fix_next:
 iny
 iny
 bne fix_map
 inc pix+1
 dex
 bne fix_map
 lda #0
 sta $d060
 lda #$70
 sta $d061
