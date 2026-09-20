"""Compile native pixel UI layouts and text."""
from pathlib import Path
import math
P=Path(__file__).resolve().parent
c=[];s=[]
def say(text,x,y,color=7,label=None,big=False):
    if x is None:
        scale=2 if big else 1
        x=(256-(len(text)*6-1)*scale)//2
        # The final period ends two pixels before the full glyph advance.
        if text.endswith('.'):x+=scale
    label=label or f'text_{len(s)}'
    if text is not None:s.append(label+': .text "'+text+'"\n .byte 0')
    c.extend([' jsr ui_text',f' .byte {2 if big else 1},{x},{y},{color}',f' .word {label}'])

def panel(x,y,w,h,color=0):
    c.extend([' jsr ui_panel',f' .byte {x},{y},{w},{h},{color}'])
c+=['draw_ui:',' lda lives',' clc',' adc #48',' sta lives_text',
    ' lda credits',' ldx #48','hundreds:',' cmp #100',' bcc credit_tens',' sec',' sbc #100',' inx',' bne hundreds',
    'credit_tens:',' stx moneytext',' jsr digits',' stx moneytext+1',' sta moneytext+2',
    ' lda blade',' clc',' adc #48',' sta bladetext']
c+=[' lda state',' cmp #2',' beq ui_no_world_labels',' jsr world_labels','ui_no_world_labels:']
c+=[' jsr draw_hearts',' jsr draw_field'];say('CR',78,1,19);say(None,96,1,19,'moneytext')
c+=[' lda #160',' sta tx',' lda #1',' sta ty',' lda #10',' sta ink',' lda #<hud_sword',' sta src',' lda #>hud_sword',' sta src+1',' jsr hud_icon'];say(None,172,1,10,'bladetext');say(None,202,1,7,'lives_caption');say(None,238,1,19,'lives_text')

c+=[' lda ammo',' jsr digits',' stx ammotext',' sta ammotext+1']
c+=[' lda #2',' sta tx',' lda #9',' sta ty',' lda #19',' sta ink',' lda #<hud_bullet',' sta src',' lda #>hud_bullet',' sta src+1',' jsr hud_icon'];say(None,14,9,19,'ammotext')
c+=[' jsr draw_countdown']
c+=[' ldx stage',' lda districtlo,x',' sta txt',' lda districthi,x',' sta txt+1',' lda #82',' sta tx',' lda #9',' sta ty',' lda #5',' sta ink',' jsr draw_text']
c+=[' lda state',' cmp #2',' beq shop_footer']
# Gameplay footer removed: world artwork fills this strip.
c+=[' jmp footer_done','shop_footer:']
say('W/S SELECT FIRE BUY ESC LEAVE',None,185,6)
c+=['footer_done:']
c+=[' lda state',' beq title_ui',' cmp #2',' beq shop_ui',' cmp #3',' beq death_ui',' cmp #4',' beq ending_ui',' lda paused',' bne pause_ui',' lda message_timer',' beq ui_done']
panel(0,18,256%256,22) # rectangle width 0 intentionally denotes 256 pixels
c+=[' ldx message',' lda messagelo,x',' sta txt',' lda messagehi,x',' sta txt+1',' lda messagex,x',' sta tx',' lda #24',' sta ty',' lda #11',' sta ink',' jsr draw_text','ui_done:',' rts','title_ui:']
c+=[' jmp intro_screen']
c+=[' rts','pause_ui:'];panel(48,66,160,53,1);say('PAUSED',92,79,11);say('P TO RESUME',95,101,6);c+=[' rts','shop_ui:']
say('PATCH / MOD CLINIC',None,24,15)
for i,t in enumerate(['1 EDGE UPGRADE 25 CR','2 FORCE FIELD  30 CR','3 FULL REPAIR  10 CR','4 GUN +12 AMMO 40 CR','5 AMMO +8      10 CR']):
    y=100+i*12
    c+=[' lda #48',' sta ox',' lda #0',' sta ox+1',f' lda #{y}',' sta oy',f' lda #<item{i}',' sta src',f' lda #>item{i}',' sta src+1',' jsr draw_sprite']
    say(t,72,y,7)
say('6 EXIT CLINIC',72,160,6)
c+=[' ldx selection',' lda choice_y,x',' sta ty',' lda #34',' sta tx',' lda #19',' sta ink',' lda #<arrow',' sta txt',' lda #>arrow',' sta txt+1',' jsr draw_text']
c+=[' lda message',' cmp #4',' bcc shop_ui_welcome',' cmp #7',' bcc shop_status_ready',' cmp #9',' bne shop_ui_welcome','shop_status_ready:',' tax',' lda messagelo,x',' sta txt',' lda messagehi,x',' sta txt+1',' lda messagex,x',' sta tx',' lda #168',' sta ty',' lda #19',' sta ink',' jsr draw_text',' rts','shop_ui_welcome:']
say('GOOD GEAR. NO QUESTIONS.',None,168,6)
c+=[' rts','choice_y: .byte 100,112,124,136,148,160','death_ui:']
panel(16,48,224,104,1)
say('SIGNAL SEVERED',None,64,15,big=True)
s.append('death_normal_text: .text "YOUR GEAR SURVIVES."\n .byte 0')
c+=[' jsr death_mode_text']
say('FIRE TO RETRY THIS DISTRICT',None,120,19)
c+=[' rts','ending_ui:',' jmp cinematic']
c+=[' rts',
    'digits:',' ldx #48','digit_loop:',' cmp #10',' bcc digit_done',' sec',' sbc #10',' inx',' bne digit_loop','digit_done:',' clc',' adc #48',' rts']
c+=['world_labels:',' lda #1',' sta textscale',' lda stage','world_old_stage:',' cmp #5',' bne outdoor_labels',' jmp arena_labels','outdoor_labels:']
for i,(text,x,y,color) in enumerate([('LINK',52,116,10),('PATCH',245,116,15),('RELAY',949,108,19)]):
    
    if i==0:c += [' lda stage',' bne world_next0']
    label=f'world_label{i}';s.append(label+': .text "'+text+'"\n .byte 0')
    c += [' sec',f' lda #<{x}',' sbc camera',' sta tx',f' lda #>{x}',' sbc camera+1',f' bne world_next{i}',' lda tx',f' cmp #{256-len(text)*6}',f' bcs world_next{i}',f' lda #{y}',' sta ty',f' lda #{color}',' sta ink',f' lda #<{label}',' sta txt',f' lda #>{label}',' sta txt+1',' jsr draw_text',f'world_next{i}:']
c+=[' rts']
c+=['arena_labels:']
say('THE WARDEN',None,30,15)
c+=[' lda relay',' beq arena_sealed']
say('EXIT',220,110,19)
c+=[' rts','arena_sealed:']
say('LOCK',220,110,30)
c+=[' rts']
c+=['shop_room:']
panel(0,0,0,192,1)
panel(0,0,0,16,0)
panel(0,184,0,8,0)
panel(0,176,0,8,2)
panel(16,40,64,40,3)
panel(24,48,48,24,2)
panel(32,48,8,24,10)
panel(48,56,16,8,11)
panel(176,40,64,40,3)
panel(184,48,48,24,2)
panel(192,48,8,24,18)
panel(200,48,8,24,19)
panel(216,56,8,8,10)
c+=[' lda #112',' sta ox',' lda #0',' sta ox+1',' lda #40',' sta oy',' lda #<vendor',' sta src',' lda #>vendor',' sta src+1',' jsr draw_vendor']
panel(64,80,128,8,13)
panel(64,88,128,8,3)
c+=[' rts']
s+=['ammotext: .text "00"\n .byte 0','moneytext: .text "020"\n .byte 0','leveltext: .text "1"\n .byte 0','bladetext: .text "0"\n .byte 0','armourtext: .text "0"\n .byte 0','xptext: .text "0"\n .byte 0','arrow: .text ">"\n .byte 0']
districts=['01 / SUMP MARKET','02 / FLOODWORKS','03 / ARCHIVE SPIRE','04 / CROWN ROOFTOPS','05 / COIL GARDENS','07 / WARDEN CORE','06 / SKYWAY']
messages=['','BREAK THE RELAY.','GEAR: S AT THE PINK DOOR.','DEFEAT THE RELAY GUARD.','PATCH: UPGRADE INSTALLED.','PATCH: NOT ENOUGH CREDIT.','PATCH: ALREADY AT MAXIMUM.','BOSS KEY DROPPED - COLLECT IT.','EXIT: S AT THE GOLD DOOR.','BUY A GUN FIRST.']
for prefix,items in [('district',districts),('message',messages)]:
    for i,t in enumerate(items):s.append(f'{prefix}{i}: .text "{t}"\n .byte 0')
    s+=[prefix+'lo: .byte '+','.join('<'+prefix+str(i) for i in range(len(items))),prefix+'hi: .byte '+','.join('>'+prefix+str(i) for i in range(len(items)))]
s.append('messagex: .byte '+','.join(str((256-(len(t)*6-1))//2) for t in messages))
c+=['intro_menu:',' lda title_help',' bne help_menu']
say('TSW PRESENTS',None,22,6)
say('BREAK THE GRID. FREE THE NAMES.',None,112,6)
panel(32,120,192,24,19)
panel(40,120,176,24,2)
say('ENTER THE CITY',None,125,19,big=True)
say('CONTROLS / HELP',None,148,6)
c+=[' jsr difficulty_menu',' ldx title_choice',' lda intro_choice_y,x',' sta ty',' lda #24',' sta tx',' lda #19',' sta ink',' lda #1',' sta textscale',' lda #<arrow',' sta txt',' lda #>arrow',' sta txt+1',' jsr draw_text']
c+=[' jsr score_menu']
say('W/S SELECT   FIRE CONFIRM',None,176,6)
# Shortcut hints are in the field guide; keep the title footer uncluttered.
c+=[' rts','intro_choice_y: .byte 132,148,159,168','help_menu:']
panel(12,24,232,152,1)
say('COURIER FIELD GUIDE',None,30,11)
for y,t in enumerate(['A/D OR JOYSTICK: MOVE','W / UP: JUMP','SPACE / FIRE: BLADE','G / DOWN+FIRE: PISTOL','S / DOWN: SHOP AND RELAY','SHOP: W/S SELECT / FIRE BUY','P: PAUSE   M: MUTE','HIT 1/2 / LEAP 1 / SHOT 1/3','1991: PITS ARE FATAL','FIELD: ONE EXTRA BLUE HEART']):
    say(t,None,47+y*11,6 if y<7 else 19)
c+=[' jsr intro_sound_status']
say('H / S / FIRE: BACK',None,179,5)
c+=[' rts','intro_sound_status:',' lda fx_mode',' bne intro_sid_status']
say('FX: FM  /  F TO COMPARE',None,159,11)
c+=[' rts','intro_sid_status:']
say('FX: SID /  F TO COMPARE',None,159,19)
c+=[' rts']
split=c.index('intro_sound_status:')
(P/'intro_status.inc').write_text('\n'.join(c[split:])+'\n')
extra=[v for v in s if any(t in v for t in ['ENTER THE CITY','W/S SELECT   FIRE CONFIRM','H HELP   F COMPARE FX','TSW PRESENTS'])]
(P/'intro_strings.inc').write_text('\n'.join(extra)+'\n')
(P/'ui.inc').write_text('\n'.join(c[:split]+[v for v in s if v not in extra])+'\n')
