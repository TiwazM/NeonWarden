from test_machine import *
ntsc='--ntsc' in sys.argv
m=Machine('gear-cheat-ntsc' if ntsc else 'gear-cheat-pal',ntsc=ntsc,disk=True,sound=True);results=[]
def check(n,c):
 print(n,bool(c),flush=True);results.append(dict(test=n,passed=bool(c)));assert c,n
def type_code(s):
 for c in s:
  m.set('event',ord(c));m.set('keys',4 if c.upper()=='W' else 0);m.set('prev',0);m.call('game_tick')
def gear():return tuple(m.v(n) for n in ['blade','armour','gun','ammo'])
def equip():
 for n,v in zip(['blade','armour','gun','ammo'],[3,6,1,17]):m.set(n,v)
def word(n):return int.from_bytes(m.read(LABELS[n],2),'little')
try:
 m.cmd('t1');check('Disk boots, cheat disabled',m.v('state')==0 and m.v('gear_cheat')==0)
 m.set('difficulty',1);choice=m.v('title_choice');muted=m.v('muted')
 type_code('TINWIM207');check('Incomplete code does not enable cheat',m.v('gear_cheat')==0)
 type_code('X');check('Wrong character resets recognition',m.v('cheat_progress')==0)
 type_code('TINWIM2077');check('Exact code enables cheat on title',m.v('gear_cheat')==1 and m.v('state')==0)
 check('Code W and M do not move menu or mute music',m.v('title_choice')==choice and m.v('muted')==muted)
 m.call('new_game');equip();m.set('stage',2);m.call('load_stage');m.set('lives',3);m.call('player_dead');m.set('keys',8);m.set('prev',0);m.call('death_tick')
 check('1991 retry keeps blade, shield, gun and ammo',gear()==(3,6,1,17) and m.v('state')==1)
 check('Lives and district timer retain normal penalty',m.v('lives')==2 and m.v('time_left')==65)
 a=LABELS['credit_amount'];m.write(0x6c80,0xa9,5,0x20,a&255,a>>8,0x60);LABELS['credit_test']=0x6c80;m.call('credit_test');m.call('score_clear_bonus');m.call('score_total')
 check('Cheat credit/time points and total remain zero',word('credit_points')==0 and word('time_points')==0 and word('run_score')==0)
 m.call('score_submit');check('Cheat cannot enter records',m.v('score_count')==0)
 m.set('event',ord('T'));m.set('keys',0);m.call('game_tick');check('Code cannot toggle during gameplay',m.v('gear_cheat')==1 and m.v('cheat_progress')==0)
 m.set('time_left',1);m.set('time_fraction',1);m.call('countdown_tick');check('Cheat does not prevent timeout game over',m.v('state')==5 and m.v('lives')==0)
 m.set('keys',8);m.set('prev',0);m.call('gameover_tick');m.call('render');m.call('render');check('Score screen shows zero and remains unranked',m.v('state')==6 and word('run_score')==0 and m.v('score_count')==0)
 m.set('event',27);m.set('keys',0);m.call('game_tick');type_code('tinwim2077');check('Lowercase code toggles protection off',m.v('gear_cheat')==0)
 m.call('new_game');equip();m.call('difficulty_retry');check('1991 gear loss restored for next run',gear()==(0,0,0,0) and m.v('run_cheat')==0)
 m.call('credit_test');check('Clean runs can score again',word('credit_points')==50)
 m.call('score_submit');check('Clean runs can enter table again',m.v('score_count')==1)
 m.set('state',0);type_code('TINWIM2077');m.set('difficulty',0);m.call('new_game');equip();m.call('difficulty_retry');check('Normal rules unaffected by title toggle',m.v('run_cheat')==0 and gear()==(3,6,1,17))
 m.set('state',0);m.set('difficulty',1);m.set('title_help',0);m.call('render');m.call('render')
finally:
 m.close();(OUT/('test-gear-cheat-ntsc.json' if ntsc else 'test-gear-cheat-pal.json')).write_text(json.dumps(results,indent=2))
