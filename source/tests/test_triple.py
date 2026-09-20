from test_machine import *
ntsc='--ntsc' in sys.argv
m=Machine('triple-ntsc' if ntsc else 'triple-pal',ntsc=ntsc,disk=True,sound=True);results=[]
def check(n,c):
 print(n,bool(c),flush=True);results.append(dict(test=n,passed=bool(c)));assert c,n
def steps(n,keys=0):
 m.set('keys',keys);m.set('event',0);m.write(0x6c30,n);a=LABELS['game_tick'];m.write(0x6c40,0x20,a&255,a>>8,0xa5,LABELS['keys'],0x85,LABELS['prev'],0xe6,LABELS['frame'],0xce,0x30,0x6c,0xd0,0xf2,0x4c,0x4e,0x6c);m.cmd('g6c40');m.cmd('t0');time.sleep(.15);m.cmd('t1');assert m.read(0x6c30)==b'\0';m.call('render')
def ferry(i):return m.read(LABELS['ferry_xlo']+i)[0]+256*m.read(LABELS['ferry_xhi']+i)[0]
def word(n):return int.from_bytes(m.read(LABELS[n],2),'little')
def place(x,y=120,ride=1):m.set('playerx',x&255,x>>8);m.set('playery',y);m.set('grounded',1);m.set('ferry_ride',ride);m.set('velocity',0);m.set('prev',0)
try:
 m.cmd('t1');check('Disk boots',m.v('state')==0);m.call('new_game')
 for mode in (0,1):
  m.set('difficulty',mode);m.set('stage',6);m.call('load_stage');m.set('ferry_phase',100);m.call('ferry_positions');place(584,136);m.set('invuln',250)
  steps(1,6);steps(9,2);steps(12)
  check(f'{mode}: board triple crossing from solid pier',m.v('ferry_ride')==1 and m.v('grounded')==1)
  # Verify real enemy update cannot let the riding guard walk into the pit.
  steps(35)
  check(f'{mode}: middle guard rides its platform',m.read(LABELS['ex']+6)[0]+256*m.read(LABELS['exhi']+6)[0]==ferry(2)+40 and m.read(LABELS['ey']+6)==bytes([120]) and m.read(LABELS['ehp']+6)==bytes([6]))
  # Verify transfers over several representative phases with collision/render active.
  for phase in (0,50,100,150):
   for i in (1,2):
    m.set('ferry_phase',phase);m.call('ferry_positions');place(ferry(i)+48,120,i);m.write(LABELS['ehp']+6,0);m.write(LABELS['ehit']+6,0);steps(1,6);steps(18,2);steps(2)
    check(f'{mode}: transfer {i} to {i+1} at phase {phase}',m.v('grounded')==1 and m.v('ferry_ride')==i+1)
  m.set('ferry_phase',0);m.call('ferry_positions');place(ferry(3)+48,120,3);steps(1,6);steps(22,2)
  check(f'{mode}: final jump reaches solid far bank',word('playerx')+8>=896 and m.v('grounded')==1 and m.v('playery')==136)
  # Sword kills the deck guard; credit then stays attached until collected.
  m.write(LABELS['ehp']+6,6);m.write(LABELS['ehit']+6,0);m.set('blade',3);place(ferry(2)+12,120,2);m.set('facing',0);m.set('cooldown',0);steps(1,8);steps(12);steps(1,8)
  check(f'{mode}: guard can be defeated by sword',m.read(LABELS['ehp']+6)==b'\0')
  steps(2);check(f'{mode}: credit rests on moving deck',m.read(LABELS['ey']+6)==bytes([128]) and m.read(LABELS['ecool']+6)==bytes([128]))
  cash=m.v('credits');place(ferry(2)+40,120,2);steps(1);check(f'{mode}: deck credit collects normally',m.v('credits')==cash+(5 if mode else 10))
 for difficulty in (0,1):
  m.set('difficulty',difficulty);m.set('state',1);m.set('paused',0)
  for seconds in (11,10,6,5,1,0):
   values=[]
   for frame in (0,8,16,24):
    m.set('time_left',seconds);m.set('frame',frame);m.call('countdown_border');values.append(m.read(0xffd3020)[0])
   expected=[0,0,0,0] if not difficulty or seconds in (0,11) else [0,0,30,30] if seconds>5 else [0,30,0,30]
   check(f'Border mode {difficulty}, {seconds}s',values==expected)
 m.set('difficulty',1);m.set('time_left',3);m.set('frame',24)
 for state,paused in [(1,1),(2,0),(3,0),(0,0)]:
  m.set('state',state);m.set('paused',paused);m.call('countdown_border');check('Menus/pause clear warning',m.read(0xffd3020)==b'\0')
 m.set('state',1);m.set('paused',0);m.set('time_left',90);m.set('stage',6);m.call('load_stage');place(690,120,2);m.call('update_camera');m.set('message_timer',0);m.call('render');m.call('render')
finally:m.close();(OUT/('test-triple-ntsc.json' if ntsc else 'test-triple-pal.json')).write_text(json.dumps(results,indent=2))

