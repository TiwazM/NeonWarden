"""Small authored pose variations, preserving the game's existing silhouettes."""
def packed(rows):
    pixels=sum(rows,[]);data=[len(rows[0]),len(rows)];i=0
    while i<len(pixels):
        end=i+1
        while end<len(pixels) and pixels[end]==pixels[i] and end-i<255:end+=1
        data.extend([end-i,pixels[i]]);i=end
    return data+[0]

def motion(rows,kind):
    a=[r[:] for r in rows];w=len(a[0]);h=len(a)
    if kind=='guard':
        # Opposing leg steps and a bent forward arm, with feet on the same floor.
        for y in range(16,h):
            a[y]=[0]*w
            for x,c in enumerate(rows[y]):
                nx=x+(-1 if x<w//2 else 1)
                if 0<=nx<w:a[y][nx]=c
        for y in range(9,13):a[y][12]=17
    elif kind=='drone':
        # Raised wing tips and bright twin exhaust jets.
        for y in range(6,10):
            for x in list(range(1,5))+list(range(11,15)):
                a[y-1][x]=rows[y][x];a[y][x]=0
        for x in (6,9):a[16][x]=11;a[17][x]=10;a[18][x]=9
    else:
        # A guarded pose: lift the outer claws and open the stance.
        for y in range(6,20):
            for x in list(range(0,6))+list(range(w-6,w)):
                a[y-2][x]=rows[y][x]
        for y in range(h-7,h):
            a[y]=[0]*w
            for x,c in enumerate(rows[y]):
                nx=x+(-1 if x<w//2 else 1)
                if 0<=nx<w:a[y][nx]=c
    return a
