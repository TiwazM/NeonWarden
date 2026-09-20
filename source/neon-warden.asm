; NEON WARDEN - original action RPG prototype for a stock MEGA65.
; 45GS02, 256x192 FCM software framebuffer, pixel-smooth tile scrolling.
; Hardware entry and screen maps adapted from the supplied ION DRIFT reference.
.include "boot.inc"
pix=$20
src=$24
src2=$26
mp=$28
txt=$2c
glyph=$2e
ink=$30
tx=$31
ty=$32
col=$33
fontrow=$34
bits=$35
temp=$36
temp2=$37
obj=$38
frame=$39
back=$3a
state=$3b                 ; title/play/shop/death/ending = 0..4
keys=$3c                  ; left/right/jump/attack/interact = 1/2/4/8/16
prev=$3d
event=$3e
paused=$3f
playerx=$40               ; 16-bit world position (left edge)
playery=$42
velocity=$43              ; signed vertical velocity, pixels per game tick
grounded=$44
facing=$45                ; 0 right, 1 left
health=$46
maxhealth=$47
credits=$48
blade=$49
armour=$4a                ; force-field charge, 0..6 (one extra heart)
level=$4b
lives=$4c
stage=$4d
attack=$4e
cooldown=$4f
invuln=$50
camera=$51                ; 16-bit pixel position
oldx=$53
oldy=$55
floor_y=$56
ox=$57                    ; signed 16-bit sprite screen x
oy=$59
sw=$5a
sh=$5b
sx=$5c
sy=$5d
mapcol=$5e
maprow=$5f
fine=$60
hitmask=$61
distance=$62
message=$63
message_timer=$64
checkpoint=$65
relay=$66
bossphase=$67
soundtime=$68
muted=$69
musicstep=$6a
musicdelay=$6b
tickphase=$6c
videoticks=$6d
stepdue=$6e
difficulty=$ce           ; 0 Normal, 1 1991; preserved across district loads
selection=$6f
textscale=$70
job=$71
pairptr=$75
pairptr2=$77
pairindex=$79
paircount=$7a
rectrows=$7b
audiophase=$7c
patrolx=$7d
enemyheight=$7f
cloudwind=$80
cloudx=$81
cloudy=$82
cloudcol=$83
cloudrow=$84
cloudindex=$85
cloudscroll=$86
cloudwidth=$87
cloudfine=$88
cloudbase=$89
cachebank=$8a
gun=$8b
ammo=$8c
shotx=$8d
shoty=$8f
shotdir=$90
shotlife=$91
shotcool=$92
platformindex=$93
lzcount=$94
packed_in=$95
packed_out=$96
packed_byte_value=$97
hero_pose=$98
hero_last=$99
hero_last_face=$9a
hero_x=$9b
hero_y=$9c
wasgrounded=$9d
landtimer=$9e
fxlife=$9f
fxx=$a0
fxy=$a2
fxkind=$a3
fxradius=$a4
hero_buffer=$6a00
skyclip=$6100
ex=$6000                  ; 8 enemies, separate low/high x arrays
exhi=$6010
ey=$6020
ehp=$6030
etype=$6040
ehit=$6050
edir=$6060
ecool=$6070
home=$6080
homehi=$6090
bulletx=$60a0
bulletxh=$60b0
bullety=$60c0
bulletdir=$60d0
bulletlife=$60e0
.include "hardware.inc"
 ldx #0
 lda #0
init_ram:
 sta $6000,x
 sta $6100,x
 inx
 bne init_ram
init_zp:
 sta $20,x
 inx
 cpx #$e0
 bne init_zp
 lda #4
 sta back
 lda #50
 sta videoticks
 lda $d06f
 and #$80
 beq init_pal
 lda #60
 sta videoticks
init_pal:
 jsr init_actor_cache
 jsr init_heart_cache
 jsr init_background_jobs
 jsr init_audio
 jsr install_audio_irq
 jsr new_game
 lda #0
 sta state
 jmp main

main:
 jsr wait_frame
 jsr read_input
 ; Two raster periods per rendered frame: PAL 25 / NTSC 30.
 ; Skip one simulation update in six on NTSC to retain 25 Hz game time.
 clc
 lda tickphase
 adc #50
 cmp videoticks
 bcc skip_step
 sbc videoticks
 sta tickphase
 inc frame
 lda frame
 and #7
 bne wind_done
 inc cloudwind
wind_done:
 jsr game_tick
 lda keys
 sta prev
 jmp main_render
skip_step:
 sta tickphase
main_render:
 jsr render
 jmp main
wait_frame:
 bit $d011
 bmi wait_frame
wait_edge:
 bit $d011
 bpl wait_edge
 rts

; Hardware raster interrupt: music time is independent of rendering/menu load.
install_audio_irq:
 sei
 lda #<audio_irq
 sta $fffe
 lda #>audio_irq
 sta $ffff
 lda #0
 sta audiophase
 ; Late visible raster avoids the background DMA window in PAL and NTSC.
 ; Line zero is also unsuitable on NTSC cores that start counting at seven.
 lda #192
 sta $d012
 lda $d011
 and #$7f
 sta $d011
 lda #1
 sta $d019
 sta $d01a
 cli
 rts
audio_irq:
 pha
 phx
 phy
 phz
 lda #1
 sta $d019
 jsr tape_raster
 bcc audio_irq_done
 clc
 lda audiophase
 adc #25
 cmp videoticks
 bcc audio_irq_phase
 sbc videoticks
 sta audiophase
 jsr audio_tick
 jmp audio_irq_done
audio_irq_phase:
 sta audiophase
audio_irq_done:
 plz
 ply
 plx
 pla
 rti

new_game:
 jsr score_reset
 lda #3
 sta lives
 lda #0
 sta stage
 sta gun
 sta ammo
 sta blade
 sta armour
 sta paused
 sta musicstep
 lda #1
 sta level
 lda #18
 sta maxhealth
 lda #20
 sta credits
load_stage:
 lda #0
 sta laser_clock
 jsr reset_countdown
 lda #1
 sta state
 lda #0
 sta relay
 sta checkpoint
 sta shotlife
 sta shotcool
 sta velocity
 sta attack
 sta cooldown
 sta bossphase
 sta camera
 sta camera+1
 sta playerx+1
 sta grounded
 lda #40
 sta playerx
 lda #136
 sta playery
 lda maxhealth
 sta health
 lda #255
 sta hero_last
 lda #0
 sta fxlife
 sta landtimer
 lda #50
 sta invuln
 lda #1
 sta message
 lda #110
 sta message_timer
load_old_district:
 ; Decode this district's tilemap to bank 1. No ROM-shadow RAM required.
 ldx stage
 lda maps_lo,x
 sta src
 lda maps_hi,x
 sta src+1
 lda #0
 sta mp
 sta mp+1
 sta mp+3
 lda #1
 sta mp+2
decode_run:
 jsr packed_byte
 beq decode_done
 bmi decode_match
 sta lzcount
literal_byte:
 jsr packed_byte
 jsr unpack_byte
 dec lzcount
 bne literal_byte
 bra decode_run
decode_match:
 and #127
 clc
 adc #3
 sta lzcount
 jsr packed_byte
 sta temp
 jsr packed_byte
 sta temp2
 sec
 lda mp
 sbc temp
 sta pix
 lda mp+1
 sbc temp2
 sta pix+1
 lda #1
 sta pix+2
 lda #0
 sta pix+3
match_byte:
 ldz #0
 lda [pix],z
 jsr unpack_byte
 inc pix
 bne match_no_carry
 inc pix+1
match_no_carry:
 dec lzcount
 bne match_byte
 bra decode_run
packed_byte:
 ldy #0
 lda (src),y
 inc src
 bne packed_done
 inc src+1
packed_done:
 cmp #0
 rts
unpack_byte:
 ldz #0
 sta [mp],z
 inc mp
 bne unpack_done
 inc mp+1
unpack_done:
 rts
decode_done:
 jsr build_cache
 ldx #7
spawn_enemies:
 lda spawnlo,x
 sta ex,x
 sta home,x
 lda spawnhi,x
 sta exhi,x
 sta homehi,x
 lda #0
 sta ehit,x
 sta bulletlife,x
 lda #50
 sta ecool,x
 txa
 and #1
 sta edir,x
 lda kinds,x
 sta etype,x
 lda #4
 clc
 adc stage
 sta ehp,x
 cpx #7
 bne spawn_not_boss
 lda #2
 sta etype,x
 lda #12
 sta ehp,x
 lda stage
 cmp #5
 bne spawn_not_boss
 lda #60
 sta ehp,x
spawn_not_boss:
 stx obj
 clc
 lda ex,x
 adc #8
 sta oldx
 lda exhi,x
 adc #0
 sta oldx+1
 jsr get_floor
 ldx obj
 ldy etype,x
 lda floor_y
 sec
 sbc enemy_heights,y
 sta ey,x
 lda etype,x
 cmp #1
 bne spawn_next
 lda #108
 sta ey,x
spawn_next:
 lda stage
 cmp #3
 bne spawn_height_done
 cpx #5
 bne rooftop_position_done
 lda #192
 sta ex,x
 sta home,x
rooftop_position_done:
 lda rooftop_y,x
 cmp #255
 beq spawn_height_done
 sta ey,x
spawn_height_done:
 lda stage
 cmp #4
 bne fifth_spawn_done
 lda fifth_y,x
 sta ey,x
 cpx #3
 bne fifth_spawn_done
 lda #32
 sta ex,x
 sta home,x
 lda #2
 sta exhi,x
 sta homehi,x
fifth_spawn_done:
 jsr route_spawn
 dex
 bpl spawn_enemies
 jsr arena_setup
 jsr ferry_setup
 rts
spawnlo: .byte <180,<320,<428,<512,<632,<728,<816,<896
spawnhi: .byte >180,>320,>428,>512,>632,>728,>816,>896
kinds: .byte 0,0,1,0,1,0,0,2
fifth_y: .byte 136,104,108,72,48,72,136,132
rooftop_y: .byte 136,104,108,40,108,72,136,132
enemy_heights: .byte 24,24,28
enemy_widths: .byte 16,16,24

read_input:
 lda #0
 sta keys
 lda #1
 sta $d614
 lda $d613
 eor #$ff
 sta temp
 and #4
 beq input_d
 inc keys
input_d:
 lda #2
 sta $d614
 lda $d613
 and #4
 bne input_w
 lda keys
 ora #2
 sta keys
input_w:
 lda temp
 and #2
 beq input_s
 lda keys
 ora #4
 sta keys
input_s:
 lda temp
 and #$20
 beq input_fire
 lda keys
 ora #16
 sta keys
input_fire:
 lda #7
 sta $d614
 lda $d613
 and #16
 bne input_joy
 lda keys
 ora #8
 sta keys
input_joy:
 lda #3
 sta $d614
 lda $d613
 and #4
 bne gun_key_done
 lda keys
 ora #32
 sta keys
gun_key_done:
 lda #4
 sta $d030
 lda #0
 sta $dc02
 lda $dc00
 eor #$ff
 sta temp
 lda #5
 sta $d030
 ldx #4
joy_bits:
 lda temp
 and joymask,x
 beq joy_next
 lda keys
 ora keymask,x
 sta keys
joy_next:
 dex
 bpl joy_bits
 lda state
 cmp #1
 bne gun_chord_done
 lda keys
 and #24
 cmp #24
 bne gun_chord_done
 lda keys
 and #$e7
 ora #32
 sta keys
gun_chord_done:
 lda $d610
 beq input_done
 sta $d610
 sta event
input_done:
 rts
joymask: .byte 4,8,1,16,2
keymask: .byte 1,2,4,8,16

game_tick:
 jmp cheat_dispatch
cheat_dispatch_return:
 lda event
 and #$df
 cmp #$4d
 bne not_mute
 lda muted
 eor #1
 sta muted
not_mute:
 lda event
 and #$df
 cmp #$46
 bne not_audio_mode
 lda fx_mode
 eor #1
 sta fx_mode
 lda #2
 jsr sfx
not_audio_mode:
 jmp score_dispatch
score_dispatch_return:
 lda state
 beq title_tick
 cmp #2
 beq shop_tick
 cmp #3
 beq death_tick
 cmp #4
 beq ending_tick
 cmp #5
 beq gameover_tick
 lda event
 and #$df
 cmp #$50
 bne no_pause
 lda paused
 eor #1
 sta paused
no_pause:
 lda #0
 sta event
 lda paused
 bne tick_done
 jsr countdown_tick
 lda state
 cmp #1
 bne tick_done
 jsr play_tick
tick_done:
 rts
title_tick:
 jmp intro_input
menu_done:
 lda #0
 sta event
 rts
death_tick:
 lda keys
 and #8
 beq menu_done
 lda prev
 and #8
 bne menu_done
 ; Normal retains gear; 1991 loses upgrades but keeps cash and district.
 jsr difficulty_retry
 jsr load_stage
 jmp menu_done
gameover_tick:
 ; No fixed delay: reject a held attack, accept the next fresh press.
gameover_input:
 lda keys
 and #8
 beq menu_done
 lda prev
 and #8
 bne menu_done
 jmp score_run_exit
ending_tick:
 lda cine_time
 cmp #192
 bcs ending_hold
 inc cine_time
ending_hold:
 lda event
 and #$df
 cmp #$52
 bne menu_done
 jmp score_run_exit

shop_tick:
 lda keys
 and #20
 beq shop_choose
 lda prev
 and #20
 bne shop_choose
 lda keys
 and #4
 beq shop_down
 dec selection
 bpl shop_choose
 lda #5
 sta selection
 bne shop_choose
shop_down:
 inc selection
 lda selection
 cmp #6
 bcc shop_choose
 lda #0
 sta selection
shop_choose:
 lda event
 cmp #$31
 bcc shop_fire
 cmp #$37
 bcs shop_fire
 sec
 sbc #$31
 sta selection
 jmp purchase
shop_fire:
 lda keys
 and #8
 beq shop_leave
 lda prev
 and #8
 bne shop_leave
purchase:
 ldx selection
 cpx #5
 beq close_shop
 lda credits
 cmp prices,x
 bcc shop_poor
 cpx #0
 bne buy_armour
 lda blade
 cmp #3
 bcs shop_maxed
 inc blade
 jmp pay
buy_armour:
 cpx #1
 bne buy_heal
 lda armour
 cmp #6
 bcs shop_maxed
 lda #6
 sta armour
 jmp pay
buy_heal:
 cpx #2
 bne buy_gun
 lda health
 cmp maxhealth
 beq shop_maxed
 lda maxhealth
 sta health
 jmp pay
buy_gun:
 cpx #3
 bne buy_ammo
 lda gun
 bne shop_maxed
 inc gun
 lda #12
 sta ammo
 bra pay
buy_ammo:
 lda gun
 bne ammo_owned
 lda #9
 bne shop_set_message
ammo_owned:
 lda ammo
 cmp #24
 bcs shop_maxed
 clc
 adc #8
 cmp #25
 bcc ammo_store
 lda #24
ammo_store:
 sta ammo
pay:
 lda credits
 sec
 sbc prices,x
 sta credits
 lda #4
 sta message
 lda #4
 jsr sfx
 jmp shop_message
shop_maxed:
 lda #6
 bne shop_set_message
shop_poor:
 lda #5
shop_set_message:
 sta message
shop_message:
 lda #80
 sta message_timer
shop_leave:
 lda event
 cmp #27
 bne menu_done
close_shop:
 lda #1
 sta state
 jmp menu_done
prices: .byte 25,30,10,40,10

play_tick:
 jsr ferry_tick
play_old_district:
 jsr visual_tick
 lda message_timer
 beq timer_invuln
 dec message_timer
timer_invuln:
 lda invuln
 beq timer_attack
 dec invuln
timer_attack:
 lda attack
 beq timer_cool
 dec attack
timer_cool:
 lda cooldown
 beq player_move
 dec cooldown
player_move:
 lda playerx
 sta oldx
 lda playerx+1
 sta oldx+1
 lda playery
 sta oldy
 lda keys
 and #1
 beq move_right
 lda #1
 sta facing
 lda playerx+1
 bne left_ok
 lda playerx
 cmp #3
 bcc move_right
left_ok:
 sec
 lda playerx
 sbc #3
 sta playerx
 lda playerx+1
 sbc #0
 sta playerx+1
move_right:
 lda keys
 and #2
 beq move_floor
 lda #0
 sta facing
 lda playerx+1
 cmp #3
 bcc right_ok
 lda playerx
 cmp #237
 bcs move_floor
right_ok:
 clc
 lda playerx
 adc #3
 sta playerx
 lda playerx+1
 adc #0
 sta playerx+1
move_floor:
 jsr arena_bounds
 ; Collision uses the leading foot when walking into a step.
 lda oldx
 pha
 lda oldx+1
 pha
 clc
 lda playerx
 adc #8
 sta oldx
 lda playerx+1
 adc #0
 sta oldx+1
 jsr get_floor
 pla
 sta oldx+1
 pla
 sta oldx
 lda playery
 clc
 adc #23
 cmp floor_y
 bcc try_jump
 lda floor_y
 cmp #192
 beq try_jump
 lda oldx
 sta playerx
 lda oldx+1
 sta playerx+1
 ; Keep the floor from the accepted position.
 clc
 lda playerx
 adc #8
 sta oldx
 lda playerx+1
 adc #0
 sta oldx+1
 jsr get_floor
try_jump:
 lda keys
 and #4
 beq gravity
 lda prev
 and #4
 bne gravity
 lda grounded
 beq gravity
 lda #$f6
 sta velocity
 lda #0
 sta grounded
 lda #1
 jsr sfx
gravity:
 lda velocity
 cmp #10
 beq apply_velocity
 inc velocity
apply_velocity:
 lda playery
 clc
 adc velocity
 sta playery
 lda velocity
 bpl vertical_bounded
 bcc ceiling_clamp
 lda playery
 cmp #16
 bcs vertical_bounded
ceiling_clamp:
 lda #16
 sta playery
 lda #0
 sta velocity
vertical_bounded:
 lda #0
 sta grounded
 lda velocity
 bmi attack_input
 jsr landing_platform
 jsr ferry_landing
 lda playery
 clc
 adc #24
 cmp floor_y
 bcc attack_input
 lda floor_y
 cmp #192
 beq fell
 sec
 sbc #24
 sta playery
 lda #0
 sta velocity
 lda #1
 sta grounded
 jmp attack_input
fell:
 lda playery
 cmp #168
 bcc attack_input
 lda #0
 sta invuln
 lda #3
 jsr pit_damage
 lda state
 cmp #1
 beq pit_survived
 rts
pit_survived:
 lda checkpoint
 beq fall_start
 lda #$18
 sta playerx
 lda #1
 sta playerx+1
 bne fall_place
fall_start:
 lda #40
 sta playerx
 lda #0
 sta playerx+1
fall_place:
 lda #136
 sta playery
 lda #0
 sta velocity
attack_input:
 jsr visual_landing
 lda keys
 and #8
 beq combat_tick
 lda cooldown
 bne combat_tick
 lda #7
 sta attack
 lda #12
 sta cooldown
 lda #0
 sta hitmask
 lda #2
 jsr sfx
combat_tick:
 jsr update_gun
 jsr arena_boss
 jsr update_enemies
 jsr update_bullets
 jsr laser_tick
 lda state
 cmp #1
 beq combat_survived
 rts
combat_survived:
 lda playerx+1
 cmp #1
 bcc interact
 lda #1
 sta checkpoint
interact:
 lda stage
 cmp #5
 beq arena_interact
 lda keys
 and #16
 beq update_camera
 lda prev
 and #16
 bne update_camera
 ; Clinic/vendor occupies world x=232..287.
 lda playerx+1
 beq shop_near_low
 cmp #1
 bne exit_near
 lda playerx
 cmp #32
 bcc open_shop
 bcs exit_near
shop_near_low:
 lda playerx
 cmp #232
 bcs open_shop
 cmp #88
 bcs exit_near
 lda stage
 bne exit_near
 lda #2
 sta message
 lda #120
 sta message_timer
 jmp update_camera
open_shop:
 lda #2
 sta state
 lda #0
 sta selection
 jmp update_camera
exit_near:
 lda playerx+1
 cmp #3
 bne update_camera
 lda playerx
 cmp #176
 bcc update_camera
 lda relay
 bne advance_stage
 lda #3
 sta message
 lda #80
 sta message_timer
 jmp update_camera
advance_stage:
 jsr score_clear_bonus
 lda stage
 cmp #5
 beq win_game
 cmp #6
 bne advance_coil
 lda #5
 sta stage
 jmp tape_load
advance_coil:
 cmp #4
 bne advance_regular
 lda #6
 sta stage
 jsr tape_load
 rts
advance_regular:
 inc stage
 jsr tape_load
 jmp update_camera
win_game:
 jsr score_submit
 jsr tape_unlock
 jsr begin_cinematic
 rts
update_camera:
 lda stage
 cmp #5
 beq arena_camera
 sec
 lda playerx
 sbc #112
 sta camera
 lda playerx+1
 sbc #0
 sta camera+1
 bpl camera_high
 lda #0
 sta camera
 sta camera+1
camera_high:
 lda camera+1
 cmp #3
 bcc camera_done
 lda #3
 sta camera+1
 lda #0
 sta camera
camera_done:
 rts

; oldx / 8 indexes the heightfield for the current district.
get_floor:
 lda oldx
 lsr
 lsr
 lsr
 sta temp
 lda oldx+1
 asl
 asl
 asl
 asl
 asl
 ora temp
 tay
 ldx stage
 lda terrainlo,x
 sta src
 lda terrainhi,x
 sta src+1
 lda (src),y
 sta floor_y
 rts
terrainlo: .byte <terrain,<(terrain+128),<(terrain+256),<(terrain+384),<(terrain+512),<(terrain+640),<(terrain+768)
terrainhi: .byte >terrain,>(terrain+128),>(terrain+256),>(terrain+384),>(terrain+512),>(terrain+640),>(terrain+768)

; Returns saturated absolute world distance to enemy obj, preserves its index.
enemy_distance:
 ldx obj
 sec
 lda ex,x
 sbc playerx
 sta distance
 lda exhi,x
 sbc playerx+1
 bpl distance_positive
 cmp #$ff
 bne distance_far
 lda distance
 eor #$ff
 clc
 adc #1
 sta distance
 bcs distance_far
 rts
distance_positive:
 bne distance_far
 rts
distance_far:
 lda #255
 sta distance
 rts



collect_credit:
 lda ehit,x
 cmp #253
 bcc credit_return
 jsr credit_fall
 ldx obj
 lda ehit,x
 cmp #253
 bcc credit_return
 jsr enemy_distance
 lda distance
 cmp #12
 bcs credit_return
 lda ey,x
 sec
 sbc playery
 clc
 adc #18
 cmp #36
 bcs credit_return
 lda ehit,x
 pha
 lda #0
 sta ehit,x
 pla
 cmp #253
 beq collect_exit_key
 lda #10
 ldy difficulty
 beq credit_amount
 lda #5
credit_amount:
 jsr score_credit
 clc
 adc credits
 bcc credit_store
 lda #255
credit_store:
 sta credits
 lda #9
 jmp sfx
collect_exit_key:
 lda #1
 sta relay
 lda #8
 sta message
 lda #100
 sta message_timer
 lda #9
 jmp sfx
credit_return:
 rts
draw_credit:
 lda ehit,x
 cmp #253
 bcc credit_return
 sec
 lda ex,x
 sbc camera
 sta ox
 lda exhi,x
 sbc camera+1
 bne credit_return
 lda ox
 cmp #248
 bcs credit_return
 lda ey,x
 clc
 adc #14
 sta oy
 lda #19
 sta ink
 ldx obj
 lda ehit,x
 cmp #253
 bne credit_coin_art
 lda ox
 sta tx
 lda oy
 sec
 sbc #3
 sta ty
 lda stage
 cmp #5
 beq draw_warden_card
 lda #<hud_key
 sta src
 lda #>hud_key
 sta src+1
 jmp hud_icon
draw_warden_card:
 ; Same 8x7 footprint, with a breathing cyan rim and white card circuitry.
 dec ty
 lda frame
 and #8
 beq card_dim
 lda #10
 bra card_ink
card_dim:
 lda #5
card_ink:
 sta ink
 lda #<card_rim
 sta src
 lda #>card_rim
 sta src+1
 jsr hud_icon
 lda #7
 sta ink
 lda #<card_detail
 sta src
 lda #>card_detail
 sta src+1
 jmp hud_icon
card_rim: .byte $7e,$81,$81,$81,$81,$81,$7e
card_detail: .byte $00,$00,$5a,$40,$4e,$00,$00
credit_coin_art:
 lda #5
 sta sx
credit_pixels:
 ldx obj
 lda ox
 clc
 adc sx
 tax
 ldy oy
 jsr plot
 iny
 jsr plot
 dec sx
 bne credit_pixels
 rts


; Dead enemy ecool is downward velocity; ey+16 is the pickup bottom.
credit_fall:
lda ecool,x
cmp #$80
bne credit_not_settled
rts
credit_not_settled:
lda etype,x
cmp #1
bne credit_floor_lookup
lda edir,x
bne credit_arc_left
lda exhi,x
cmp #3
bne credit_arc_right
lda ex,x
cmp #248
bcs credit_floor_lookup
credit_arc_right:
inc ex,x
bne credit_floor_lookup
inc exhi,x
bra credit_floor_lookup
credit_arc_left:
lda ex,x
bne credit_arc_decrement
lda exhi,x
beq credit_floor_lookup
dec exhi,x
credit_arc_decrement:
dec ex,x
credit_floor_lookup:
 lda ex,x
 clc
 adc #3
 sta oldx
 lda exhi,x
 adc #0
 sta oldx+1
 lda ey,x
 clc
 adc #16
 sta enemyheight
 jsr get_floor
 lda stage
 cmp #3
 bcc credit_gravity
 cmp #5
 bcs credit_gravity
 jsr platform_range
credit_platform:
 tya
 cmp platform_left,x
 bcc credit_platform_next
 cmp platform_right,x
 bcs credit_platform_next
 lda platform_y,x
 cmp enemyheight
 bcc credit_platform_next
 cmp floor_y
 bcs credit_platform_next
 sta floor_y
credit_platform_next:
 dex
 bmi credit_gravity
 cpx platformindex
 bcs credit_platform
credit_gravity:
 ldx obj
 lda ecool,x
 bmi credit_accelerate
 cmp #4
 bcs credit_velocity
credit_accelerate:
 inc ecool,x
credit_velocity:
 lda ey,x
 clc
 adc ecool,x
 clc
 adc #16
 cmp floor_y
 bcc credit_air
 lda floor_y
 pha
 lda #$80
 sta ecool,x
 pla
credit_air:
 cmp #184
 bcc credit_position
 ; Search leftward for nearby solid ground, wrapping within this district.
 lda ex,x
 sta oldx
 lda exhi,x
 sta oldx+1
credit_rescue_search:
 sec
 lda oldx
 sbc #8
 sta oldx
 lda oldx+1
 sbc #0
 and #3
 sta oldx+1
 jsr get_floor
 lda floor_y
 cmp #184
 bcs credit_rescue_search
 ldx obj
 lda oldx
 sta ex,x
 lda oldx+1
 sta exhi,x
 lda #$80
 sta ecool,x
 lda floor_y
credit_position:
 sec
 sbc #16
 sta ey,x
 rts

; IDs: none, jump, blade, impact, reward, hurt, gun, death, boss volley, credit pling.
fx_duration: .byte 0,4,3,4,9,7,3,18,6,12
fx_freq: .byte 0,90,210,170,100,220,245,230,190,160
fx_block: .byte 0,$12,$0e,$0b,$16,$0e,$12,$0a,$0e,$1c
fx_ratio: .byte 0,2,14,7,3,9,12,6,10,5
fx_mod: .byte 0,28,3,6,24,5,0,10,2,32
fx_sweep: .byte 0,22,232,242,12,240,208,248,238,0
fx_sid_wave: .byte 0,$11,$81,$81,$11,$81,$81,$21,$81,$11

update_enemies:
 lda #0
 sta obj
enemy_loop:
 ldx obj
 lda ehp,x
 bne enemy_alive
 jsr collect_credit
 jmp enemy_next
enemy_alive:
 lda ehit,x
 beq enemy_cool
 dec ehit,x
enemy_cool:
 lda ecool,x
 beq enemy_active
 dec ecool,x
enemy_active:
 lda shotlife
 beq shot_miss
 sec
 lda shotx
 sbc ex,x
 sta temp
 lda shotx+1
 sbc exhi,x
 bne shot_miss
 ldy etype,x
 lda temp
 cmp enemy_widths,y
 bcs shot_miss
 sec
 lda shoty
 sbc ey,x
 bcc shot_miss
 cmp enemy_heights,y
 bcs shot_miss
 lda #0
 sta shotlife
 lda #7
 sta ehit,x
 jsr spawn_hit
 lda ehp,x
 sec
 sbc #6
 bcc enemy_killed
 beq enemy_killed
 sta ehp,x
shot_miss:
 lda stage
 cmp #5
 beq enemy_attack_check
 jsr enemy_distance
 lda distance
 cmp #160
 bcs enemy_next
 ; Boss telegraphs a burst. Drones fire independently.
 lda etype,x
 beq enemy_walk
 lda ecool,x
 bne enemy_walk
 jsr difficulty_fire_delay
 sta ecool,x
 jsr fire_bullet
enemy_walk:
 lda etype,x
 cmp #1
 beq enemy_attack_check
 jsr difficulty_patrol
 beq enemy_attack_check
 jsr ferry_patrol
enemy_attack_check:
enemy_on_floor:
 jsr enemy_distance
 lda attack
 beq enemy_contact
 lda distance
 cmp #34
 bcs enemy_contact
 lda ehit,x
 bne enemy_contact
 ; Swing hits only ahead, and only once per target per swing.
 lda hitmask
 and masks,x
 bne enemy_contact
 lda exhi,x
 cmp playerx+1
 bcc enemy_is_left
 bne enemy_is_right
 lda ex,x
 cmp playerx
 bcc enemy_is_left
enemy_is_right:
 lda facing
 bne enemy_contact
 beq enemy_vertical
enemy_is_left:
 lda facing
 beq enemy_contact
enemy_vertical:
 lda ey,x
 sec
 sbc playery
 bpl enemy_dy
 eor #$ff
 clc
 adc #1
enemy_dy:
 cmp #28
 bcs enemy_contact
 lda hitmask
 ora masks,x
 sta hitmask
 lda #7
 sta ehit,x
 lda blade
 clc
 adc #1
 sta temp
 jsr spawn_hit
 lda ehp,x
 sec
 sbc temp
 bcc enemy_killed
 beq enemy_killed
 sta ehp,x
 lda #3
 jsr sfx
 jmp enemy_contact
enemy_killed:
 jsr spawn_burst
 lda #0
 sta ehp,x
 sta ecool,x
 lda etype,x
 cmp #1
 bne credit_spawn_ready
 lda #$fc
 sta ecool,x
credit_spawn_ready:
 lda #255
 sta ehit,x              ; dead enemy slot now holds an uncollected credit
enemy_reward:
 cpx #7
 bne enemy_reward_sound
 lda #253
 sta ehit,x
 lda #7
 sta message
 lda #100
 sta message_timer
enemy_reward_sound:
 lda #4
 jsr sfx
 jmp enemy_next
enemy_contact:
 lda ehp,x
 beq enemy_next
 lda distance
 cmp #14
 bcs enemy_next
 lda ey,x
 sec
 sbc playery
 bpl contact_dy
 eor #$ff
 clc
 adc #1
contact_dy:
 cmp #22
 bcs enemy_next
 lda stage
 cmp #5
 bne contact_normal
 lda bossphase
 cmp #112
 bcc contact_normal
 cmp #144
 bcs contact_normal
 lda #6
 bra contact_hurt
contact_normal:
 lda #3
contact_hurt:
 jsr hurt_player
enemy_next:
 inc obj
 lda obj
 cmp #8
 bne enemy_loop
 rts

; Six internal units per heart: normal hit 3, leap 6, boss bullet 2.
heart_limit=$cf
heart_color=$d1
heart_empty=$d2
build_hearts:
 lda #14
 sta heart_empty
 lda #3
 sta heart_limit
 lda #15
 sta heart_color
 lda #0
 sta heart_index
heart_next:
 lda heart_index
 asl
 sta temp
 asl
 clc
 adc temp
 sta temp
 lda health
 sec
 sbc temp
 bcs heart_positive
 lda #0
heart_positive:
 cmp #7
 bcc heart_amount
 lda #6
heart_amount:
 sta heart_units
 lda #0
 sta heart_row
heart_rows:
 lda heart_row
 asl
 tax
 lda heart_shape,x
 sta heart_bits
 lda heart_shape+1,x
 sta heart_bits+1
 lda heart_outline,x
 sta heart_edge
 lda heart_outline+1,x
 sta heart_edge+1
 lda #0
 sta heart_col
heart_columns:
 lda heart_bits
 and #1
 beq heart_skip
 lda heart_empty
 sta ink
 lda heart_col
 lsr
 cmp heart_units
 bcc heart_filled
 lda heart_edge
 and #1
 bne heart_pixel
 lda #0
 sta ink
 bra heart_pixel
heart_filled:
 lda heart_color
 sta ink
heart_pixel:
 lda heart_index
 asl
 asl
 asl
 asl
 clc
 adc #2
 adc heart_col
 tax
 lda heart_row
 ina
 tay
 jsr plot
heart_skip:
 lsr heart_edge+1
 ror heart_edge
 lsr heart_bits+1
 ror heart_bits
 inc heart_col
 lda heart_col
 cmp #12
 bne heart_columns
 inc heart_row
 lda heart_row
 cmp #7
 bne heart_rows
 inc heart_index
 lda heart_index
 cmp heart_limit
 bne heart_next
 rts
masks: .byte 1,2,4,8,16,32,64,128

; Patrol between local bounds, turning in place at both walls and ledges.
; Reject a proposed step before committing it; never snap back to home.
patrol_enemy:
 lda ex,x
 sta patrolx
 lda exhi,x
 sta patrolx+1
 lda edir,x
 bne patrol_left
 inc patrolx
 bne patrol_bounds
 inc patrolx+1
 jmp patrol_bounds
patrol_left:
 lda patrolx
 bne patrol_dec
 dec patrolx+1
patrol_dec:
 dec patrolx
patrol_bounds:
 sec
 lda patrolx
 sbc home,x
 sta temp
 lda patrolx+1
 sbc homehi,x
 bpl patrol_positive
 cmp #$ff
 bne patrol_turn
 lda temp
 eor #$ff
 clc
 adc #1
 bcs patrol_turn
 jmp patrol_distance
patrol_positive:
 bne patrol_turn
 lda temp
patrol_distance:
 cmp #41
 bcs patrol_turn
 ldy etype,x
 lda ey,x
 clc
 adc enemy_heights,y
 sta enemyheight
 lda patrolx
 sta oldx
 lda patrolx+1
 sta oldx+1
 jsr patrol_floor
 lda floor_y
 cmp enemyheight
 bne patrol_turn_restore
 ldx obj
 ldy etype,x
 clc
 lda patrolx
 adc enemy_widths,y
 sta oldx
 lda patrolx+1
 adc #0
 sta oldx+1
 lda oldx
 bne patrol_right_dec
 dec oldx+1
patrol_right_dec:
 dec oldx
 jsr patrol_floor
 lda floor_y
 cmp enemyheight
 bne patrol_turn_restore
 ldx obj
 lda patrolx
 sta ex,x
 lda patrolx+1
 sta exhi,x
 rts
patrol_turn_restore:
 ldx obj
patrol_turn:
 lda edir,x
 eor #1
 sta edir,x
 rts

hurt_player:
 sta temp2
 lda invuln
 bne hurt_done
hurt_apply:
 lda state
 cmp #1
 bne hurt_done
 jsr absorb_field
 lda health
 sec
 sbc temp2
 bcc player_dead
 beq player_dead
 sta health
 lda #18
 sta invuln
 lda #5
 jsr sfx
hurt_done:
 rts
player_dead:
 lda state
 cmp #1
 bne hurt_done
 lda #0
 sta health
 sta cine_time
 lda lives
 beq final_life_lost
 dec lives
 beq final_life_lost
 lda #3
 bra life_state
final_life_lost:
 lda #5
life_state:
 sta state
 jsr score_death
 lda #7
 jsr sfx
 rts

fire_bullet:
 lda bulletlife,x
 bne fire_done
 lda #50
 sta bulletlife,x
 lda ex,x
 sta bulletx,x
 lda exhi,x
 sta bulletxh,x
 lda ey,x
 clc
 adc #10
 sta bullety,x
 lda #0
 sta bulletdir,x
 lda exhi,x
 cmp playerx+1
 bcc fire_done
 bne fire_left
 lda ex,x
 cmp playerx
 bcc fire_done
fire_left:
 lda #1
 sta bulletdir,x
fire_done:
 rts
update_bullets:
 ldx #7
bullet_loop:
 lda bulletlife,x
 beq bullet_next
 dec bulletlife,x
 lda bulletdir,x
 beq bullet_right
 sec
 lda bulletx,x
 sbc #4
 sta bulletx,x
 lda bulletxh,x
 sbc #0
 sta bulletxh,x
 jmp bullet_collision
bullet_right:
 clc
 lda bulletx,x
 adc #4
 sta bulletx,x
 lda bulletxh,x
 adc #0
 sta bulletxh,x
bullet_collision:
 sec
 lda bulletx,x
 sbc playerx
 sta temp
 lda bulletxh,x
 sbc playerx+1
 bne bullet_next
 lda temp
 cmp #16
 bcs bullet_next
 lda bullety,x
 sec
 sbc playery
 cmp #24
 bcs bullet_next
 lda #0
 sta bulletlife,x
 lda stage
 cmp #5
 bne bullet_normal_hit
 ; Each unique boss projectile is one third of a heart. No shared hit cooldown
 ; can erase the second/third bullet of the volley; the projectile is consumed.
 lda #2
 sta temp2
 jsr hurt_apply
 bra bullet_next
bullet_normal_hit:
 lda #3
 jsr hurt_player
bullet_next:
 dex
 bpl bullet_loop
 rts

hud_icon:
 lda #0
 sta sy
hud_icon_row:
 ldy sy
 lda (src),y
 sta bits
 lda #0
 sta sx
hud_icon_pixel:
 asl bits
 bcc hud_icon_skip
 lda tx
 clc
 adc sx
 tax
 lda ty
 clc
 adc sy
 tay
 jsr plot
hud_icon_skip:
 inc sx
 lda sx
 cmp #8
 bne hud_icon_pixel
 inc sy
 lda sy
 cmp #7
 bne hud_icon_row
 rts
hud_sword: .byte $06,$0c,$18,$b0,$60,$70,$88
hud_bullet: .byte $18,$3c,$3c,$3c,$3c,$24,$3c
hud_key: .byte $60,$90,$90,$7e,$0a,$00,$00


render:
 jmp score_render_dispatch
score_render_return:
 jsr countdown_border
 lda back
 sta pix+2
 lda #0
 sta pix+3
 lda state
 beq render_intro
 cmp #4
 beq render_cinematic
 cmp #5
 bne render_not_prison
 jsr prison_screen
 jmp render_swap
render_not_prison:
 cmp #2
 beq render_shop
render_old_district:
 jsr draw_background
 jsr draw_clouds
 jsr draw_lasers
 jsr ferry_draw
 jsr draw_actors
 jsr draw_shot
 jsr draw_impacts
 jmp render_ui
render_cinematic:
 jsr cinematic
 jmp render_swap
render_intro:
 jsr intro_screen
 jsr cheat_title_status
 jmp render_swap
render_shop:
 jsr shop_room
render_ui:
 jsr draw_ui
render_swap:
 jsr wait_frame
swap_now:
 lda back
 cmp #4
 bne swap_b
 lda #$70
 sta $d061
 lda #5
 bne swap_done
swap_b:
 lda #$78
 sta $d061
 lda #4
swap_done:
 sta back
 rts

; Build invariant DMA descriptors once. The 768 destinations never move.
init_background_jobs:
 lda #0
 sta pix
 sta pix+1
 sta job
 sta job+3
 sta col
 lda #$10
 sta job+1
 lda #1
 sta job+2
 ldz #0
 lda #$0a
 sta [job],z
 inc job
 lda #3
 sta maprow
init_background_job:
 lda #0
 ldz #11
init_background_zero:
 sta [job],z
 dez
 bpl init_background_zero
 ldz #1
 lda #4
 sta [job],z
 inz
 lda #64
 sta [job],z
 ldz #6
 lda #2
 sta [job],z
 inz
 lda pix
 sta [job],z
 inz
 lda pix+1
 sta [job],z
 inz
 lda #4
 sta [job],z
 clc
 lda pix
 adc #64
 sta pix
 bcc init_background_dest
 inc pix+1
init_background_dest:
 clc
 lda job
 adc #12
 sta job
 bcc init_background_ptr
 inc job+1
init_background_ptr:
 dec col
 bne init_background_job
 dec maprow
 bne init_background_job
 sec
 lda job
 sbc #12
 sta job
 lda job+1
 sbc #0
 sta job+1
 ldz #1
 lda #0
 sta [job],z
 rts

draw_background:
 lda camera
 and #7
 tax
 txa
 asl
 asl
 asl
 asl
 asl
 asl
 sta fine
 lda camera
 lsr
 lsr
 lsr
 sta mapcol
 lda camera+1
 asl
 asl
 asl
 asl
 asl
 ora mapcol
 sta mapcol
 lda #0
 sta pix
 sta pix+1
 sta mp+1
 sta mp+3
 sta maprow
 lda #1
 sta mp+2
 sta job+2
 lda #0
 sta job+3
 lda #$10
 sta job+1
 lda #1
 sta job
 ; Enhanced F018A DMA job list lives at physical $11000.
 lda #$0a
 sta $d702
 lda #0
 sta $d702
background_row:
 lda mapcol
 sta mp
 lda maprow
 lsr
 sta mp+1
 bcc map_even
 lda mp
 ora #$80
 sta mp
map_even:
 lda #0
 sta col
background_tile:
 ldz #0
 lda [mp],z
 asl
 sta temp
 lda #2
 adc #0
 sta cachebank
 ; Only source address and target bank vary per frame.
 ldz #4
 lda fine
 sta [job],z
 inz
 lda camera
 and #4
 beq bg_source_even
 lda temp
 ora #1
 bne bg_source_high
bg_source_even:
 lda temp
bg_source_high:
 sta [job],z
 inz
 lda cachebank
 sta [job],z
 ldz #9
 lda back
 sta [job],z
 clc
 lda job
 adc #12
 sta job
 bcc bg_job_carry
 inc job+1
bg_job_carry:
 inc mp
 inc col
 lda col
 cmp #32
 bne background_tile
 inc maprow
 lda maprow
 cmp #24
 bne background_row
 lda #0
 sta job
 lda #$10
 sta job+1
 lda #$0a
 ldz #0
 sta [job],z
 lda #0
 sta $d704
 lda #1
 sta $d702
 lda #$10
 sta $d701
 lda #0
 sta $d705
 rts

build_cache:
 ; Bank 2 is stock RAM but may be ROM-write-protected after BASIC entry.
 lda #0
 sta pix
 sta pix+1
 sta pix+3
 lda #2
 sta pix+2
 ldz #0
 lda [pix],z
 eor #$ff
 sta [pix],z
 cmp [pix],z
 beq cache_unlocked
 lda #$70
 sta $d640
 nop
cache_unlocked:
 ldx stage
 lda pair_firstlo,x
 sta pairptr
 lda pair_firsthi,x
 sta pairptr+1
 lda pair_secondlo,x
 sta pairptr2
 lda pair_secondhi,x
 sta pairptr2+1
 lda pair_counts,x
 sta paircount
 lda #0
 sta pairindex
cache_pair:
 ldy pairindex
 lda (pairptr),y
 tax
 lda tilelo,x
 sta src
 lda tilehi,x
 sta src+1
 lda #0
 sta src2
 lda #$62
 sta src2+1
 jsr unpack_tile
 ldy pairindex
 lda (pairptr2),y
 tax
 lda tilelo,x
 sta src
 lda tilehi,x
 sta src+1
 lda #$40
 sta src2
 lda #$62
 sta src2+1
 jsr unpack_tile
 lda #0
 sta src
 lda #$62
 sta src+1
 lda #0
 sta fine
cache_phase:
 ldx fine
 lda copylo,x
 sta cache_call+1
 lda copyhi,x
 sta cache_call+2
cache_call:
 jsr copy0
 clc
 lda pix
 adc #64
 sta pix
 bcc cache_next_phase
 inc pix+1
 bne cache_next_phase
 inc pix+2
cache_next_phase:
 inc fine
 lda fine
 cmp #8
 bne cache_phase
 inc pairindex
 lda pairindex
 cmp paircount
 bne cache_pair
 rts

; Wind-blown cloud silhouettes at quarter-camera speed. The skyline mask is
; generated from the actual map so buildings and mounted signs occlude clouds.
draw_clouds:
 lda state
 cmp #1
 beq clouds_live
 rts
clouds_live:
 lda stage
 cmp #5
 bne outdoor_clouds
 rts
outdoor_clouds:
 ldx stage
 lda skylinelo,x
 sta src
 lda skylinehi,x
 sta src+1
 lda camera
 and #7
 sta cloudfine
 ; Cache the visible skyline once, so cloud pixels need only one lookup.
 ldx #0
 ldy mapcol
 lda #8
 sec
 sbc cloudfine
 taz
cloud_clip:
 lda (src),y
cloud_clip_run:
 sta skyclip,x
 inx
 beq cloud_clip_done
 dez
 bne cloud_clip_run
 iny
 ldz #8
 jmp cloud_clip
cloud_clip_done:
 lda camera+1
 sta temp
 lda camera
 lsr temp
 ror
 lsr temp
 ror
 sec
 sbc cloudwind
 sta cloudscroll
 lda #0
 ldx message_timer
 beq clouds_start_index
 lda #1
clouds_start_index:
 sta cloudindex
cloud_next:
 ldx cloudindex
 lda cloud_centres,x
 sec
 sbc cloudscroll
 sta cloudbase
 lda cloud_tops,x
 sta cloudy
 lda #0
 sta cloudrow
cloud_line:
 ldx cloudrow
 lda cloud_widths,x
 sta cloudwidth
 lsr
 sta temp
 lda cloudbase
 sec
 sbc temp
 sta cloudx
 tax
 ldy cloudy
 lda xoffsetlo,x
 clc
 adc yoffsetlo,y
 sta pix
 lda xoffsethi,x
 adc yoffsethi,y
 sta pix+1
 ldz #0
 ldx cloudrow
 lda cloud_inks,x
 sta ink
 ldx cloudx
cloud_pixel:
 lda skyclip,x
 cmp cloudy
 beq cloud_skip
 bcc cloud_skip
 lda ink
 sta [pix],z
cloud_skip:
 inc pix
 bne cloud_ptr_ok
 inc pix+1
cloud_ptr_ok:
 inx
 txa
 and #7
 bne cloud_no_tile
 clc
 lda pix
 adc #56
 sta pix
 bcc cloud_no_carry
 inc pix+1
cloud_no_carry:
 cpx #0
 bne cloud_no_tile
 sec
 lda pix+1
 sbc #8
 sta pix+1
cloud_no_tile:
 dec cloudwidth
 bne cloud_pixel
 inc cloudy
 inc cloudrow
 lda cloudrow
 cmp #8
 bne cloud_line
 inc cloudindex
 lda cloudindex
 cmp #2
 bne cloud_next
 rts
cloud_centres: .byte 64,204
cloud_tops: .byte 27,45
cloud_widths: .byte 36,58,76,88,96,88,68,40
cloud_inks: .byte 34,33,32,32,32,32,33,33


prison_screen:
 jsr ui_panel
 .byte 0,0,0,192,0
 jsr ui_text
 .byte 2,74,10,15
 .word prison_text0
 jsr ui_text
 .byte 1,56,33,6
 .word prison_text2
 jsr prison_panel
 .byte 42,49,172,89,3
 jsr prison_panel
 .byte 46,53,164,79,1
 jsr prison_panel
 .byte 56,62,144,66,0
 jsr prison_panel
 .byte 46,54,164,3,5
 jsr prison_panel
 .byte 42,132,172,7,5
 jsr prison_panel
 .byte 34,139,188,5,2
 jsr prison_panel
 .byte 56,71,32,1,2
 jsr prison_panel
 .byte 172,71,28,1,2
 jsr prison_panel
 .byte 56,91,32,1,2
 jsr prison_panel
 .byte 172,91,28,1,2
 jsr prison_panel
 .byte 56,111,32,1,2
 jsr prison_panel
 .byte 172,111,28,1,2
 jsr prison_panel
 .byte 177,64,16,11,3
 jsr prison_panel
 .byte 181,67,8,2,10
 jsr prison_panel
 .byte 181,71,5,1,14
 jsr prison_panel
 .byte 61,61,13,6,2
 jsr prison_panel
 .byte 65,62,5,2,15
 jsr prison_hero
 jsr prison_panel
 .byte 58,59,3,71,3
 jsr prison_panel
 .byte 59,59,1,71,10
 jsr prison_panel
 .byte 82,59,3,71,3
 jsr prison_panel
 .byte 83,59,1,71,10
 jsr prison_panel
 .byte 106,59,3,71,3
 jsr prison_panel
 .byte 107,59,1,71,10
 jsr prison_panel
 .byte 154,59,3,71,3
 jsr prison_panel
 .byte 155,59,1,71,10
 jsr prison_panel
 .byte 178,59,3,71,3
 jsr prison_panel
 .byte 179,59,1,71,10
 jsr prison_panel
 .byte 198,59,3,71,3
 jsr prison_panel
 .byte 199,59,1,71,10
 jsr prison_panel
 .byte 55,78,146,2,5
 jsr prison_panel
 .byte 55,125,146,3,5
 jsr ui_text
 .byte 1,71,150,6
 .word prison_text4
 jsr prison_reason
 jsr ui_text
 .byte 1,68,179,7
 .word prison_text8
 rts
lives_caption: .text "LIVES"
 .byte 0
lives_text: .text "3"
 .byte 0
prison_text0: .text "GAME OVER"
 .byte 0
prison_text2: .text "NEON DETENTION / CELL 065"
 .byte 0
prison_text4: .text "THE CITY CAUGHT YOU."
 .byte 0
prison_text6: .text "NO LIVES REMAIN"
 .byte 0
prison_text8: .text "FIRE: RETURN TO TITLE"
 .byte 0

; Enlarge the existing courier artwork 2x, preserving the game's palette.
prison_hero:
 lda facing
 pha
 lda #0
 sta facing
 jsr pose_selected
 pla
 sta facing
 lda #<hero_buffer+2
 sta src
 lda #>hero_buffer+2
 sta src+1
 lda #0
 sta sy
prison_hero_row:
 lda #0
 sta sx
prison_hero_pixel:
 ldy #0
 lda (src),y
 beq prison_hero_skip
 sta ink
 lda sx
 asl
 clc
 adc #112
 tax
 lda sy
 asl
 clc
 adc #76
 tay
 jsr plot
 inx
 jsr plot
 iny
 jsr plot
 dex
 jsr plot
prison_hero_skip:
 inc src
 bne prison_hero_ptr
 inc src+1
prison_hero_ptr:
 inc sx
 lda sx
 cmp hero_buffer
 bne prison_hero_pixel
 inc sy
 lda sy
 cmp hero_buffer+1
 bne prison_hero_row
 rts

prison_panel:
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
 lda #0
 sta sy
prison_rect_row:
 lda #0
 sta sx
prison_rect_pixel:
 lda tx
 clc
 adc sx
 tax
 lda ty
 clc
 adc sy
 tay
 jsr plot
 inc sx
 lda sx
 cmp sw
 bne prison_rect_pixel
 inc sy
 lda sy
 cmp sh
 bne prison_rect_row
 rts
plot:
 lda xoffsetlo,x
 clc
 adc yoffsetlo,y
 sta pix
 lda xoffsethi,x
 adc yoffsethi,y
 sta pix+1
 lda ink
 ldz #0
 sta [pix],z
 rts

draw_actors:
 lda #0
 sta obj
draw_enemy_loop:
 ldx obj
 lda ehp,x
 bne draw_living_enemy
 jsr draw_credit
 jmp draw_enemy_next
draw_living_enemy:
 lda ehit,x
 and #1
 bne draw_enemy_next
 sec
 lda ex,x
 sbc camera
 sta ox
 lda exhi,x
 sbc camera+1
 sta ox+1
 lda ey,x
 sta oy
 lda ox+1
 beq enemy_art_ready
 cmp #$ff
 bne draw_enemy_next
 lda ox
 cmp #224
 bcc draw_enemy_next
enemy_art_ready:
 jsr animate_enemy
 jsr draw_sprite
 ; Visible health also communicates the shooter's short charge window.
 ldx obj
 lda ehp,x
 sta sw
 ldy etype,x
 lda enemy_widths,y
 sec
 sbc sw
 cmp #$80
 ror
 sta temp
 clc
 adc ox
 sta ox
 lda temp
 bmi bar_offset_negative
 lda ox+1
 adc #0
 jmp bar_offset_done
bar_offset_negative:
 lda ox+1
 adc #$ff
bar_offset_done:
 sta ox+1
 lda #30
 sta ink
 lda etype,x
 beq enemy_bar_start
 lda ecool,x
 cmp #10
 bcs enemy_bar_start
 lda #19
 sta ink
enemy_bar_start:
 lda #0
 sta sx
enemy_bar:
 lda ox
 clc
 adc sx
 tax
 lda ox+1
 adc #0
 bne enemy_bar_skip
 lda oy
 sec
 sbc #4
 tay
 jsr plot
enemy_bar_skip:
 inc sx
 lda sx
 cmp sw
 bne enemy_bar
draw_enemy_next:
 inc obj
 lda obj
 cmp #8
 bne draw_enemy_loop
 lda invuln
 beq draw_player
 lda frame
 and #2
 bne draw_shots
draw_player:
 sec
 lda playerx
 sbc camera
 sta ox
 lda playerx+1
 sbc camera+1
 sta ox+1
 lda playery
 sta oy
 jsr select_hero_pose
 jsr draw_sprite
 lda attack
 beq draw_shots
 lda playery
 clc
 adc #5
 sta oy
 lda facing
 bne slash_left
 lda ox
 clc
 adc #14
 sta ox
 lda ox+1
 adc #0
 sta ox+1
 jmp slash_start
slash_left:
 sec
 lda ox
 sbc #18
 sta ox
 lda ox+1
 sbc #0
 sta ox+1
slash_start:
 lda #11
 sta ink
 lda #0
 sta sx
slash_loop:
 lda ox
 clc
 adc sx
 tax
 lda ox+1
 adc #0
 bne slash_skip
 stx temp2
 ldx sx
 lda attack
 cmp #4
 bcc arc_follow
 lda arc_upper,x
 bra arc_offset
arc_follow:
 lda arc_lower,x
arc_offset:
 clc
 adc oy
 tay
 ldx temp2
 jsr plot
 iny
 jsr plot
slash_skip:
 inc sx
 lda sx
 cmp #20
 bne slash_loop
draw_shots:
 lda #0
 sta obj
draw_shot_loop:
 ldx obj
 lda bulletlife,x
 beq draw_shot_next
 sec
 lda bulletx,x
 sbc camera
 sta temp
 lda bulletxh,x
 sbc camera+1
 bne draw_shot_next
 lda temp
 cmp #254
 bcs draw_shot_next
 ldy bullety,x
 ldx temp
 lda #15
 sta ink
 jsr plot
 inx
 jsr plot
 iny
 jsr plot
draw_shot_next:
 inc obj
 lda obj
 cmp #8
 bne draw_shot_loop
 rts
sprlo: .byte <guard,<drone,<boss
sprhi: .byte >guard,>drone,>boss
draw_sprite:
 lda ox+1
 beq sprite_visible
 cmp #$ff
 bne sprite_done
 lda ox
 cmp #224
 bcc sprite_done
sprite_visible:
 ldy #0
 lda (src),y
 sta sw
 iny
 lda (src),y
 sta sh
 clc
 lda src
 adc #2
 sta src
 bcc sprite_ptr_ok
 inc src+1
sprite_ptr_ok:
 lda #0
 sta sy
sprite_row:
 lda #0
 sta sx
sprite_col:
 ldy #0
 lda (src),y
 beq sprite_skip
 sta ink
 lda ox
 clc
 adc sx
 tax
 lda ox+1
 adc #0
 bne sprite_skip
 lda oy
 clc
 adc sy
 cmp #184
 bcs sprite_skip
 cmp #16
 bcc sprite_skip
 tay
 jsr plot
sprite_skip:
 inc src
 bne sprite_inc
 inc src+1
sprite_inc:
 inc sx
 lda sx
 cmp sw
 bne sprite_col
 inc sy
 lda sy
 cmp sh
 bne sprite_row
sprite_done:
 rts

draw_text:
 ldy #0
 lda (txt),y
 beq text_done
 sec
 sbc #32
 sta glyph
 lda #0
 sta glyph+1
 asl glyph
 rol glyph+1
 asl glyph
 rol glyph+1
 asl glyph
 rol glyph+1
 clc
 lda glyph
 adc #<font
 sta glyph
 lda glyph+1
 adc #>font
 sta glyph+1
 lda textscale
 cmp #1
 beq text_fast
 lda #0
 sta fontrow
text_row:
 ldy fontrow
 lda (glyph),y
 sta bits
 lda #0
 sta col
text_col:
 lda bits
 and #16
 beq text_skip
 lda tx
 clc
 adc col
 ldx textscale
 cpx #2
 bne text_x_ok
 clc
 adc col
text_x_ok:
 tax
 lda ty
 clc
 adc fontrow
 ldy textscale
 cpy #2
 bne text_y_ok
 clc
 adc fontrow
text_y_ok:
 tay
 jsr plot
 lda textscale
 cmp #2
 bne text_skip
 inx
 jsr plot
 iny
 jsr plot
 dex
 jsr plot
text_skip:
 asl bits
 inc col
 lda col
 cmp #5
 bne text_col
 inc fontrow
 lda fontrow
 cmp #7
 bne text_row
text_next_char:
 inc txt
 bne text_ptr
 inc txt+1
text_ptr:
 clc
 lda tx
 adc #6
 ldx textscale
 cpx #2
 bne text_advance
 clc
 adc #6
text_advance:
 sta tx
 jmp draw_text
text_done:
 rts

; Normal-size UI text: form one long destination pointer per glyph scanline,
; then advance across the tile boundary without recomputing pixel coordinates.
text_fast:
 lda #0
 sta fontrow
text_fast_row:
 ldy fontrow
 lda (glyph),y
 sta bits
 clc
 lda ty
 adc fontrow
 tay
 ldx tx
 lda xoffsetlo,x
 clc
 adc yoffsetlo,y
 sta pix
 lda xoffsethi,x
 adc yoffsethi,y
 sta pix+1
 lda tx
 and #7
 sta temp2
 lda #0
 sta col
 ldz #0
text_fast_col:
 lda bits
 and #16
 beq text_fast_skip
 lda ink
 sta [pix],z
text_fast_skip:
 asl bits
 inz
 inc temp2
 lda temp2
 cmp #8
 bne text_fast_cont
 tza
 clc
 adc #56
 taz
text_fast_cont:
 inc col
 lda col
 cmp #5
 bne text_fast_col
 inc fontrow
 lda fontrow
 cmp #7
 bne text_fast_row
 jmp text_next_char

; Panel fill rounded outward to whole 8x8 tiles, using DMA instead of pixels.
rectangle:
 lda tx
 and #$f8
 tax
 lda ty
 and #$f8
 tay
 lda xoffsetlo,x
 clc
 adc yoffsetlo,y
 sta panel_job+8
 lda xoffsethi,x
 adc yoffsethi,y
 sta panel_job+9
 lda back
 sta panel_job+10
 lda ink
 sta panel_job+5
 lda sh
 clc
 adc #7
 lsr
 lsr
 lsr
 sta rectrows
 lda sw
 beq panel_full
 clc
 adc #7
 lsr
 lsr
 lsr
 jmp panel_width
panel_full:
 lda #32
panel_width:
 sta temp
 lda #0
 sta panel_job+4
 lda temp
 asl
 rol panel_job+4
 asl
 rol panel_job+4
 asl
 rol panel_job+4
 asl
 rol panel_job+4
 asl
 rol panel_job+4
 asl
 rol panel_job+4
 sta panel_job+3
panel_row:
 lda #0
 sta $d702
 sta $d704
 lda #>panel_job
 sta $d701
 lda #<panel_job
 sta $d705
 clc
 lda panel_job+9
 adc #8
 sta panel_job+9
 dec rectrows
 bne panel_row
 rts
panel_job:
 .byte $0a,0,3
 .word 0
 .word 0
 .byte 0
 .word 0
 .byte 4
 .word 0
.include "graphics.inc"
.include "arena.inc"
.include "expansion.inc"
.include "polish.asm"
.include "ui.inc"
.include "cinematic.inc"
.include "lookup.inc"
.include "scroll.inc"
.include "hero.inc"
.include "countdown.inc"
.include "route_spawn.inc"
.include "pit_lasers.inc"
.include "geography_tail.inc"
program_end:
.cerror program_end > $5cf2, "Code overlaps cinematic particles ", format("%x", program_end)
*=$5cf2
.include "cinema_particles.inc"
.cerror * > $5d40, "Cinema particles overlap enemy buffer"
*=$6290
.include "field.inc"
.cerror * > $6300, "Field code overlaps audio"
*=$6300
.include "audio.asm"
.include "heart_cache.inc"
.include "difficulty_ai.inc"
.cerror * > $6700, "Heart cache setup overlaps colour table"
*=$6700
.include "hero_low.inc"
*=$6840
.include "hero_colors.inc"
.cerror * > $6a00, "Colour tables overlap hero buffer"
*=$6950
.include "actor_cache.inc"
.include "intro_status.inc"
.include "cinema_begin.inc"
.cerror * > $6a00, "Actor cache code overlaps hero"
*=$6c80
.include "cinema_sprite.inc"
.cerror * > $6d00, "Cinema sprite overlaps test results"
*=$6d02
.include "pursuit.inc"
.cerror * > $6d40, "Pursuit overlaps presentation"
*=$6d40
.include "presentation.inc"
.cerror * > $7000, "Warden art overlaps screen maps"
*=$8000
.include "geography.inc"
.include "world.inc"
.include "assets.inc"
.include "music.inc"
.include "intro_strings.inc"
.include "cinema_texts.inc"
death_1991_text: .text "UPGRADES LOST."
 .byte 0

asset_end:
.cerror asset_end > $d000, "Assets overlap IO ", format("%x", asset_end), " code ", format("%x",program_end)









*=$e000
.include "ferry_level.inc"
.include "ferry_world.inc"
.include "scores.inc"
.include "cheat.inc"
.include "tape.inc"
sky_code_end:
.cerror sky_code_end > $fc00, "Ferry extension exceeds RAM"

