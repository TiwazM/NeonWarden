; One-way rooftop catwalks. Bounds are world tile columns, right-exclusive.
.include "platforms.inc"
; get_floor leaves Y as the world tile column.
patrol_floor:
 jsr get_floor
 lda stage
 cmp #3
 bcc platform_return
 cmp #5
 bcs platform_return
 jsr platform_range
patrol_platform_loop:
 tya
 cmp platform_left,x
 bcc patrol_platform_next
 cmp platform_right,x
 bcs patrol_platform_next
 lda platform_y,x
 cmp enemyheight
 bne patrol_platform_next
 sta floor_y
 rts
patrol_platform_next:
 dex
 bmi platform_return
 cpx platformindex
 bcs patrol_platform_loop
platform_return:
 rts
landing_platform:
 lda stage
 cmp #3
 bcc platform_return
 cmp #5
 bcs platform_return
 lda playerx
 clc
 adc #8
 sta oldx
 lda playerx+1
 adc #0
 sta oldx+1
 jsr get_floor
 jsr platform_range
landing_loop:
 tya
 cmp platform_left,x
 bcc landing_next
 cmp platform_right,x
 bcs landing_next
 lda oldy
 clc
 adc #24
 cmp platform_y,x
 bcc landing_cross
 bne landing_next
landing_cross:
 lda playery
 clc
 adc #24
 cmp platform_y,x
 bcc landing_next
 lda platform_y,x
 cmp floor_y
 bcs landing_next
 sta floor_y
landing_next:
 dex
 bmi platform_return
 cpx platformindex
 bcs landing_loop
 rts
update_gun:
 lda shotcool
 beq gun_cool_done
 dec shotcool
gun_cool_done:
 lda shotlife
 beq gun_input
 dec shotlife
 lda shotdir
 bne shot_left
 clc
 lda shotx
 adc #6
 sta shotx
 lda shotx+1
 adc #0
 sta shotx+1
 bra shot_terrain
shot_left:
 sec
 lda shotx
 sbc #6
 sta shotx
 lda shotx+1
 sbc #0
 sta shotx+1
shot_terrain:
 lda shotx+1
 cmp #4
 bcs shot_expire
 lda shotx
 sta oldx
 lda shotx+1
 sta oldx+1
 jsr get_floor
 lda shoty
 cmp floor_y
 bcc gun_input
shot_expire:
 lda #0
 sta shotlife
gun_input:
 lda keys
 and #32
 beq gun_done
 lda gun
 beq gun_done
 lda ammo
 beq gun_done
 lda shotcool
 ora shotlife
 bne gun_done
 dec ammo
 lda #12
 sta shotcool
 lda #22
 sta shotlife
 lda facing
 sta shotdir
 lda playerx
 clc
 adc #8
 sta shotx
 lda playerx+1
 adc #0
 sta shotx+1
 lda playery
 clc
 adc #10
 sta shoty
 lda #6
 jsr sfx
gun_done:
 rts
draw_shot:
 lda shotcool
 cmp #8
 bcc draw_projectile
 sec
 lda playerx
 sbc camera
 sta ox
 lda playerx+1
 sbc camera+1
 sta ox+1
 lda playery
 clc
 adc #8
 sta oy
 lda facing
 bne gun_hand_left
 clc
 lda ox
 adc #11
 sta ox
 bcc gun_hand_right_ptr
 inc ox+1
gun_hand_right_ptr:
 lda #<item3
 sta src
 lda #>item3
 sta src+1
 bra gun_hand_draw
gun_hand_left:
 sec
 lda ox
 sbc #3
 sta ox
 bcs gun_hand_left_ptr
 dec ox+1
gun_hand_left_ptr:
 lda #<gun_left
 sta src
 lda #>gun_left
 sta src+1
gun_hand_draw:
 jsr draw_sprite
draw_projectile:
 lda shotlife
 beq gun_done
 sec
 lda shotx
 sbc camera
 sta ox
 lda shotx+1
 sbc camera+1
 sta ox+1
 lda #19
 sta ink
 lda #0
 sta sx
shot_pixel:
 clc
 lda ox
 adc sx
 tax
 lda ox+1
 adc #0
 bne shot_pixel_skip
 ldy shoty
 jsr plot
shot_pixel_skip:
 inc sx
 lda sx
 cmp #4
 bne shot_pixel
 rts

platform_range:
 lda #0
 sta platformindex
 ldx #8
 lda stage
 cmp #4
 bne platform_range_done
 lda #9
 sta platformindex
 ldx #17
platform_range_done:
 rts
