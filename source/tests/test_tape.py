from test_machine import *
ntsc='--ntsc' in sys.argv
m=Machine('tape-ntsc' if ntsc else 'tape-pal',ntsc=ntsc,disk=True,sound=True);results=[]
def check(n,c):
 print(n,bool(c),flush=True);results.append(dict(test=n,passed=bool(c)));assert c,n
def word(n):return int.from_bytes(m.read(LABELS[n],2),'little')
def steps(n):
 m.set('keys',0);m.set('event',0);m.write(0x6c30,n);a=LABELS['game_tick'];m.write(0x6c40,0x20,a&255,a>>8,0xce,0x30,0x6c,0xd0,0xf8,0x4c,0x48,0x6c);m.cmd('g6c40');m.cmd('t0');time.sleep(.15);m.cmd('t1');assert m.read(0x6c30)==b'\0'
def equip():
 for k,v in [('blade',3),('armour',6),('gun',1),('ammo',17),('credits',123),('lives',2)]:m.set(k,v)
def gear():return tuple(m.v(k) for k in ['blade','armour','gun','ammo','credits','lives'])
try:
 m.cmd('t1');check('Disk boots with secret mode locked',m.v('state')==0 and m.v('tape_unlocked')==0)
 for expected in [1,0,1,0]:m.call('tape_mode_cycle');check('Locked selector retains two modes',m.v('difficulty')==expected)
 for c in 'TINWIM2077':m.set('event',ord(c));m.set('keys',0);m.call('game_tick')
 m.set('difficulty',1);m.call('tape_mode_cycle');check('Cheat cannot expose TAPE mode',m.v('difficulty')==0);m.set('difficulty',2) # Test fixture only: exercise unranked-win rejection.
 m.call('new_game');check('Cheat tape run remains unranked',m.v('run_cheat')==1)
 m.set('stage',5);m.call('load_stage');m.call('advance_stage');check('Cheat win does not earn normal unlock',m.v('tape_unlocked')==0)
 m.set('gear_cheat',0);m.set('difficulty',0);m.call('new_game');m.set('stage',5);m.call('load_stage');m.call('advance_stage');check('Normal win does not unlock',m.v('tape_unlocked')==0)
 m.set('difficulty',1);m.call('new_game');m.set('stage',5);m.call('load_stage');m.call('advance_stage');check('Clean 1991 win unlocks tape mode',m.v('tape_unlocked')==1 and m.v('state')==4)
 m.call('tape_mode_cycle');check('Earned unlock makes third mode selectable',m.v('difficulty')==2)
 m.set('state',0);m.set('title_choice',0);m.set('keys',8);m.set('prev',0);m.set('event',0);m.call('intro_input');check('Title launch loads district 1 through tape sequence',m.v('state')==7 and m.v('stage')==0 and m.v('tape_count')==1)
 m.set('tape_joke',0);steps(114);check('First district clock stays paused',m.v('state')==7 and m.v('time_left')==60);steps(1);check('First district starts with full timer',m.v('state')==1 and m.v('time_left')==60)
 m.call('new_game');m.set('tape_joke_load',3);equip()
 for i,(old,new,seconds,digit) in enumerate([(0,1,75,'2'),(1,2,65,'3'),(2,3,60,'4'),(3,4,70,'5'),(4,6,80,'6'),(6,5,100,'7')],1):
  m.set('stage',old);m.call('load_stage');m.set('time_left',19);before=gear();m.call('advance_stage')
  check(f'Transition {i}: enters fake loader for district {digit}',m.v('state')==7 and m.v('stage')==new and m.v('tape_district_digit')==ord(digit))
  steps(84);check(f'Transition {i}: timer/gear frozen',m.v('time_left')==19 and gear()==before and m.v('tape_clock')==84)
  steps(1);m.call('tape_visual_phase')
  check(f'Transition {i}: correct joke selection',m.v('tape_joke')==(i==3) and (m.v('tape_phase')==5 if i==3 else m.v('tape_phase')==3))
  if i==3:
   steps(12);m.call('tape_visual_phase');check('Stall reveals JUST KIDDING',m.v('tape_phase')==6)
   steps(42)
  else:steps(29)
  check(f'Transition {i}: no early exit',m.v('state')==7 and m.v('time_left')==19)
  steps(1);check(f'Transition {i}: safe exit with full timer',m.v('state')==1 and m.v('time_left')==seconds and gear()==before)
 check('Exactly one joke in six transitions',m.v('tape_count')==6)
 for mode in (0,1):
  m.set('difficulty',mode);m.call('new_game')
  for old,new in [(0,1),(1,2),(2,3),(3,4),(4,6),(6,5)]:
   m.set('stage',old);m.call('load_stage');m.call('advance_stage');check(f'Existing mode {mode}: {old}->{new} remains immediate',m.v('state')==1 and m.v('stage')==new and m.v('tape_count')==0)
 # Real main-loop wall clock, with all raster bands and SID audio running.
 for joke in (False,True):
  m.set('difficulty',2);m.call('new_game');m.set('tape_joke_load',1 if joke else 6);m.call('advance_stage');m.set('invuln',250);m.set('keys',0);m.set('prev',0);m.set('event',0)
  start=time.monotonic();m.resume()
  while m.v('state')==7 and time.monotonic()-start<7:time.sleep(.03)
  elapsed=time.monotonic()-start;m.cmd('t1');results.append(dict(test=f'Real-time loader joke={joke}: {elapsed:.2f}s',passed=(5.2<elapsed<6.1 if joke else 4.2<elapsed<5.1)));print(results[-1],flush=True);assert results[-1]['passed']
  check('Audio IRQ returns to single raster after load',m.v('tape_irq_line')==192)
  check('Screen-edge tile restored after loader',m.read(0x6800,64)==bytes(64))
 # Capture the joke screen with actual animated border/SID playback.
 m.set('difficulty',2);m.call('new_game');m.set('tape_joke_load',1);m.call('advance_stage');steps(97);m.call('render');m.call('render')
 check('Tape render returns and shows joke',m.v('state')==7 and m.v('tape_phase')==6)
 # Exercise a nonblack band and confirm every edge pixel receives its colour.
 m.set('tape_irq_line',32);m.set('tape_clock',1);m.write(0xffd301a,0);m.call('tape_raster');check('Former black edge tile follows border band',m.read(0x6800,64)==bytes([19])*64)
finally:
 m.close();(OUT/('test-tape-ntsc.json' if ntsc else 'test-tape-pal.json')).write_text(json.dumps(results,indent=2))
