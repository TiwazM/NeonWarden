.include "boot.inc"
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
 lda #$2f
 sta $00
 lda #$35
 sta $01
 ldx #0
loader_relocate:
 lda decoder_image,x
 sta $0400,x
 lda decoder_image+$100,x
 sta $0500,x
 inx
 bne loader_relocate
 jmp $0400
decoder_image:
.logical $0400
 lda #<payload
 sta $20
 lda #>payload
 sta $21
 lda #0
 sta $22
 sta $23
 sta $24
 sta $25
 sta $27
 lda #1
 sta $26
 lda #<(payload_end-payload)
 sta $2e
 lda #>(payload_end-payload)
 sta $2f
 ldz #0
loader_save:
 lda [$20],z
 sta [$24],z
 jsr loader_inc_src
 jsr loader_inc_dst
 lda $2e
 bne loader_count_low
 dec $2f
loader_count_low:
 dec $2e
 lda $2e
 ora $2f
 bne loader_save
 lda #0
 sta $20
 sta $21
 sta $26
 lda #1
 sta $22
 lda #$40
 sta $24
 lda #$20
 sta $25
 lda #2
 sta $2f
loader_token:
 jsr loader_read
 beq loader_done
 bmi loader_match
 sta $2c
loader_literal:
 jsr loader_read
 jsr loader_write
 dec $2c
 bne loader_literal
 bra loader_token
loader_match:
 and #127
 clc
 adc #3
 sta $2c
 jsr loader_read
 sta $2d
 jsr loader_read
 sta $2e
 sec
 lda $24
 sbc $2d
 sta $28
 lda $25
 sbc $2e
 sta $29
 lda #0
 sta $2a
 sta $2b
loader_match_byte:
 lda [$28],z
 jsr loader_write
 inc $28
 bne loader_match_next
 inc $29
loader_match_next:
 dec $2c
 bne loader_match_byte
 bra loader_token
loader_done:
 dec $2f
 beq loader_enter
 lda #0
 sta $24
 lda #$e0
 sta $25
 jmp loader_token
loader_enter:
 jmp $2040
loader_read:
 lda [$20],z
 jsr loader_inc_src
 cmp #0
 rts
loader_write:
 sta [$24],z
loader_inc_dst:
 inc $24
 bne loader_dst_done
 inc $25
loader_dst_done:
 rts
loader_inc_src:
 inc $20
 bne loader_src_done
 inc $21
loader_src_done:
 rts
.cerror * > $0600, "Loader relocation buffer exceeded"
.here
payload:
.binary "packed-game.bin"
payload_end:
.cerror payload_end >= $c000, "Packed file too large for safe BASIC loading"

