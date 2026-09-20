draw_ui:
 lda lives
 clc
 adc #48
 sta lives_text
 lda credits
 ldx #48
hundreds:
 cmp #100
 bcc credit_tens
 sec
 sbc #100
 inx
 bne hundreds
credit_tens:
 stx moneytext
 jsr digits
 stx moneytext+1
 sta moneytext+2
 lda blade
 clc
 adc #48
 sta bladetext
 lda state
 cmp #2
 beq ui_no_world_labels
 jsr world_labels
ui_no_world_labels:
 jsr draw_hearts
 jsr draw_field
 jsr ui_text
 .byte 1,78,1,19
 .word text_0
 jsr ui_text
 .byte 1,96,1,19
 .word moneytext
 lda #160
 sta tx
 lda #1
 sta ty
 lda #10
 sta ink
 lda #<hud_sword
 sta src
 lda #>hud_sword
 sta src+1
 jsr hud_icon
 jsr ui_text
 .byte 1,172,1,10
 .word bladetext
 jsr ui_text
 .byte 1,202,1,7
 .word lives_caption
 jsr ui_text
 .byte 1,238,1,19
 .word lives_text
 lda ammo
 jsr digits
 stx ammotext
 sta ammotext+1
 lda #2
 sta tx
 lda #9
 sta ty
 lda #19
 sta ink
 lda #<hud_bullet
 sta src
 lda #>hud_bullet
 sta src+1
 jsr hud_icon
 jsr ui_text
 .byte 1,14,9,19
 .word ammotext
 jsr draw_countdown
 ldx stage
 lda districtlo,x
 sta txt
 lda districthi,x
 sta txt+1
 lda #82
 sta tx
 lda #9
 sta ty
 lda #5
 sta ink
 jsr draw_text
 lda state
 cmp #2
 beq shop_footer
 jmp footer_done
shop_footer:
 jsr ui_text
 .byte 1,41,185,6
 .word text_1
footer_done:
 lda state
 beq title_ui
 cmp #2
 beq shop_ui
 cmp #3
 beq death_ui
 cmp #4
 beq ending_ui
 lda paused
 bne pause_ui
 lda message_timer
 beq ui_done
 jsr ui_panel
 .byte 0,18,0,22,0
 ldx message
 lda messagelo,x
 sta txt
 lda messagehi,x
 sta txt+1
 lda messagex,x
 sta tx
 lda #24
 sta ty
 lda #11
 sta ink
 jsr draw_text
ui_done:
 rts
title_ui:
 jmp intro_screen
 rts
pause_ui:
 jsr ui_panel
 .byte 48,66,160,53,1
 jsr ui_text
 .byte 1,92,79,11
 .word text_2
 jsr ui_text
 .byte 1,95,101,6
 .word text_3
 rts
shop_ui:
 jsr ui_text
 .byte 1,74,24,15
 .word text_4
 lda #48
 sta ox
 lda #0
 sta ox+1
 lda #100
 sta oy
 lda #<item0
 sta src
 lda #>item0
 sta src+1
 jsr draw_sprite
 jsr ui_text
 .byte 1,72,100,7
 .word text_5
 lda #48
 sta ox
 lda #0
 sta ox+1
 lda #112
 sta oy
 lda #<item1
 sta src
 lda #>item1
 sta src+1
 jsr draw_sprite
 jsr ui_text
 .byte 1,72,112,7
 .word text_6
 lda #48
 sta ox
 lda #0
 sta ox+1
 lda #124
 sta oy
 lda #<item2
 sta src
 lda #>item2
 sta src+1
 jsr draw_sprite
 jsr ui_text
 .byte 1,72,124,7
 .word text_7
 lda #48
 sta ox
 lda #0
 sta ox+1
 lda #136
 sta oy
 lda #<item3
 sta src
 lda #>item3
 sta src+1
 jsr draw_sprite
 jsr ui_text
 .byte 1,72,136,7
 .word text_8
 lda #48
 sta ox
 lda #0
 sta ox+1
 lda #148
 sta oy
 lda #<item4
 sta src
 lda #>item4
 sta src+1
 jsr draw_sprite
 jsr ui_text
 .byte 1,72,148,7
 .word text_9
 jsr ui_text
 .byte 1,72,160,6
 .word text_10
 ldx selection
 lda choice_y,x
 sta ty
 lda #34
 sta tx
 lda #19
 sta ink
 lda #<arrow
 sta txt
 lda #>arrow
 sta txt+1
 jsr draw_text
 lda message
 cmp #4
 bcc shop_ui_welcome
 cmp #7
 bcc shop_status_ready
 cmp #9
 bne shop_ui_welcome
shop_status_ready:
 tax
 lda messagelo,x
 sta txt
 lda messagehi,x
 sta txt+1
 lda messagex,x
 sta tx
 lda #168
 sta ty
 lda #19
 sta ink
 jsr draw_text
 rts
shop_ui_welcome:
 jsr ui_text
 .byte 1,57,168,6
 .word text_11
 rts
choice_y: .byte 100,112,124,136,148,160
death_ui:
 jsr ui_panel
 .byte 16,48,224,104,1
 jsr ui_text
 .byte 2,45,64,15
 .word text_12
 jsr death_mode_text
 jsr ui_text
 .byte 1,47,120,19
 .word text_14
 rts
ending_ui:
 jmp cinematic
 rts
digits:
 ldx #48
digit_loop:
 cmp #10
 bcc digit_done
 sec
 sbc #10
 inx
 bne digit_loop
digit_done:
 clc
 adc #48
 rts
world_labels:
 lda #1
 sta textscale
 lda stage
world_old_stage:
 cmp #5
 bne outdoor_labels
 jmp arena_labels
outdoor_labels:
 lda stage
 bne world_next0
 sec
 lda #<52
 sbc camera
 sta tx
 lda #>52
 sbc camera+1
 bne world_next0
 lda tx
 cmp #232
 bcs world_next0
 lda #116
 sta ty
 lda #10
 sta ink
 lda #<world_label0
 sta txt
 lda #>world_label0
 sta txt+1
 jsr draw_text
world_next0:
 sec
 lda #<245
 sbc camera
 sta tx
 lda #>245
 sbc camera+1
 bne world_next1
 lda tx
 cmp #226
 bcs world_next1
 lda #116
 sta ty
 lda #15
 sta ink
 lda #<world_label1
 sta txt
 lda #>world_label1
 sta txt+1
 jsr draw_text
world_next1:
 sec
 lda #<949
 sbc camera
 sta tx
 lda #>949
 sbc camera+1
 bne world_next2
 lda tx
 cmp #226
 bcs world_next2
 lda #108
 sta ty
 lda #19
 sta ink
 lda #<world_label2
 sta txt
 lda #>world_label2
 sta txt+1
 jsr draw_text
world_next2:
 rts
arena_labels:
 jsr ui_text
 .byte 1,98,30,15
 .word text_18
 lda relay
 beq arena_sealed
 jsr ui_text
 .byte 1,220,110,19
 .word text_19
 rts
arena_sealed:
 jsr ui_text
 .byte 1,220,110,30
 .word text_20
 rts
shop_room:
 jsr ui_panel
 .byte 0,0,0,192,1
 jsr ui_panel
 .byte 0,0,0,16,0
 jsr ui_panel
 .byte 0,184,0,8,0
 jsr ui_panel
 .byte 0,176,0,8,2
 jsr ui_panel
 .byte 16,40,64,40,3
 jsr ui_panel
 .byte 24,48,48,24,2
 jsr ui_panel
 .byte 32,48,8,24,10
 jsr ui_panel
 .byte 48,56,16,8,11
 jsr ui_panel
 .byte 176,40,64,40,3
 jsr ui_panel
 .byte 184,48,48,24,2
 jsr ui_panel
 .byte 192,48,8,24,18
 jsr ui_panel
 .byte 200,48,8,24,19
 jsr ui_panel
 .byte 216,56,8,8,10
 lda #112
 sta ox
 lda #0
 sta ox+1
 lda #40
 sta oy
 lda #<vendor
 sta src
 lda #>vendor
 sta src+1
 jsr draw_vendor
 jsr ui_panel
 .byte 64,80,128,8,13
 jsr ui_panel
 .byte 64,88,128,8,3
 rts
intro_menu:
 lda title_help
 bne help_menu
 jsr ui_text
 .byte 1,92,22,6
 .word text_50
 jsr ui_text
 .byte 1,36,112,6
 .word text_51
 jsr ui_panel
 .byte 32,120,192,24,19
 jsr ui_panel
 .byte 40,120,176,24,2
 jsr ui_text
 .byte 2,45,125,19
 .word text_52
 jsr ui_text
 .byte 1,83,148,6
 .word text_53
 jsr difficulty_menu
 ldx title_choice
 lda intro_choice_y,x
 sta ty
 lda #24
 sta tx
 lda #19
 sta ink
 lda #1
 sta textscale
 lda #<arrow
 sta txt
 lda #>arrow
 sta txt+1
 jsr draw_text
 jsr score_menu
 jsr ui_text
 .byte 1,53,176,6
 .word text_54
 rts
intro_choice_y: .byte 132,148,159,168
help_menu:
 jsr ui_panel
 .byte 12,24,232,152,1
 jsr ui_text
 .byte 1,71,30,11
 .word text_55
 jsr ui_text
 .byte 1,65,47,6
 .word text_56
 jsr ui_text
 .byte 1,92,58,6
 .word text_57
 jsr ui_text
 .byte 1,71,69,6
 .word text_58
 jsr ui_text
 .byte 1,65,80,6
 .word text_59
 jsr ui_text
 .byte 1,56,91,6
 .word text_60
 jsr ui_text
 .byte 1,47,102,6
 .word text_61
 jsr ui_text
 .byte 1,74,113,6
 .word text_62
 jsr ui_text
 .byte 1,47,124,19
 .word text_63
 jsr ui_text
 .byte 1,68,135,19
 .word text_64
 jsr ui_text
 .byte 1,47,146,19
 .word text_65
 jsr intro_sound_status
 jsr ui_text
 .byte 1,74,179,5
 .word text_66
 rts
text_0: .text "CR"
 .byte 0
text_1: .text "W/S SELECT FIRE BUY ESC LEAVE"
 .byte 0
text_2: .text "PAUSED"
 .byte 0
text_3: .text "P TO RESUME"
 .byte 0
text_4: .text "PATCH / MOD CLINIC"
 .byte 0
text_5: .text "1 EDGE UPGRADE 25 CR"
 .byte 0
text_6: .text "2 FORCE FIELD  30 CR"
 .byte 0
text_7: .text "3 FULL REPAIR  10 CR"
 .byte 0
text_8: .text "4 GUN +12 AMMO 40 CR"
 .byte 0
text_9: .text "5 AMMO +8      10 CR"
 .byte 0
text_10: .text "6 EXIT CLINIC"
 .byte 0
text_11: .text "GOOD GEAR. NO QUESTIONS."
 .byte 0
text_12: .text "SIGNAL SEVERED"
 .byte 0
death_normal_text: .text "YOUR GEAR SURVIVES."
 .byte 0
text_14: .text "FIRE TO RETRY THIS DISTRICT"
 .byte 0
world_label0: .text "LINK"
 .byte 0
world_label1: .text "PATCH"
 .byte 0
world_label2: .text "RELAY"
 .byte 0
text_18: .text "THE WARDEN"
 .byte 0
text_19: .text "EXIT"
 .byte 0
text_20: .text "LOCK"
 .byte 0
ammotext: .text "00"
 .byte 0
moneytext: .text "020"
 .byte 0
leveltext: .text "1"
 .byte 0
bladetext: .text "0"
 .byte 0
armourtext: .text "0"
 .byte 0
xptext: .text "0"
 .byte 0
arrow: .text ">"
 .byte 0
district0: .text "01 / SUMP MARKET"
 .byte 0
district1: .text "02 / FLOODWORKS"
 .byte 0
district2: .text "03 / ARCHIVE SPIRE"
 .byte 0
district3: .text "04 / CROWN ROOFTOPS"
 .byte 0
district4: .text "05 / COIL GARDENS"
 .byte 0
district5: .text "07 / WARDEN CORE"
 .byte 0
district6: .text "06 / SKYWAY"
 .byte 0
districtlo: .byte <district0,<district1,<district2,<district3,<district4,<district5,<district6
districthi: .byte >district0,>district1,>district2,>district3,>district4,>district5,>district6
message0: .text ""
 .byte 0
message1: .text "BREAK THE RELAY."
 .byte 0
message2: .text "GEAR: S AT THE PINK DOOR."
 .byte 0
message3: .text "DEFEAT THE RELAY GUARD."
 .byte 0
message4: .text "PATCH: UPGRADE INSTALLED."
 .byte 0
message5: .text "PATCH: NOT ENOUGH CREDIT."
 .byte 0
message6: .text "PATCH: ALREADY AT MAXIMUM."
 .byte 0
message7: .text "BOSS KEY DROPPED - COLLECT IT."
 .byte 0
message8: .text "EXIT: S AT THE GOLD DOOR."
 .byte 0
message9: .text "BUY A GUN FIRST."
 .byte 0
messagelo: .byte <message0,<message1,<message2,<message3,<message4,<message5,<message6,<message7,<message8,<message9
messagehi: .byte >message0,>message1,>message2,>message3,>message4,>message5,>message6,>message7,>message8,>message9
messagex: .byte 128,80,53,59,53,53,50,38,53,80
text_51: .text "BREAK THE GRID. FREE THE NAMES."
 .byte 0
text_53: .text "CONTROLS / HELP"
 .byte 0
text_55: .text "COURIER FIELD GUIDE"
 .byte 0
text_56: .text "A/D OR JOYSTICK: MOVE"
 .byte 0
text_57: .text "W / UP: JUMP"
 .byte 0
text_58: .text "SPACE / FIRE: BLADE"
 .byte 0
text_59: .text "G / DOWN+FIRE: PISTOL"
 .byte 0
text_60: .text "S / DOWN: SHOP AND RELAY"
 .byte 0
text_61: .text "SHOP: W/S SELECT / FIRE BUY"
 .byte 0
text_62: .text "P: PAUSE   M: MUTE"
 .byte 0
text_63: .text "HIT 1/2 / LEAP 1 / SHOT 1/3"
 .byte 0
text_64: .text "1991: PITS ARE FATAL"
 .byte 0
text_65: .text "FIELD: ONE EXTRA BLUE HEART"
 .byte 0
text_66: .text "H / S / FIRE: BACK"
 .byte 0
text_67: .text "FX: FM  /  F TO COMPARE"
 .byte 0
text_68: .text "FX: SID /  F TO COMPARE"
 .byte 0
