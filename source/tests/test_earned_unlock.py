from test_machine import *
ntsc='--ntsc' in sys.argv
m=Machine('earned-unlock',ntsc=ntsc,disk=True);results=[]
def check(n,c):
 print(n,bool(c),flush=True);results.append(dict(test=n,passed=bool(c)));assert c,n
def code():
 m.set('state',0)
 for c in 'TINWIM2077':m.set('event',ord(c));m.set('keys',0);m.call('game_tick')
try:
 m.cmd('t1');code();check('Gear cheat still activates',m.v('gear_cheat')==1)
 m.set('difficulty',1);m.call('tape_mode_cycle');check('Cheat cannot select locked TAPE',m.v('difficulty')==0 and m.v('tape_unlocked')==0)
 m.set('difficulty',1);m.call('new_game');m.set('blade',3);m.set('gun',1);m.set('ammo',12);m.call('difficulty_retry');check('Cheat still retains equipment',m.v('blade')==3 and m.v('gun')==1 and m.v('ammo')==12)
 m.set('stage',5);m.call('load_stage');m.call('advance_stage');check('Cheat completion cannot unlock TAPE or rank',m.v('tape_unlocked')==0 and m.v('score_count')==0)
 code();m.set('difficulty',1);m.call('new_game');m.set('stage',5);m.call('load_stage');m.call('advance_stage');check('Clean 1991 completion earns unlock',m.v('tape_unlocked')==1)
 m.call('tape_mode_cycle');check('Earned TAPE is selectable',m.v('difficulty')==2)
 code();m.set('difficulty',2);m.call('tape_launch');check('Cheat usable in legitimately unlocked TAPE',m.v('state')==7 and m.v('run_cheat')==1 and m.v('tape_unlocked')==1)
 code();check('Toggling cheat off preserves earned access',m.v('difficulty')==2 and m.v('tape_unlocked')==1)
finally:m.close();(OUT/('test-earned-unlock-ntsc.json' if ntsc else 'test-earned-unlock-pal.json')).write_text(json.dumps(results,indent=2))
