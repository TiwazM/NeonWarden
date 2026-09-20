; Compact tile and hero storage. Scratch lives outside the active entity arrays.
unpack_tile:
 ldy #0
 lda (src),y
 tax
 iny
unpack_palette:
 lda (src),y
 sta $627f,y
 iny
 dex
 bne unpack_palette
 sty packed_in
 lda #0
 sta packed_out
unpack_tile_byte:
 ldy packed_in
 lda (src),y
 sta packed_byte_value
 inc packed_in
 lsr
 lsr
 lsr
 lsr
 tax
 lda $6280,x
 ldy packed_out
 sta (src2),y
 iny
 sty packed_out
 lda packed_byte_value
 and #15
 tax
 lda $6280,x
 sta (src2),y
 iny
 sty packed_out
 cpy #64
 bne unpack_tile_byte
 rts
select_hero_pose:
 lda #0
 sta hero_pose
 lda attack
 beq pose_gun
 lda #7
 bra pose_selected
pose_gun:
 lda shotcool
 cmp #8
 bcc pose_air
 lda #9
 bra pose_selected
pose_air:
 lda grounded
 bne pose_land
 lda velocity
 bmi pose_up
 lda #6
 bra pose_selected
pose_up:
 lda #5
 bra pose_selected
pose_land:
 lda landtimer
 beq pose_run
 lda #8
 bra pose_selected
pose_run:
 lda keys
 and #3
 beq pose_idle
 lda frame
 lsr
 lsr
 and #3
 ina
 bra pose_selected
pose_idle:
 lda #0
pose_selected:
 sta hero_pose
 cmp hero_last
 bne decode_hero
 lda facing
 cmp hero_last_face
 beq hero_ready
decode_hero:
 lda hero_pose
 sta hero_last
 tax
 lda facing
 sta hero_last_face
 lda hero_pose_lo,x
 sta src
 lda hero_pose_hi,x
 sta src+1
 lda #16
 sta hero_buffer
 lda #24
 sta hero_buffer+1
 lda #<hero_buffer+2
 sta src2
 lda #>hero_buffer+2
 sta src2+1
 lda #0
 sta packed_in
 sta hero_y
hero_unpack_row:
 lda facing
 bne hero_unpack_left
 lda #0
 sta hero_x
hero_unpack_right_pair:
 ldy packed_in
 lda (src),y
 tax
 inc packed_in
 ldy hero_x
 lda hero_high,x
 sta (src2),y
 iny
 lda hero_low,x
 sta (src2),y
 iny
 sty hero_x
 cpy #16
 bne hero_unpack_right_pair
 bra hero_unpack_row_done
hero_unpack_left:
 lda #15
 sta hero_x
hero_unpack_left_pair:
 ldy packed_in
 lda (src),y
 tax
 inc packed_in
 ldy hero_x
 lda hero_high,x
 sta (src2),y
 dey
 lda hero_low,x
 sta (src2),y
 dey
 sty hero_x
 cpy #255
 bne hero_unpack_left_pair
hero_unpack_row_done:
 clc
 lda src2
 adc #16
 sta src2
 bcc hero_unpack_next_row
 inc src2+1
hero_unpack_next_row:
 inc hero_y
 lda hero_y
 cmp #24
 bne hero_unpack_row
hero_ready:
 lda #<hero_buffer
 sta src
 lda #>hero_buffer
 sta src+1
 rts
visual_tick:
 lda grounded
 sta wasgrounded
 lda landtimer
 beq visual_fx_tick
 dec landtimer
visual_fx_tick:
 lda fxlife
 beq visual_done
 dec fxlife
visual_done:
 rts
visual_landing:
 lda wasgrounded
 bne visual_done
 lda grounded
 beq visual_done
 lda #3
 sta landtimer
 rts
spawn_hit:
 lda #4
 sta fxlife
 lda #0
 sta fxkind
 bra fx_position
spawn_burst:
 lda #10
 sta fxlife
 lda #1
 sta fxkind
fx_position:
 clc
 lda ex,x
 adc #8
 sta fxx
 lda exhi,x
 adc #0
 sta fxx+1
 lda ey,x
 clc
 adc #10
 sta fxy
 rts
draw_impacts:
 lda state
 cmp #1
 bne visual_done
 lda fxlife
 beq visual_done
 lda #10
 sec
 sbc fxlife
 lsr
 clc
 adc #2
 sta fxradius
 sec
 lda fxx
 sbc camera
 sta ox
 lda fxx+1
 sbc camera+1
 sta ox+1
 lda fxy
 sta oy
 lda fxkind
 beq impact_gold
 lda #18
 bra impact_colour
impact_gold:
 lda #19
impact_colour:
 sta ink
 lda #0
 sta sx
impact_point:
 ldx sx
 lda fxsigns,x
 bmi impact_left
 clc
 lda ox
 adc fxradius
 tax
 lda ox+1
 adc #0
 bra impact_x_done
impact_left:
 sec
 lda ox
 sbc fxradius
 tax
 lda ox+1
 sbc #0
impact_x_done:
 bne impact_skip
 lda sx
 and #2
 beq impact_above
 lda oy
 clc
 adc fxradius
 bra impact_y_done
impact_above:
 lda oy
 sec
 sbc fxradius
impact_y_done:
 cmp #16
 bcc impact_skip
 cmp #184
 bcs impact_skip
 tay
 jsr plot
impact_skip:
 inc sx
 lda sx
 cmp #4
 bne impact_point
 rts
fxsigns: .byte 0,255,0,255
arc_upper: .byte 4,3,2,1,0,0,0,0,1,1,2,2,3,4,5,6,7,8,9,10
arc_lower: .byte 5,6,7,8,9,10,10,10,10,9,9,8,8,7,6,5,4,3,2,1

