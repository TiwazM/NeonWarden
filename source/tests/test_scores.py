from test_machine import *
ntsc='--ntsc' in sys.argv
m=Machine('scores-ntsc' if ntsc else 'scores-pal',ntsc=ntsc,disk=True,sound=True);results=[]
def check(n,c):
 print(n,bool(c),flush=True);results.append(dict(test=n,passed=bool(c)));assert c,n
def word(n):return int.from_bytes(m.read(LABELS[n],2),'little')
def setw(n,v):m.set(n,v&255,v>>8)
def credit(v):
 a=LABELS['credit_amount'];m.write(0x6c80,0xa9,v,0x20,a&255,a>>8,0x60);LABELS['credit_test']=0x6c80;m.call('credit_test')
def table():
 b=m.read(LABELS['score_table'],10);return [int.from_bytes(b[i:i+2],'little') for i in range(0,10,2)]
try:
 m.cmd('t1');check('Disk boots to title',m.v('state')==0)
 check('Only requested timers changed',list(m.read(LABELS['district_seconds'],7))==[60,75,65,60,70,100,80])
 m.set('difficulty',1);m.call('new_game');credit(5)
 check('Pickup earns 50 points, starting cash excluded',word('credit_points')==50 and m.v('credits')==25)
 m.set('selection',0);m.set('event',0);m.call('purchase');check('Shopping does not spend score',word('credit_points')==50 and m.v('credits')==0 and m.v('blade')==1)
 m.set('credits',255);credit(5);check('Full wallet still counts collected credits',m.v('credits')==255 and word('credit_points')==100)
 m.set('stage',2);m.call('load_stage');check('Archive starts at 65 seconds',m.v('time_left')==65)
 m.set('time_left',12);m.call('advance_stage');check('Clear bonus captured before next timer',word('time_points')==60 and m.v('stage')==3 and m.v('time_left')==60)
 m.set('lives',2);m.call('player_dead');check('Retry preserves earned score, no record yet',m.v('state')==3 and word('credit_points')==100 and m.v('score_count')==0)
 m.set('keys',8);m.set('prev',0);m.call('death_tick');check('Retry resets stage timer only',m.v('state')==1 and m.v('time_left')==60 and word('time_points')==60)
 m.set('time_left',1);m.set('time_fraction',1);m.call('countdown_tick');check('Timeout records run once',m.v('state')==5 and table()[0]==160 and m.v('score_count')==1)
 m.call('score_submit');check('Repeated submission ignored',m.v('score_count')==1)
 m.set('keys',8);m.set('prev',8);m.call('gameover_tick');check('Held fire leaves prison visible',m.v('state')==5)
 m.set('prev',0);m.call('gameover_tick');check('Fresh fire opens table',m.v('state')==6)
 m.set('prev',8);m.call('game_tick');check('Held fire does not skip table',m.v('state')==6)
 m.set('prev',0);m.call('game_tick');check('Next fire returns to title',m.v('state')==0)
 m.set('title_choice',3);m.set('keys',8);m.set('prev',0);m.call('intro_input');check('Joystick menu entry opens table',m.v('state')==6)
 m.set('event',27);m.set('keys',0);m.call('game_tick');check('Escape closes table',m.v('state')==0)
 for value in (900,300,700,200,500,500):
  m.call('new_game');setw('credit_points',value);m.call('score_submit')
 check('Top five ordered, ties retained, lowest removed',table()==[900,700,500,500,300] and m.v('score_count')==5)
 m.call('new_game');m.set('stage',5);m.call('load_stage');m.set('time_left',20);setw('credit_points',1000);m.call('advance_stage');check('Final boss exit records bonus before cinematic',m.v('state')==4 and table()[0]==1100)
 m.set('event',ord('R'));m.call('ending_tick');check('Ending R opens scores',m.v('state')==6)
 m.set('difficulty',0);m.call('new_game');credit(10);m.set('time_left',10);m.call('score_clear_bonus');m.set('lives',1);m.call('player_dead');check('Normal does not enter ranking',word('credit_points')==0 and word('time_points')==0 and table()[0]==1100)
 m.set('keys',8);m.set('prev',0);m.call('gameover_tick');check('Normal game-over still returns to title',m.v('state')==0)
 m.set('difficulty',1);m.call('new_game');setw('credit_points',65530);credit(5);check('Credit score saturates safely',word('credit_points')==65535)
 setw('time_points',10);m.call('score_total');check('Total score saturates safely',word('run_score')==65535)
 # Screenshot representative populated score screen, not overflow diagnostic.
 m.call('new_game');setw('credit_points',1050);setw('time_points',425);m.call('score_submit');m.call('score_open');m.call('render');m.call('render')
 check('Decimal rendering and stable row origin',m.read(LABELS['score_digits'],5)==b'00425' and m.v('score_row_y')==60)
finally:
 m.close();(OUT/('test-scores-ntsc.json' if ntsc else 'test-scores-pal.json')).write_text(json.dumps(results,indent=2))
