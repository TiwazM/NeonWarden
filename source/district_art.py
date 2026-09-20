"""District-specific industrial architecture; collision data is supplied unchanged."""
def paint(stage,tile,block,sky,blank,h):
    base,edge,light=[(45,46,9),(47,48,13),(36,37,40),(49,50,21)][stage-1]
    wall=tile(block(base));far=tile(block(35))
    a=block(base);a[0]=[edge]*8;a[1]=[35]*8;roof=tile(a)
    a=block(base)
    for y in range(8):a[y][0]=35;a[y][1]=edge;a[y][7]=35
    column=tile(a)
    panels=[]
    for n in range(3):
        a=block(base)
        if stage==1: # Pump housings, inset gauges and pipe clamps.
            for y in range(8):a[y][2]=35;a[y][3]=edge;a[y][4]=light
            if n==1:a[2][1:6]=[edge]*5;a[5][1:6]=[edge]*5
            if n==2:
                for y in range(2,6):a[y][1:7]=[35]*6
                a[3][3:5]=[9,10];a[6][2:6]=[edge]*4
        elif stage==2: # Vertical archive racks with restrained status lights.
            for y in range(1,7):a[y][1:7]=[35]*6
            for y in (2,4,6):a[y][2:5]=[edge]*3
            a[2+n][6]=light
            if n==2:a[4][6]=15
        elif stage==3: # Recessed upper-floor windows and ventilation grilles.
            for y in range(2,6):a[y][1:7]=[38]*6
            if n==1:a[3][2:6]=[40]*4
            if n==2:
                for y in (2,4):a[y][2:6]=[edge]*4
            a[6][1:7]=[edge]*6
        else: # Greenhouse panes, climbing vines and caged electrical cells.
            for y in range(1,7):a[y][1:7]=[35]*6
            for y in range(8):a[y][4]=edge
            a[4]=[edge]*8
            if n==1:
                for y in range(8):a[y][2+y%3]=21
                a[2][1]=22;a[5][5]=21
            if n==2:a[2][2]=9;a[3][2]=10;a[6][6]=light
        panels.append(tile(a))
    a=block(1)
    if stage in (1,4):
        for y in range(3,8):a[y][1:7]=[edge]*6
        a[3][2:6]=[light]*4;a[5][2:6]=[35]*4
        if stage==4:a[1][3:5]=[21,22];a[2][2:6]=[21]*4
    else:
        for y in range(8):a[y][4]=edge
        a[2][1:7]=[edge]*6;a[0][4]=light
    equipment=tile(a)
    a=block(base);a[0]=[6 if stage==3 else (10 if stage==1 else 27 if stage==2 else 23)]*8
    a[1]=[edge]*8;a[7]=[0]*8;a[4][1]=edge;a[4][6]=edge
    a[5][3:5]=[35]*2;ledge=tile(a)
    a=block(base);a[0]=[35]*8
    for y in range(8):a[y][0]=35;a[y][7]=edge
    a[3][2]=edge;a[5][5]=edge;steel=tile(a)
    a=block(base)
    for y in range(8):a[y][y]=edge;a[y][7-y]=edge
    brace=tile(a)
    # Sparse supports beneath one-way catwalks remain visually recessed.
    a=block(1)
    for y in range(8):a[y][3]=35;a[y][4]=edge
    support=tile(a)
    buildings=[];left=0;i=0
    widths={1:[8,6,10,6],2:[4,6,4,8],3:[8,6,10,4],4:[6,10,8,6]}[stage]
    roofs={1:[10,7,11,8,12],2:[5,8,4,7,10],3:[12,9,14,11,8],4:[10,7,12,8,11]}[stage]
    while left<128:
        w=widths[i%len(widths)];buildings.append((left,min(left+w,128),roofs[i%len(roofs)]));left+=w+2;i+=1
    m=[sky]*3072
    for row in range(24):
        for col in range(128):
            t=far if row>=15+(col//8)%2 else sky
            for i,(l,r,rh) in enumerate(buildings):
                if l<=col<r:
                    if row>=rh:
                        t=roof if row==rh else column if col in (l,r-1) else panels[(row//3+(col-l)//2+i)%3] if (row-rh)%4 else wall
                    if row==rh-1 and col==l+1:t=equipment
                    break
            if h[col]<192 and row>=h[col]//8:t=ledge if row==h[col]//8 else brace if row==h[col]//8+1 and col%3==1 else steel
            if h[col]==192 and row>=20:t=blank
            if row<2 or row==23:t=blank
            m[row*128+col]=t
    return m,buildings,ledge,support

def arena(tile,block,sky,blank):
    wall=tile(block(35))
    a=block(36);a[0]=[37]*8;a[7]=[35]*8
    for y in range(8):a[y][0]=35;a[y][7]=37
    panel=tile(a)
    a=block(35)
    for y in range(8):a[y][2]=37;a[y][3]=9;a[y][4]=8
    conduit=tile(a)
    a=block(35);a[2]=[37]*8;a[3]=[9]*8;cross=tile(a)
    a=block(2);a[0]=[6]*8;a[1]=[4]*8;a[7]=[0]*8;a[4][1]=18;a[4][6]=18;floor=tile(a)
    m=[blank]*3072
    for y in range(2,23):
        for x in range(32):
            t=wall
            if x in (0,31) or y in (2,19):t=panel
            elif x in (5,26):t=conduit
            elif y in (6,16):t=cross
            if y>=20:t=floor if y==20 else panel
            m[y*128+x]=t
    # A large recessed reactor halo, deliberately darker than the boss.
    a=[[35]*64 for _ in range(64)]
    for y in range(64):
        for x in range(64):
            d=abs(x-31.5)+abs(y-31.5)
            if 27<=d<=30:a[y][x]=9
            elif 30<d<=33:a[y][x]=37
            elif abs(x-31.5)<5 and abs(y-31.5)<19:a[y][x]=8
    for y in range(0,64,8):
        for x in range(0,64,8):m[(6+y//8)*128+12+x//8]=tile([r[x:x+8] for r in a[y:y+8]])
    return m

def warden():
    a=[[0]*24 for _ in range(28)]
    def rect(x,y,w,h,c):
        for yy in range(y,y+h):
            for xx in range(x,x+w):a[yy][xx]=c
    # Crown, single slit eye and a dark, faceted helmet.
    rect(6,1,2,5,18);rect(16,1,2,5,18);rect(11,0,2,5,19)
    rect(7,4,10,5,2);rect(8,5,8,2,4);rect(9,7,6,1,30)
    # Shoulder armour spreads to the edge; inset chest has a luminous core.
    rect(3,9,18,10,2);rect(1,10,6,5,4);rect(17,10,6,5,4)
    rect(2,10,4,2,18);rect(18,10,4,2,18)
    rect(7,10,10,9,3);rect(9,11,6,6,9);rect(10,12,4,4,11)
    rect(11,12,2,3,7);rect(7,18,10,3,2)
    # Heavy claws and two planted, splayed mechanical feet.
    rect(1,15,3,6,2);rect(20,15,3,6,2)
    rect(0,19,2,4,18);rect(3,19,2,3,18);rect(19,19,2,3,18);rect(22,19,2,4,18)
    rect(6,20,5,5,4);rect(13,20,5,5,4);rect(7,21,2,3,2);rect(15,21,2,3,2)
    rect(4,24,6,3,3);rect(14,24,6,3,3);rect(3,26,8,2,2);rect(13,26,8,2,2)
    rect(3,26,3,1,18);rect(18,26,3,1,18)
    return a
