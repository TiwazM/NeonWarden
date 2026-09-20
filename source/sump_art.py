"""Hand-authored tile vocabulary and skyline for the Sump Market art pass."""
def paint(tile,block,sky,blank,h):
    wall=tile(block(36));far=tile(block(35))
    windows=[]
    for n in range(6):
        a=block(36)
        for y in range(2,6):
            for x in range(1,6):a[y][x]=38
        if n in (1,2,4):
            for y in (3,4):
                for x in (2,3,4):a[y][x]=39 if n!=4 else 40
        if n==2:a[3][4]=9
        a[6][1:6]=[37]*5
        windows.append(tile(a))
    a=block(36)
    for y in range(8):a[y][0]=35;a[y][1]=37;a[y][7]=35
    column=tile(a)
    a=block(36);a[0]=[37]*8;a[1]=[35]*8
    roof=tile(a)
    a=block(36)
    for y in range(8):a[y][3]=35;a[y][4]=41;a[y][5]=37
    pipe=tile(a)
    a=block(1)
    for y in range(3,8):
        for x in range(1,7):a[y][x]=37
    for x in range(2,6):a[3][x]=41;a[5][x]=35
    plant=tile(a)
    a=block(1)
    for y in range(8):a[y][4]=37
    a[1][2:7]=[37]*5;a[0][4]=39
    aerial=tile(a)
    a=block(42);a[0]=[43]*8;a[1]=[41]*8;a[2]=[37]*8;a[7]=[0]*8
    a[4][1]=6;a[4][6]=6;a[5][3:5]=[35,35]
    ledge=tile(a)
    a=block(36);a[0]=[35]*8
    for y in range(8):a[y][0]=35;a[y][7]=37
    a[2][2]=37;a[5][5]=37
    steel=tile(a)
    a=block(36)
    for y in range(8):
        for x in range(8):
            if (x+y)%8 in (0,1):a[y][x]=37
    brace=tile(a)
    buildings=[];left=0;i=0
    widths=[6,8,4,6,8,4];roofs=[9,6,11,7,10,5,8,12,7,9,6]
    while left<128:
        width=widths[i%len(widths)];buildings.append((left,min(left+width,128),roofs[i%len(roofs)]))
        left+=width+2;i+=1
    m=[sky]*3072
    for row in range(24):
        for col in range(128):
            t=sky
            # Recessed silhouettes between the nearer buildings.
            if row>=12+(col//8)%3:t=far
            for i,(l,r,rh) in enumerate(buildings):
                if l<=col<r:
                    if row>=rh:
                        t=wall
                        if row==rh:t=roof
                        elif col in (l,r-1):t=column
                        elif (row-rh)%3:t=windows[(i+(col-l)//2+(row-rh)//3)%6]
                        if i%3==1 and col==r-2 and row>rh:t=pipe
                    if row==rh-1 and col==l+1:t=plant if i%2 else aerial
                    break
            if h[col]<192 and row>=h[col]//8:
                t=ledge if row==h[col]//8 else (brace if col%4==1 and row==h[col]//8+1 else steel)
            if h[col]==192 and row>=20:t=blank
            if row<2 or row==23:t=blank
            m[row*128+col]=t
    # One coherent recessed clinic facade masks the unrelated skyline behind it.
    for row in range(13,20):
        for col in range(29,36):
            m[row*128+col]=roof if row==13 else column if col in (29,35) else wall
    for row in range(14,20):
        for col in range(30,35):m[row*128+col]=tile(block(35))
    return m,buildings
