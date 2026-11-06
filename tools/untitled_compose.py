#!/usr/bin/env python3
"""UNTITLED background composer: builds unt_* maps from intact tile stamps cut by
coordinate from coherent source maps (vermillion+sample_town / kanto-house1f /
240401-dungeon2), quantizes to DMG and writes matching collisions."""
import json,os,glob
from PIL import Image,ImageDraw
R=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BG=R+'/assets/backgrounds'
C=[(224,248,208),(136,192,112),(52,104,86),(8,24,32)]
GRAYMAP={0:3,44:3,92:2,116:2,157:1,188:1,252:0}
def q(px):
    if px[0]==px[1]==px[2] and px[0] in GRAYMAP: return C[GRAYMAP[px[0]]]
    l=0.3*px[0]+0.59*px[1]+0.11*px[2]
    return min(C,key=lambda c:abs(0.3*c[0]+0.59*c[1]+0.11*c[2]-l))
SRC={}
def src(n):
    if n not in SRC: SRC[n]=Image.open(BG+'/'+n).convert('RGB')
    return SRC[n]
class Map:
    def __init__(s,w,h):
        s.w,s.h=w,h; s.im=Image.new('RGB',(w*8,h*8),C[0]); s.col=[[0]*w for _ in range(h)]
    def stamp(s,srcn,sx,sy,x,y,w=1,h=1,solid=False):
        t=src(srcn).crop((sx*8,sy*8,(sx+w)*8,(sy+h)*8))
        for j in range(h):
            for i in range(w):
                X,Y=x+i,y+j
                if 0<=X<s.w and 0<=Y<s.h:
                    s.im.paste(t.crop((i*8,j*8,i*8+8,j*8+8)),(X*8,Y*8))
                    if solid is not None: s.col[Y][X]=0x0f if solid else 0
    def fill(s,srcn,sx,sy,x0,y0,x1,y1,pw=1,ph=1,solid=False):
        for y in range(y0,y1):
            for x in range(x0,x1):
                s.stamp(srcn,sx+(x-x0)%pw,sy+(y-y0)%ph,x,y,solid=solid)
    def save(s,name,scene=None,check=()):
        im=s.im.convert('RGB'); px=im.load()
        for y in range(im.height):
            for x in range(im.width): px[x,y]=q(px[x,y])
        tiles={im.crop((x,y,x+8,y+8)).tobytes() for y in range(0,im.height,8) for x in range(0,im.width,8)}
        im.save(BG+'/'+name)
        for (ax,ay) in check:
            assert s.col[ay][ax]==0,(name,ax,ay,'actor on solid')
        if scene:
            p=R+f'/project/scenes/{scene}/scene.gbsres'; d=json.load(open(p))
            assert d['width']==s.w and d['height']==s.h
            d['collisions']=compress([v for r in s.col for v in r])
            json.dump(d,open(p,'w'),indent=2)
        print(name,im.size,'unique tiles',len(tiles)); return len(tiles)
def compress(arr):
    out='';last=-1;cnt=0
    for v in arr:
        if v!=last:
            if cnt==1: out+='!'
            elif cnt>0: out+=f'{cnt:x}+'
            cnt=0;last=v;out+=f'{v%256:02x}'
        cnt+=1
    out+='!' if cnt==1 else f'{cnt:x}+'
    return out

# ---------- CURB (vermillion buildings/pavement + sample_town grass/trees) ----------
ST='sample_town.png'; VM='230927-Kanto-vermillion.png'
def tree(m,x,y): m.stamp(ST,4,12,x,y,4,4,solid=True)
def curb():
    m=Map(40,36)
    m.fill(ST,9,9,0,0,40,36,2,2)                     # grass
    for x in range(-2,40,4): tree(m,x,0)            # north tree line rows 0-3
    for i,bx in enumerate((4,13,22,31)):            # 4 buildings rows 4-11, doors at bx+1..2 rows 10-11
        m.stamp(VM,12,0,bx,4,8,8,solid=True)
        m.stamp(VM,13,6,bx+1,10,2,2,solid=False)
    m.fill(VM,14,9,0,12,40,16,2,2)                  # sidewalk
    m.fill(ST,15,20,0,16,40,22)                     # road base (flat mid green)
    road=Image.new('RGB',(8,8),C[2]); dash=road.copy(); ImageDraw.Draw(dash).rectangle((0,3,4,4),fill=C[0])
    for y in range(16,22):
        for x in range(40): m.im.paste(dash if (y==18 and x%3==0) else road,(x*8,y*8))
    m.stamp(VM,12,12,0,22,1,1)
    for x in range(40): m.stamp(VM,12+(x%8),12,x,22,solid=True)   # hedge ledge along park
    for x in (14,15): m.fill(VM,14,9,x,22,x+1,23)                 # gap in hedge to park
    for (x,y) in ((2,25),(7,30),(20,27),(26,24),(31,30),(36,25),(1,31),(14,31)): tree(m,x,y)
    m.stamp(ST,25,13,34,13,2,2,solid=True)           # notice board by bus stop
    for (x,y) in ((10,25),(18,24),(24,33),(30,27)): m.stamp(ST,4,11,x,y,solid=True)
    for x in range(40): m.col[0][x]=m.col[35][x]=0x0f
    for y in range(36): m.col[y][0]=m.col[y][39]=0x0f
    return m.save('unt_curb.png','scn_unt_curb',[(14,23),(35,17),(32,13),(30,13),(5,12),(36,15)])

# ---------- interiors: kanto-house1f ----------
H='230927-kanto-house1f.png'
def room(w,h,top=1,doorx=None,ends=None):
    m=Map(w,h)
    m.fill(H,0,0,0,0,w,h,solid=True)                # black void/border
    m.fill(H,6,5,1,top+2,w-1,h-1,2,2)               # diagonal floor
    for x in range(1,w-1):                          # wall band (2 rows)
        m.stamp(H,6+(x%2),1,x,top,1,2,solid=True)
    for x in range(4,w-4,7): m.stamp(H,8,1,x,top,2,2,solid=True)   # windows
    if doorx is not None:
        m.fill(H,6,5,doorx,h-1,doorx+4,h,2,2)
        m.stamp(H,6,15,doorx,h-3,4,2,solid=False)    # door mat
    if ends:
        for y in range(ends[0],ends[1]):
            m.stamp(H,6,5+(y%2),0,y); m.stamp(H,7,5+(y%2),w-1,y)
    return m
def office():
    m=room(32,24,doorx=14)
    m.stamp(H,2,1,1,1,2,4,solid=True); m.stamp(H,4,1,3,1,2,4,solid=True)   # bookshelves
    m.stamp(H,16,3,28,3,2,2,solid=True)             # plant
    m.stamp(H,8,3,13,3,2,2,solid=True)              # PC desk
    for (x,y) in ((18,8),(6,14),(20,15)):           # meeting desks w/ chairs
        m.stamp(H,8,9,x,y,4,4,solid=True)
        m.stamp(H,6,10,x-2,y+1,2,2,solid=True); m.stamp(H,12,10,x+4,y+1,2,2,solid=True)
    return m.save('unt_office.png','scn_unt_office',[(14,9),(9,8),(14,22),(15,22),(16,22),(17,22)])
def hall():
    m=room(32,18,ends=(8,11))
    for x in range(1,31): m.stamp(H,0,0,x,13,solid=True)
    for y in range(13,18): m.fill(H,0,0,0,y,32,y+1,solid=True)
    for x in (6,20): m.stamp(H,2,1,x,1,2,4,solid=True)  # shelves along wall
    return m.save('unt_hall.png','scn_unt_hall',[(15,7),(0,8),(0,9),(0,10),(31,8),(31,9),(31,10)])
def house():
    m=room(32,24,doorx=14)
    m.stamp(H,2,1,1,1,2,4,solid=True); m.stamp(H,4,1,3,1,2,4,solid=True)
    m.stamp(H,8,3,8,3,2,2,solid=True)               # TV
    m.stamp(H,16,3,28,3,2,2,solid=True)             # plant
    m.stamp(H,2,1,22,1,2,4,solid=True)
    m.stamp(H,8,9,14,9,4,4,solid=True)              # dining table
    m.stamp(H,6,10,12,10,2,2,solid=True); m.stamp(H,12,10,18,10,2,2,solid=True)
    return m.save('unt_house.png','scn_unt_house',[(5,7),(26,7),(16,14),(14,22),(17,22)])

# ---------- stone rooms: 240401-dungeon2 ----------
D='240401-dungeon2.png'
def stone(w,h,leftcut=False,door=None):
    m=Map(w,h)
    m.fill(D,12,13,0,0,w,h,2,1)                      # stipple floor
    for x in range(0,w,2): m.stamp(D,8,8,x,0,2,2,solid=True); m.stamp(D,8,8,x,h-2,2,2,solid=True)
    for x in range(2,w-2,2): m.stamp(D,10,10,x,2,2,1,solid=True)   # brick face under north wall
    for y in range(2,h-2,2):
        if leftcut: m.stamp(D,9,8,0,y,1,2,solid=True)
        elif not (door and door[0]<=y<door[1]): m.stamp(D,8,8,0,y,2,2,solid=True)
        m.stamp(D,8,8,w-2,y,2,2,solid=True)
    return m
def nave():
    m=stone(32,32,door=(14,18))
    m.stamp(D,10,10,0,13,2,1,solid=True)
    m.fill(D,15,17,15,9,17,30,1,1)                  # aisle (altar inlay tile)
    m.stamp(D,14,16,13,3,6,6,solid=False)           # altar dais
    for y in range(11,28,4):
        for x in (6,10,21,25): m.stamp(D,12,14,x,y,1,2,solid=True)   # pillar rows (pews)
    m.stamp(D,12,14,11,4,1,2,solid=True); m.stamp(D,12,14,20,4,1,2,solid=True)
    return m.save('unt_nave.png','scn_unt_nave',[(21,7),(15,8),(0,15),(0,16),(0,17)])
def cellar():
    m=stone(32,24,leftcut=True)
    m.stamp(D,14,16,13,11,6,6,solid=False)          # rite circle
    for (x,y) in ((3,4),(22,4),(3,17),(27,17),(22,17)): m.stamp(D,12,14,x,y,1,2,solid=True)
    return m.save('unt_cellar.png','scn_unt_cellar',[(8,6),(4,10),(10,8),(1,5),(12,8),(6,10)])

def title():
    m=Map(20,18); m.im=Image.new('RGB',(160,144),C[3])
    for x in range(0,20,2): m.stamp(D,8,8,x,16,2,2,solid=None)
    m.fill(D,12,13,0,14,20,16,2,1,solid=None)
    t=Image.new('1',(80,16),0); d=ImageDraw.Draw(t); d.text((12,2),"UNTITLED",fill=1)
    big=t.resize((160,32),0); m.im.paste(Image.new('RGB',(160,32),C[0]),(0,32),big)
    s=Image.new('1',(160,10),0); ImageDraw.Draw(s).text((44,0),"PRESS START",fill=1)
    m.im.paste(Image.new('RGB',(160,10),C[1]),(0,88),s)
    return m.save('unt_title.png')
def white():
    m=Map(20,18); return m.save('unt_white.png')
if __name__=='__main__':
    for f in (curb,office,hall,house,nave,cellar,title,white): f()
