"""Usage: python build.py [path/to/64tass]. No Python packages required."""
from pathlib import Path
import sys,runpy,shutil,subprocess
p=Path(__file__).resolve().parent
tass=sys.argv[1] if len(sys.argv)>1 else shutil.which('64tass')
if not tass:raise SystemExit('Install 64tass 1.60+ or supply its executable path.')
for name in ['assets.py','ui.py','music.py']:runpy.run_path(str(p/name),run_name='__main__')
subprocess.run([tass,'-a','-B',str(p/'neon-warden.asm'),'-o',str(p/'neon-warden-unpacked.prg'),'-l',str(p/'neon-warden.labels')],check=True)
sys.path.insert(0,str(p))
import pack_game
pack_game.prepare(p)
subprocess.run([tass,'-a','-B',str(p/'packed_loader.asm'),'-o',str(p.parent/'NEONWARD.PRG')],check=True)
runpy.run_path(str(p/'make_disk.py'),run_name='__main__')

