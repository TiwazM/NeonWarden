"""Pack the program; bootstrap uses ordinary MEGA65 RAM and 45GS02 long pointers."""
from pathlib import Path
from collections import defaultdict,deque
def pack(data):
 out=bytearray();lit=bytearray();seen=defaultdict(lambda:deque(maxlen=48));i=0
 def flush():
  if lit:out.append(len(lit));out.extend(lit);lit.clear()
 while i<len(data):
  best=0;offset=0
  for j in reversed(seen[data[i:i+3]]):
   if i-j>4095:break
   k=0
   while k<130 and i+k<len(data) and data[j+k]==data[i+k]:k+=1
   if k>best:best=k;offset=i-j
  n=best if best>=4 else 1
  if n>1:flush();out.extend((128+n-3,offset&255,offset>>8))
  else:
   lit.append(data[i])
   if len(lit)==127:flush()
  for j in range(i,i+n):seen[data[j:j+3]].append(j)
  i+=n
 flush();out.append(0)
 return out
def unpack(data):
 out=bytearray();i=0
 while data[i]:
  n=data[i];i+=1
  if n<128:out.extend(data[i:i+n]);i+=n
  else:
   d=data[i]+256*data[i+1];i+=2
   for _ in range((n&127)+3):out.append(out[-d])
 return out
def prepare(p):
 raw=(p/'neon-warden-unpacked.prg').read_bytes();assert raw[:2]==b'\x01\x20'
 data=raw[2+0x2040-0x2001:]
 first=data[:0xd000-0x2040];second=data[0xe000-0x2040:]
 a=pack(first);b=pack(second);assert unpack(a)==first and unpack(b)==second
 packed=a+b
 (p/'packed-game.bin').write_bytes(packed)
 print('Packed game:',len(data),'->',len(packed),'bytes')
