from pathlib import Path
import subprocess,os,socket,time,re,json,sys
import tempfile,shutil
OUT=Path(__file__).resolve().parents[2]
WORK=Path(os.environ.get('NEON_TEST_WORK',str(Path(tempfile.gettempdir())/'neon-warden-tests')))
WORK.mkdir(parents=True,exist_ok=True)
exe=Path(os.environ['NEON_XEMU'])
rom=Path(os.environ['NEON_ROM'])
seed=Path(os.environ['NEON_SD_IMAGE'])
local=WORK/'emulator';profile=local/'mega65';profile.mkdir(parents=True,exist_ok=True)
for name in ['xmega65.exe','SDL2.dll']:
    shutil.copy2(exe.parent/name,local/name)
(profile/'prefdir-is-here.txt').write_text('Isolated integration tests')
(profile/'notfirsttimeuser.txt').write_text('1')
i2c=Path(os.environ.get('NEON_I2C',str(exe.parent/'mega65/i2c.bin')))
if not i2c.exists():raise SystemExit('Set NEON_I2C to an existing Xemu i2c.bin from a configured emulator profile.')
shutil.copy2(i2c,profile/'i2c.bin')
if not (WORK/'test-sd.img').exists():shutil.copy2(seed,WORK/'test-sd.img')
LABELS={k:int(v,16) for k,v in re.findall(r'^(\w+)\s*=\s*\$([0-9a-fA-F]+)',(OUT/'source/neon-warden.labels').read_text(),re.M)}
class Machine:
 def __init__(self,name='title',ntsc=False,disk=False,sound=False):
  with socket.socket() as probe:
   probe.bind(('127.0.0.1',0));self.port=probe.getsockname()[1]
  env=os.environ.copy();env.update(SDL_VIDEODRIVER='dummy',SDL_RENDER_DRIVER='software')
  args=[str(WORK/'emulator/xmega65.exe'),'-rom',str(rom),'-sdimg',str(WORK/'test-sd.img'),'-uartmon',f':{self.port}','-screenshot',str(WORK/(name+'.png')),'-fastboot','-besure','-syscon','-gui','none']
  if sound:env.update(SDL_AUDIODRIVER='disk',SDL_DISKAUDIOFILE=str(WORK/(name+'.raw')))
  else:args+=['-nosound']
  if ntsc:args+=['-videostd','1','-lockvideostd']
  args+=['-8',str(OUT/'NEONWARD.D81'),'-autoload'] if disk else ['-prg',str(OUT/'NEONWARD.PRG')]
  self.proc=subprocess.Popen(args,cwd=WORK/'emulator',env=env,stdout=open(WORK/'xemu-out.txt','w'),stderr=open(WORK/'xemu-err.txt','w'),creationflags=subprocess.CREATE_NO_WINDOW)
  for i in range(100):
   time.sleep(.1)
   try:self.s=socket.create_connection(('127.0.0.1',self.port),timeout=1);break
   except OSError:
    if self.proc.poll() is not None:raise RuntimeError((WORK/'xemu-err.txt').read_text())
  self.s.settimeout(5);self.s.sendall(b'\r');self.until();time.sleep(3)
 def until(self):
  out=b''
  while True:
   d=self.s.recv(65536)
   if not d:break
   out+=d
   if out.rstrip().endswith(b'.'):break
  return out.decode(errors='replace')
 def cmd(self,c):self.s.sendall((c+'\r').encode());return self.until()
 def read(self,a,n=1):
  data=bytearray()
  stride=256 if n>=256 else 16
  for i in range(0,n,stride):
   s=self.cmd(f'{"M" if stride==256 else "m"}{a+i:x}');h=re.findall(r':[0-9a-fA-F]{8}:([0-9a-fA-F]{32})',s)
   if not h:raise RuntimeError(s)
   for value in h:data.extend(bytes.fromhex(value))
  return bytes(data[:n])
 def write(self,a,*v):self.cmd(f's{a:x} '+' '.join(f'{x&255:02x}' for x in v))
 def v(self,n):return self.read(LABELS[n])[0]
 def set(self,n,*v):self.write(LABELS[n],*v)
 def call(self,n):
  self.cmd('t1');a=LABELS[n]
  self.write(0x6c00,0xa2,0xff,0x9a,0x20,a&255,a>>8,0x4c,0x06,0x6c)
  self.cmd('g6c00');self.cmd('t0');time.sleep(.12);self.cmd('t1')
  r=self.cmd('r')
  if not re.search(r'\b6C06\b',r):raise RuntimeError(n+' did not return '+r)
 def resume(self):self.cmd('g'+format(LABELS['main'],'x'));self.cmd('t0')
 def press(self,scan,slot=0):self.write(0xffd3615+slot,scan)
 def release(self):
  for i in range(3):self.press(127,i)
 def tap(self,scan):self.press(scan);time.sleep(.2);self.release();time.sleep(.2)
 def close(self):
  try:self.cmd('t0');self.cmd('~exit')
  except (OSError,TimeoutError):pass
  self.s.close()
  try:self.proc.wait(3)
  except subprocess.TimeoutExpired:self.proc.terminate();self.proc.wait(5)
if __name__=='__main__':
 m=Machine()
 try:
  print(m.cmd('r'));print({n:m.v(n) for n in ['state','frame','back','health','stage','videoticks']})
  a=m.v('frame');time.sleep(2);print('updates/2s',(m.v('frame')-a)&255)
 finally:m.close()
