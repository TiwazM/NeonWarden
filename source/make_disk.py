"""Create an ordinary, writable 1581 D81 disk with NEONWARD as first PRG."""
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
def build():
    disk=bytearray(80*40*256)
    def pos(t,s):return ((t-1)*40+s)*256
    def sector(t,s):return memoryview(disk)[pos(t,s):pos(t,s)+256]
    used={(40,s) for s in range(4)}
    h=sector(40,0);h[0:3]=bytes([40,3,68]);h[4:20]=b'NEON WARDEN'.ljust(16,b'\xa0')
    h[20:29]=b'\xa0\xa0ID\xa03D\xa0\xa0'
    directory=sector(40,3);directory[0:2]=b'\x00\xff'
    data=(ROOT/'NEONWARD.PRG').read_bytes()
    blocks=(len(data)+253)//254
    chain=[(t,s) for t in range(1,81) if t!=40 for s in range(40)][:blocks]
    for i,(t,s) in enumerate(chain):
        used.add((t,s));sec=sector(t,s);chunk=data[i*254:(i+1)*254]
        sec[:2]=bytes(chain[i+1]) if i+1<len(chain) else bytes([0,len(chunk)+1])
        sec[2:2+len(chunk)]=chunk
    directory[2:5]=bytes([0x82,*chain[0]])
    directory[5:21]=b'NEONWARD'.ljust(16,b'\xa0')
    directory[30:32]=blocks.to_bytes(2,'little')
    for half in range(2):
        b=sector(40,half+1);b[:8]=bytes([40,2,68,0xbb,73,68,0xc0,0]) if half==0 else bytes([0,255,68,0xbb,73,68,0xc0,0])
        for n in range(40):
            t=half*40+n+1;free=[s for s in range(40) if (t,s) not in used]
            off=16+n*6;b[off]=len(free)
            for s in free:b[off+1+s//8]|=1<<(s%8)
    # Verify sector chain and allocation counts before saving.
    got=bytearray();t,s=chain[0];visited=set()
    while True:
        assert (t,s) not in visited;visited.add((t,s));b=sector(t,s)
        if not b[0]:got.extend(b[2:1+b[1]]);break
        got.extend(b[2:]);t,s=b[0],b[1]
    assert got==data and len(visited)==blocks and len(disk)==819200
    (ROOT/'NEONWARD.D81').write_bytes(disk)
    print(f'NEONWARD.D81: {blocks} program blocks, {3200-len(used)} blocks free')
if __name__=='__main__':build()
