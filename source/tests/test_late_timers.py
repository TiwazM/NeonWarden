from test_machine import *
ntsc='--ntsc' in sys.argv
m=Machine('late-timers-ntsc' if ntsc else 'late-timers-pal',ntsc=ntsc,disk=True);results=[]
def check(n,c):
 print(n,bool(c),flush=True);results.append(dict(test=n,passed=bool(c)));assert c,n
try:
 m.cmd('t1');check('Disk boots',m.v('state')==0)
 check('Timer table matches play-order intent',list(m.read(LABELS['district_seconds'],7))==[60,75,65,60,70,100,80])
 m.set('difficulty',1);m.call('new_game')
 for stage,seconds in [(4,70),(6,80),(5,100)]:
  m.set('stage',stage);m.call('load_stage');check(f'Stage index {stage} starts with {seconds}s',m.v('time_left')==seconds)
  m.set('time_fraction',1);m.call('countdown_tick');check('Countdown still decrements',m.v('time_left')==seconds-1)
  m.call('load_stage');check('Retry restores revised limit',m.v('time_left')==seconds)
 m.set('difficulty',0);m.set('time_fraction',1);m.call('countdown_tick');check('Normal remains untimed',m.v('time_left')==100)
finally:
 m.close();(OUT/('test-late-timers-ntsc.json' if ntsc else 'test-late-timers-pal.json')).write_text(json.dumps(results,indent=2))
