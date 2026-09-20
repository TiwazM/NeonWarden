from test_machine import *
import array
peaks=[]
for muted in (1,0):
 name='tape-muted' if muted else 'tape-audible';m=Machine(name,sound=True)
 try:
  m.cmd('t1');m.set('difficulty',2);m.call('new_game');m.set('tape_joke_load',6);m.call('advance_stage');m.set('tape_clock',26);m.set('muted',muted);m.call('render');m.call('render');m.cmd('t0');time.sleep(1)
 finally:m.close()
 b=(WORK/(name+'.raw')).read_bytes()[-32768:];a=array.array('h');a.frombytes(b[:len(b)//2*2]);peaks.append(max(abs(v) for v in a))
results=[dict(test='Tape audio audible, mute quiet',quiet_peak=peaks[0],tape_peak=peaks[1],passed=peaks[1]>100 and peaks[0]<10)]
(OUT/'test-tape-audio.json').write_text(json.dumps(results,indent=2));print(results);assert results[0]['passed']

