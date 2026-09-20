"""Original NEON WARDEN pixel art and world compiler. Python 3, standard library only."""
from pathlib import Path
import json,sys
sys.path.insert(0,str(Path(__file__).resolve().parent))
import sump_art,hero_art,district_art,actor_art
P=Path(__file__).resolve().parent
out=[]
def data(name,values):
    out.append(name+':')
    for i in range(0,len(values),24):out.append(' .byte '+','.join(str(v&255) for v in values[i:i+24]))
RGB=['070a17','101629','19243b','293954','435779','73829e','abc8d7','ecfbff',
     '123e4a','167382','20c4cb','8cfff0','3d204e','74365d','d44187','ff88ae',
     '48343d','96604d','df9956','ffdf87','203d3c','38634a','66b96b','c5ed91',
     '24203c','3d3760','645581','a491be','361d2e','792a40','d84849','ff9670',
     '090f1b','162034','203049','0b1320','14243a','25374c','090e19','21424d','705348','3b5c70','314559','7d9aaa','422739']
RGB+=['102b32','30545a','201c32','48405b','152b29','3a5145']
pal=[tuple(bytes.fromhex(c)) for c in RGB]+[(0,0,0)]*(256-len(RGB))
for j,n in enumerate(['palette_r','palette_g','palette_b']):data(n,[((c[j]&15)<<4)|(c[j]>>4) for c in pal[:len(RGB)]])
out.append('palette_count = '+str(len(RGB)))
FONT={
'A':[14,17,17,31,17,17,17],'B':[30,17,17,30,17,17,30],'C':[14,17,16,16,16,17,14],
'D':[30,17,17,17,17,17,30],'E':[31,16,16,30,16,16,31],'F':[31,16,16,30,16,16,16],
'G':[14,17,16,23,17,17,15],'H':[17,17,17,31,17,17,17],'I':[14,4,4,4,4,4,14],
'J':[7,2,2,2,18,18,12],'K':[17,18,20,24,20,18,17],'L':[16,16,16,16,16,16,31],
'M':[17,27,21,21,17,17,17],'N':[17,25,21,19,17,17,17],'O':[14,17,17,17,17,17,14],
'P':[30,17,17,30,16,16,16],'Q':[14,17,17,17,21,18,13],'R':[30,17,17,30,20,18,17],
'S':[15,16,16,14,1,1,30],'T':[31,4,4,4,4,4,4],'U':[17,17,17,17,17,17,14],
'V':[17,17,17,17,17,10,4],'W':[17,17,17,21,21,21,10],'X':[17,17,10,4,10,17,17],
'Y':[17,17,10,4,4,4,4],'Z':[31,1,2,4,8,16,31],
'0':[14,17,19,21,25,17,14],'1':[4,12,4,4,4,4,14],'2':[14,17,1,2,4,8,31],
'3':[30,1,1,14,1,1,30],'4':[2,6,10,18,31,2,2],'5':[31,16,16,30,1,1,30],
'6':[14,16,16,30,17,17,14],'7':[31,1,2,4,8,8,8],'8':[14,17,17,14,17,17,14],
'9':[14,17,17,15,1,1,14],'-':[0,0,0,31,0,0,0],'/':[1,1,2,4,8,16,16],
':':[0,4,4,0,4,4,0],'.':[0,0,0,0,0,4,4],'+':[0,4,4,31,4,4,0],
'!':[4,4,4,4,4,0,4],'>':[16,8,4,2,4,8,16],' ': [0]*7}
data('font',sum((FONT.get(chr(i),[0]*7)+[0] for i in range(32,96)),[]))
data('xoffsetlo',[(x//8*64+x%8)&255 for x in range(256)])
data('xoffsethi',[(x//8*64+x%8)>>8 for x in range(256)])
data('yoffsetlo',[(y//8*2048+y%8*8)&255 for y in range(192)])
data('yoffsethi',[(y//8*2048+y%8*8)>>8 for y in range(192)])
(P/'lookup.inc').write_text('\n'.join(out)+'\n')
out=[]
tiles=[];lookup={}
def tile(a):
    k=bytes(sum(a,[]))
    if k not in lookup:lookup[k]=len(tiles);tiles.append(k)
    return lookup[k]
def block(c):return [[c]*8 for _ in range(8)]
blank=tile(block(0));sky=tile(block(1))
build=[]
for family in range(3):
    wall=2 if family==0 else 24 if family==1 else 8
    for variant in range(8):
        a=block(wall)
        for y in range(8):a[y][0]=1
        for x in range(8):a[7][x]=1
        if variant<6:
            for y in (2,3,4):
                for x in (2,3,5,6):a[y][x]=[3,4,8,9,17,13][variant]
            if variant in (3,4,5):a[2][2]=[10,19,14][variant-3];a[2][5]=a[2][2]
        else:
            for y in (1,3,5):
                for x in range(2,7):a[y][x]=3 if family!=2 else 9
        build.append(tile(a))
ground=[];top=[]
for family in range(3):
    a=block([2,8,24][family]);a[0]=[5,10,14][family:family+1]*8
    a[1]=[3]*8;a[6]=[1]*8;a[4][1]=4;a[4][6]=4
    top.append(tile(a))
    a=block([2,8,24][family]);a[0]=[1]*8
    for y in range(1,8):a[y][0]=1
    a[3][3]=[3,9,25][family];a[4][4]=[3,9,25][family]
    ground.append(tile(a))
water=[]
for n in range(4):
    # A recessed black void, with sparse red danger lights far below the lip.
    a=block(0)
    if n%2==0:
        for x in (2,3):a[6][x]=29;a[7][x]=30
    water.append(tile(a))
hazard=[]
for side in range(2):
    a=block(2)
    for y in range(4):
        for x in range(8):a[y][x]=19 if (x+y)%8<4 else 0
    for y in range(4,8):a[y][7 if side==0 else 0]=18
    hazard.append(tile(a))
heights=[];maps=[];skylines=[]
platforms=[(40,46,128),(48,54,96),(56,68,64),(68,74,96),(77,84,128),(86,92,96),(96,102,96),(104,110,64),(112,116,96)]
platforms5=[(38,45,128),(47,54,96),(56,63,128),(66,74,96),(77,85,64),(88,95,96),(96,101,128),(103,109,96),(111,116,128)]
(P/'platforms.inc').write_text('\n'.join(name+': .byte '+','.join(str(v[i]) for v in platforms+platforms5) for i,name in enumerate(['platform_left','platform_right','platform_y']))+'\n')
for stage in range(7):
    family=stage%3
    h=[160]*128
    spans=[(44,50,144),(56,63,128),(70,74,192),(79,85,144)] if stage==0 else [(39,44,144),(45,50,128),(54,58,192),(59,64,144),(68,74,128),(77,81,192),(82,88,144)] if stage==1 else [(39,44,144),(48,54,128),(58,62,144),(66,70,192),(74,80,144)]
    if stage==2:spans += [(80,88,128),(88,96,96),(96,104,64),(104,110,96),(110,114,128)]
    if stage==3:spans=[(60,66,192),(74,77,192),(94,112,192)]
    if stage==4:spans=[(42,47,192),(62,66,192),(84,88,192),(95,110,192)]
    if stage==5:spans=[]
    if stage==6:spans=[(44,70,192),(76,112,192)]
    for a,b,v in spans:h[a:b]=[v]*(b-a)
    heights.extend(h)
    m=[]
    for row in range(24):
        for col in range(128):
            roof=5+((col//5*7+stage*3)%7)
            t=sky
            if row>=roof:t=build[family*8+((col//2+row//2*3)%8)]
            if row==roof and col%5!=0:
                a=block([2,24,8][family]);a[0]=[3]*8;t=tile(a)
            if col%5==0 and row<19:t=sky
            if row>=h[col]//8 and h[col]<192:t=top[family] if row==h[col]//8 else ground[family]
            if h[col]==192 and row>=min(h[max(0,col-1)],h[min(127,col+1)],160)//8:
                t=blank if row<22 else water[col%4]
            if h[col]<192 and row==h[col]//8:
                if col<127 and h[col+1]==192:t=hazard[0]
                elif col and h[col-1]==192:t=hazard[1]
            if row<2 or row==23:t=blank
            m.append(t)
    if stage in (1,2,3,4):
        m,buildings,catwalk,support=district_art.paint(stage,tile,block,sky,blank,h)
    if stage==6:
        m,buildings,catwalk,support=district_art.paint(3,tile,block,sky,blank,h)
    if stage==0:
        m,sump_buildings=sump_art.paint(tile,block,sky,blank,h)
        buildings=sump_buildings
    if stage!=5:
        for col in range(128):
            if h[col]==192:m[22*128+col]=water[col%4]
            elif (col and h[col-1]==192) or (col<127 and h[col+1]==192):m[(h[col]//8)*128+col]=hazard[0 if col<127 and h[col+1]==192 else 1]
    # Neon architecture details. Fixed tile patterns keep stock memory modest.
    def stamp(x,y,w,hh,kind):
        a=[[1]*w for _ in range(hh)]
        color=[10,14,19][kind]
        for yy in range(hh):
            for xx in range(w):
                a[yy][xx]=2 if kind!=1 else 24
                if xx in (0,w-1):a[yy][xx]=4
                elif xx in (1,w-2):a[yy][xx]=color if yy%8<6 else 3
                elif yy in (0,hh-1):a[yy][xx]=5
                elif xx in (w//2-1,w//2):a[yy][xx]=0
                elif yy%8==1:a[yy][xx]=3
                elif yy%8==6:a[yy][xx]=1
        for yy in (3,hh-4):
            for xx in (3,w-4):a[yy][xx]=6
        for yy in range(hh//2-3,hh//2+3):
            for xx in range(w-6,w-3):a[yy][xx]=0
        a[hh//2-2][w-5]=color;a[hh//2][w-5]=11
        for xx in range(3,w-3):a[hh-3][xx]=19 if xx%6<3 else 1
        for yy in range(0,hh,8):
            for xx in range(0,w,8):m[(y//8+yy//8)*128+x//8+xx//8]=tile([r[xx:xx+8] for r in a[yy:yy+8]])
    if stage==0:stamp(56,128,16,32,0)
    stamp(248,128,24,32,1);stamp(952,120,24,40,2)
    # Four unique businesses per district; the 32px signs align to 32px facades.
    words=[['NET','EYE','RAM','BAR'],['MOD','TEA','ION','MED'],['ARC','ZEN','LAB','VIP'],['SKY','AIR','CPU','HEX'],['LUX','COIL','VLT','AMP'],['CORE']*4,['SKY','AIR','CPU','HEX']][stage]
    for n,col in enumerate(() if stage==5 else (16,46,81,106)):
        if stage!=5:
            l,r,rh=min(buildings,key=lambda b:abs((b[0]+b[1])/2-(col+2)))
            col=l+(r-l-4)//2
        x=col*8;roof=(5+((col//5*7+stage*3)%7))*8
        if stage!=5:roof=rh*8
        y=roof-16 if (n+stage)%2==0 else roof+16
        # Signs sit above both the roof and raised playable terrain.
        if stage in (2,3,4):
            y=min(roof-16,min(h[col:col+4])-16)
            if stage in (3,4):
                for left,right,py in (platforms if stage==3 else platforms5):
                    if col<right and col+4>left and y<py+8 and y+16>py:
                        y=min(y,py-24)
            y=max(16,y//8*8)
        if stage in (2,3,4) and y+16<roof:
            for row in range((y+16)//8,roof//8):
                for cc in range(l,r):
                    if row*8 < h[cc]:
                        m[row*128+cc]=m[(rh if row==(y+16)//8 else rh+1)*128+cc]
        w=32;hh=16;color=[11,15,19][family]
        a=[[1]*w for _ in range(hh)]
        for xx in range(1,31):a[1][xx]=color;a[12][xx]=[9,13,17][family]
        for yy in range(2,12):a[yy][0]=color;a[yy][31]=color
        if n%2==0:
            for k,ch in enumerate(words[n]):
                for yy,b in enumerate(FONT[ch]):
                    for xx in range(5):
                        if b&(16>>xx):a[4+yy][8+k*6+xx]=color
        else:
            # Cross, eye, lightning and chip pictograms vary by district.
            kind=(n//2+stage)%4
            for yy in range(3,11):
                for xx in range(10,23):
                    on=(14<=xx<=17 or 6<=yy<=7) if kind==0 else (abs(xx-16)+abs(yy-7)*2 in (5,6) or (xx==16 and yy==7)) if kind==1 else (xx==19-yy//2 or xx==20-yy//2 or (yy==7 and 14<=xx<=19)) if kind==2 else (xx in (12,20) or yy in (4,9) or (xx%3==0 and yy in (3,10)))
                    if on:a[yy][xx]=color
        for yy in (13,14,15):a[yy][6]=4;a[yy][25]=4
        for yy in (0,8):
            for xx in (0,8,16,24):m[(y//8+yy//8)*128+x//8+xx//8]=tile([r[xx:xx+8] for r in a[yy:yy+8]])
    if stage in (3,4):
        for left,right,y in (platforms if stage==3 else platforms5):
            for col in range(left,right):m[(y//8)*128+col]=catwalk
    if stage==5:
        # A sealed 256px interior: steel walls, conduits and a central core.
        for row in range(24):
            for col in range(128):
                t=blank
                if col<32:
                    t=sky
                    if 2<=row<20 and (col in (0,31) or row==2):t=ground[2]
                    elif 3<=row<20 and col%8 in (0,7):t=build[16+(row//2)%8]
                    if 7<=row<=14 and 12<=col<=19:t=build[16+(col+row)%8]
                    if row>=20 and row<23:t=top[2] if row==20 else ground[2]
                m[row*128+col]=t
        m=district_art.arena(tile,block,sky,blank)
        stamp(8,128,16,32,0);stamp(224,120,16,40,2)
    # Extend floor art into the former gameplay-controls strip.
    for col in range(128):
        m[23*128+col]=m[22*128+col] if h[col]<192 else blank
    skylines.append([next((row*8 for row in range(2,23) if m[row*128+col]!=sky),184) for col in range(128)])
    maps.append(m)
# Discard overwritten construction tiles before assigning the final byte IDs.
used=sorted({0,1}|{t for m in maps for t in m})
remap={old:new for new,old in enumerate(used)}
tiles=[tiles[i] for i in used];maps=[[remap[t] for t in m] for m in maps]
assert len(tiles)<=256,len(tiles)
data('terrain',heights)
for n,line in enumerate(skylines):data('skyline'+str(n),line)
out.append('skylinelo: .byte <skyline0,<skyline1,<skyline2,<skyline3,<skyline4,<skyline5,<skyline6\nskylinehi: .byte >skyline0,>skyline1,>skyline2,>skyline3,>skyline4,>skyline5,>skyline6')
tail=out.index('skyline4:')
(P/'geography_tail.inc').write_text('\n'.join(out[tail:])+'\n')
out=out[:tail]
(P/'geography.inc').write_text('\n'.join(out)+'\n')
out=[]
for n,m in enumerate(maps):
    pairs=list(dict.fromkeys((m[y*128+x],m[y*128+min(x+1,127)]) for y in range(24) for x in range(128)))
    assert len(pairs)<256
    data('pair_first'+str(n),[p[0] for p in pairs])
    data('pair_second'+str(n),[p[1] for p in pairs])
    out.append('pair_count'+str(n)+' = '+str(len(pairs)))
    m=[pairs.index((m[y*128+x],m[y*128+min(x+1,127)])) for y in range(24) for x in range(128)]
    r=[];i=0;literal=[]
    def flush():
        if literal:r.extend([len(literal),*literal]);literal.clear()
    while i<len(m):
        best=0;distance=0
        for j in range(max(0,i-4095),i):
            if m[j]!=m[i]:continue
            k=1
            while k<130 and i+k<len(m) and m[j+k]==m[i+k]:k+=1
            if k>best:best=k;distance=i-j
        if best>=4:
            flush();r.extend([128+best-3,distance&255,distance>>8]);i+=best
        else:
            literal.append(m[i]);i+=1
            if len(literal)==127:flush()
    flush();r.append(0);data('map_rle'+str(n),r)
cut=out.index('pair_first6:')
(P/'ferry_world.inc').write_text('\n'.join(out[cut:])+'\n')
out=out[:cut]
out.append('maps_lo: .byte <map_rle0,<map_rle1,<map_rle2,<map_rle3,<map_rle4,<map_rle5,<map_rle6\nmaps_hi: .byte >map_rle0,>map_rle1,>map_rle2,>map_rle3,>map_rle4,>map_rle5,>map_rle6')
for base in ['pair_first','pair_second']:
    out.append(base+'lo: .byte '+','.join('<'+base+str(i) for i in range(7)))
    out.append(base+'hi: .byte '+','.join('>'+base+str(i) for i in range(7)))
out.append('pair_counts: .byte pair_count0,pair_count1,pair_count2,pair_count3,pair_count4,pair_count5,pair_count6')
(P/'world.inc').write_text('\n'.join(out)+'\n')
out=[]
# Raster sprites, explicitly authored silhouettes, armour and scarf.
def sprite(name,rows,colors):
    w=len(rows[0]);assert all(len(r)==w for r in rows)
    data(name,[w,len(rows)]+[colors.get(c,0) for r in rows for c in r])
actor_frames={}
def actor(name,rows,colors=None):
    pixels=[[colors.get(c,0) for c in r] for r in rows] if colors else rows
    actor_frames[name]=pixels
    actor_frames[name+'_alt']=actor_art.motion(pixels,name)
    data(name,actor_art.packed(pixels))
    data(name+'_alt',actor_art.packed(actor_art.motion(pixels,name)))
hero=['................','......2222......','.....222222.....','.....233332.....','.....2AAAA2.....','......3333......','....BB44444.....','..BBBB455554....','BBBB..455554....','BB....4566544...',
'.....44566544...','.....44566544...','.....44555544...','......455554....','......777777....','......445544....','......44.544....','......44.544....','.....444.544....','.....44..544....','.....44..544....','.....44..544....','....777..7777...','....777..7777...']
colors={'2':2,'3':17,'4':21,'5':22,'6':23,'7':1,'A':11,'B':14}

walk=hero[:16]+['.....444.554....','.....44...544...','....444...544...','....44.....44...','...444.....444..','...44.......44..','..777.......777.','..777.......777.']
asset_prefix=out;out=[]
data('hero_palette',hero_art.PALETTE)
data('hero_high', [hero_art.PALETTE[i>>4] for i in range(256)])
(P/'hero_colors.inc').write_text('\n'.join(out)+'\n');out=[]
data('hero_low', [hero_art.PALETTE[i&15] for i in range(256)])
(P/'hero_low.inc').write_text('\n'.join(out)+'\n');out=asset_prefix
hero_frames=hero_art.frames()
asset_prefix=out;out=[]
for i,rows in enumerate(hero_frames):
    pixels=[0 if c=='.' else int(c,16) for r in rows for c in r]
    assert len(pixels)==384
    data('hero_pose'+str(i),[(pixels[j]<<4)|pixels[j+1] for j in range(0,384,2)])
out.append('hero_pose_lo: .byte '+','.join('<hero_pose'+str(i) for i in range(len(hero_frames))))
out.append('hero_pose_hi: .byte '+','.join('>hero_pose'+str(i) for i in range(len(hero_frames))))
(P/'hero.inc').write_text('\n'.join(out)+'\n');out=asset_prefix
enemy=[r.replace('B','.') for r in hero]
actor('guard',enemy,{'2':28,'3':3,'4':16,'5':17,'6':30,'7':1,'A':19})
drone=['................','................','................','................','................','................','..22........22..','.2442......2442.','2444422222244442','22AAA444444AAA22','..224433334422..','....433AA334....','....24333342....','.....224422.....','......2..2......','......A..A......']+['................']*8
actor('drone',drone,{'2':1,'3':13,'4':5,'A':15})
boss=['........22222222........','.......2444444442.......','......244333333442......','......24AAAAAAAA42......','......244333333442......','.......2444444442.......','....2222666666662222....','...244446666666644442...','..24444466777766444442..','..24444466777766444442..','..24422466777764224442..','..2442.246666642.24442..','..2442.246666642.24442..','..2442.244444442.24442..','..2442.244444442.24442..','..2442..2222222..24442..','..2AA2..2442442..2AA42..','..2AA2..2442442..2AA42..','...22...2442442...222...','........2442442.........','.......2442.2442........','.......2442.2442........','.......2442.2442........','.......2442.2442........','......24442.24442.......','......24442.24442.......','.....244442.244442......','.....222222.222222......']
actor('boss',boss,{'2':1,'3':28,'4':5,'6':16,'7':30,'A':19})
actor('warden',district_art.warden())
(P/'warden.inc').write_text('; Warden animation is packed with the other actors.\n')
# PATCH: original shopkeeper, silver hair, cybernetic eye and orange apron.
vendor=[[0]*32 for _ in range(40)]
def vr(x,y,w,h,c):
    for yy in range(y,y+h):
        for xx in range(x,x+w):vendor[yy][xx]=c
vr(10,2,13,4,5);vr(8,5,17,5,6);vr(8,8,3,11,5)
vr(11,10,12,12,17);vr(12,10,10,4,18);vr(11,14,13,3,3)
vr(12,14,4,2,11);vr(20,14,4,2,15);vr(16,17,3,3,18)
vr(14,21,7,4,17);vr(6,25,22,15,3);vr(8,25,18,13,4)
vr(11,25,3,15,18);vr(22,25,3,15,18);vr(11,30,14,10,17)
vr(13,31,10,8,18);vr(15,32,6,2,19);vr(3,28,5,10,4)
vr(27,28,4,10,4);vr(3,36,6,4,18);vr(25,36,6,4,18)
data('vendor',actor_art.packed(vendor))
# Small shop inventory pictures: blade, plate, repair, pistol and ammunition.
icons=[['.......1','......11','.....11.','....11..','...11...','2.11....','.22.....','2..2....'],['.11..11.','12211221','12222221','.122221.','..1221..','...11...','........','........'],['..1111..','.122221.','12211221','12111121','12111121','12211221','.122221.','..1111..'],['........','.1111111','.1222221','.111111.','..121...','..121...','..111...','........'],['........','..1..1..','.121121.','.121121.','.121121.','.121121.','.111111.','........']]
for i,rows in enumerate(icons):sprite('item'+str(i),rows,{'1':[11,10,11,6,19][i],'2':[5,9,10,4,18][i]})
sprite('gun_left',[r[::-1] for r in icons[3]],{'1':6,'2':4})

names=['guard','guard_alt','drone','drone_alt','boss','boss_alt','warden','warden_alt']
out.append('enemy_pose_lo: .byte '+','.join('<'+n for n in names))
out.append('enemy_pose_hi: .byte '+','.join('>'+n for n in names))
out.append('tilelo: .byte '+','.join('<tile_'+str(i) for i in range(len(tiles))))
out.append('tilehi: .byte '+','.join('>tile_'+str(i) for i in range(len(tiles))))
for i,t in enumerate(tiles):
    pal=list(dict.fromkeys(t));assert len(pal)<=16
    codes=[pal.index(c) for c in t]
    data('tile_'+str(i),[len(pal),*pal,*[(codes[j]<<4)|codes[j+1] for j in range(0,64,2)]])
(P/'actor_frames.json').write_text(json.dumps(actor_frames))
(P/'assets.inc').write_text('\n'.join(out)+'\n')
# Fine scrolling: compose each 8x8 output tile from two adjacent source tiles.
code=[]
for fine in range(8):
    code += [f'copy{fine}:',' ldz #0',f' ldy #{fine}',f'copyrow{fine}:']
    for x in range(8):
        if x+fine==8:code+=[' tya',' sec',' sbc #8',' tay']
        code += [f" lda ({'src' if x+fine<8 else 'src2'}),y",' sta [pix],z',' inz',' iny']
    if fine:code+=[' tya',' clc',' adc #8',' tay']
    code += [' cpz #64',f' bne copyrow{fine}',' rts']
code+=['copylo: .byte '+','.join('<copy'+str(i) for i in range(8)), 'copyhi: .byte '+','.join('>copy'+str(i) for i in range(8))]
(P/'scroll.inc').write_text('\n'.join(code)+'\n')
(P/'world.json').write_text(json.dumps({'palette':RGB,'hero_frames':[[hero_art.PALETTE[0 if c=='.' else int(c,16)] for r in rows for c in r] for rows in hero_frames],'sump_buildings':sump_buildings,'tiles':[list(t) for t in tiles],'maps':maps,'platforms':platforms,'platforms5':platforms5,'skylines':skylines,'heights':[heights[i*128:(i+1)*128] for i in range(6)]}))
print(f'{len(tiles)} original tiles; five districts and a boss arena')


