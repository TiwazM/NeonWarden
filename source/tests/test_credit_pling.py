from test_machine import *
from array import array
import statistics
results=[]
def check(n,c):
 print(n,bool(c),flush=True);results.append(dict(test=n,passed=bool(c)));assert c,n
for ntsc in (False,True):
 name='pling-ntsc' if ntsc else 'pling-pal';m=Machine(name,ntsc=ntsc,disk=True,sound=True);spans=[]
 try:
  m.cmd('t1');m.call('new_game');m.set('audio_gain',0);m.set('audio_mixgain',0);m.set('paused',1);m.resume();time.sleep(.5)
  for mode in (1,0):
   m.cmd('t1');m.set('fx_mode',mode);m.set('fx_pending',9);start=(WORK/(name+'.raw')).stat().st_size;m.resume();time.sleep(.9);m.cmd('t1');end=(WORK/(name+'.raw')).stat().st_size;spans.append((mode,start,end))
  m.write(0xffd301a,0)
  # SID registers are write-only. In this isolated test instance, redirect the
  # effect's STA operands to scratch RAM to inspect its exact generated writes.
  for a,b in [(LABELS['credit_sid_start'],LABELS['credit_decay_dispatch']),(LABELS['effects_off'],LABELS['effects_return'])]:
   code=m.read(a,b-a)
   for i in range(len(code)-2):
    if code[i]==0x8d and code[i+2]==0xd4:m.write(a+i+2,0x6b)
  m.set('fx_mode',1);m.set('fx_pending',9);m.call('effects_tick')
  check(name+': quick attack/decay envelope',m.read(0x6b13,2)==bytes([7,8]) and m.read(0x6b12)==bytes([17]))
  suffix='ntsc' if ntsc else 'pal'
  check(name+': high partial',m.read(0x6b0e,2)==m.read(LABELS['freqlo_'+suffix]+67)+m.read(LABELS['freqhi_'+suffix]+67))
  m.call('effects_tick');check(name+': settles to bell fundamental',m.read(0x6b0e,2)==m.read(LABELS['freqlo_'+suffix]+60)+m.read(LABELS['freqhi_'+suffix]+60))
  for _ in range(11):m.call('effects_tick')
  check(name+': clean end',m.v('soundtime')==0 and m.read(0x6b12)==b'\0')
 finally:m.close()
 raw=(WORK/(name+'.raw')).read_bytes()
 for mode,a,b in spans:
  samples=array('h');samples.frombytes(raw[a:b]);level=statistics.pstdev(samples) if samples else 0
  check(name+f': audible output, mode {mode}',level>30)
(OUT/'test-credit-pling.json').write_text(json.dumps(results,indent=2))
