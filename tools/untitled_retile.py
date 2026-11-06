"""UNTITLED retiler: rebuilds unt_*.png from REAL 8x8 tiles sampled from repo BGs + gbassets tilesets.
Grids replay tools/untitled_gen.py exactly, so collisions in scene.gbsres stay valid. Source of truth for UNT art."""
import os,re,sys,hashlib,numpy as np
from PIL import Image,ImageDraw
R='/workspace/gbm-alltime'; BGD=R+'/assets/backgrounds'; TS=os.environ.get('GBASSETS','/workspace/gbassets-tilesets')
C=[(224,248,208),(136,192,112),(52,104,86),(8,24,32)]
# ---- gen code (tile reference + grids) pulled verbatim from untitled_gen.py ----
src=open(R+'/tools/untitled_gen.py').read()
g_tile=src[src.index('def tile(code):'):src.index('SOLID=set')]
g_grid=src[src.index('def grid(w,h,fill)'):src.index('BGID={}')]
ns={'Image':Image,'ImageDraw':ImageDraw,'C':C}; exec(g_tile+g_grid,ns)
def lv(im):  # quantize to shade index 0..3 by luminance nearest
    a=np.asarray(im.convert('RGBA'),dtype=float)
    l=0.299*a[...,0]+0.587*a[...,1]+0.114*a[...,2]
    l=np.where(a[...,3]<128,255,l)
    ref=np.array([0.299*c[0]+0.587*c[1]+0.114*c[2] for c in C])
    q=np.abs(l[...,None]-ref).argmin(-1)
    # stretch: if image uses fewer distinct luminance bands, rank-map its own 4 levels
    u=np.unique(l)
    if len(u)<=4 and len(u)>1:
        ranks={v:i for i,v in enumerate(sorted(u,reverse=True))}
        m=[0,1,2,3] if len(u)==4 else ([0,3] if len(u)==2 else [0,1,3])
        q=np.vectorize(lambda v:m[ranks[v]])(l)
    return q.astype(np.uint8)
def tiles_of(path,crop=None):
    im=Image.open(path)
    if crop: im=im.crop(crop)
    q=lv(im); h,w=q.shape; out=[]
    for y in range(0,h-7,8):
        for x in range(0,w-7,8): out.append(q[y:y+8,x:x+8])
    return out
B=BGD+'/'; T=TS+'/'
OUT=[B+f for f in ['lavender.png','sample_town.png','city.png','Hometown.png','outside.png','230927-kanto-cerulean.png','230927-Kanto-vermillion.png','route 1.png','230121 dj city.png']]+[T+'thomas.png',T+'thomas2.png',T+'map.png',T+'tilemap.png']
INT=[B+f for f in ['house.png','house-wide.png','230927-kanto-house1f.png','230927-kanto-house2f.png','230927-kanto-npchouse.png','230927-kanto-billshouse.png','230927-kanto-dighouse.png','230927-kanto-bikeshop.png','230927-kanto-mart.png','230927-kanto-club.png','230927-kanto-silph1f.png','230927-kanto-silph2f.png','control room.png','floor room.png','room.png','jadewiiroom.png']]+[T+'tiles.png',T+'tiles (22).png']
TMP=[B+f for f in ['inannastemple.png','inannatemple2.png','skytemple.png','TEMPLE19.png','250113-GB-INANNASTEMPLEFLAME.png']]+[T+'isis01.png',T+'isis02.png',T+'isis03.png',T+'FinalFantasyLegendII-FirstWorld-ShrineOfIsis.png',T+'pyramid.png']
DUN=[B+f for f in ['basement.png','cave.png','cave tileset.png','hidden_cave.png','240401-DUNGEON.png','240401-dungeon2.png','invoke_dungeon_01.png']]+[T+'dungeon rpg.png',T+'underground.png',T+'dungeon rpg water pc98.png']
def pool(paths):
    P=[];used=[]
    for p in paths:
        if not os.path.exists(p): continue
        try: ts=tiles_of(p)
        except Exception as e: print('skip',p,e); continue
        P+=ts; used.append(os.path.basename(p))
    return P,used
def bank(code,paths,n=4):
    ref=np.array(lv(ns['tile'](code)))
    P,_=pool(paths); cnt={}; rep={}
    for t in P:
        k=t.tobytes(); cnt[k]=cnt.get(k,0)+1; rep[k]=t
    sc=[]
    for k,t in rep.items():
        if cnt[k]<3: continue          # only tiles reused in real maps (terrain, not text/one-offs)
        d=np.abs(t.astype(int)-ref).sum()+abs(int(t.mean()*16)-int(ref.mean()*16))*2-6*np.log2(cnt[k])
        sc.append((d,k,t))
    sc.sort(key=lambda s:s[0]); out=[]
    for d,k,t in sc:
        if all(np.abs(t.astype(int)-o).sum()>=6 for o in out): out.append(t)
        if len(out)>=n: break
    return out
def build(name,g,roles,extra=None):
    bk={}; 
    for c in set(ch for row in g for ch in row): bk[c]=bank(c,roles.get(c,roles['*']),1 if c in 'K~' else 2 if c in 'Tr' else 4)
    h=len(g); w=len(g[0]); a=np.zeros((h*8,w*8),np.uint8)
    for y,row in enumerate(g):
        for x,c in enumerate(row):
            v=bk[c]; hsh=int(hashlib.md5(f'{name}{x},{y}'.encode()).hexdigest(),16)%100
            i=0 if hsh<70 or len(v)==1 else 1+hsh%(len(v)-1)
            a[y*8:y*8+8,x*8:x*8+8]=v[i]
    im=Image.fromarray(np.array(C,np.uint8)[a])
    if extra: extra(im)
    q=lv(im); uniq={q[y:y+8,x:x+8].tobytes() for y in range(0,h*8,8) for x in range(0,w*8,8)}
    old=Image.open(f'{BGD}/unt_{name}.png').size; assert old==im.size,(old,im.size)
    im.save(f'{BGD}/unt_{name}.png'); print(f'{name}: {w}x{h} unique_tiles={len(uniq)}'); return len(uniq)
def logo(text,sub):
    def f(im):
        d=ImageDraw.Draw(im); d.rectangle([8,32,151,68],fill=C[3]); big=Image.new('RGB',(64,10),C[3]); ImageDraw.Draw(big).text((4,0),text,fill=C[0])
        im.paste(big.resize((128,20),Image.NEAREST),(16,40)); d.rectangle([36,92,124,106],fill=C[3]); d.text((44,94),sub,fill=C[0])
    return f
G=ns; grid,box,room=G['grid'],G['box'],G['room']
# replay grids (copied from untitled_gen.py)
g=grid(20,18,'K'); box(g,0,15,19,17,'x'); build('title',g,{'*':DUN},logo('UNTITLED','PRESS START'))
g=grid(40,36,'g')
for x in range(40): g[0][x]=g[35][x]='t'
for y in range(36): g[y][0]=g[y][39]='t'
for bx in (2,12,22,31):
    box(g,bx,2,bx+6,3,'T'); box(g,bx,4,bx+6,11,'#')
    for wy in (5,8):
        for wx in (bx+1,bx+3,bx+5): g[wy][wx]='W'
    g[11][bx+3]='D'
box(g,1,12,38,15,'s'); box(g,1,16,38,21,'r')
for x in range(2,38,3): g[18][x]='-'; g[19][x]='-'
box(g,1,22,38,24,'s')
for y,x in [(26,4),(27,9),(29,15),(31,6),(28,24),(31,30),(27,34),(32,20)]: g[y][x]='t'
for x in range(3,37,6): g[25][x]='b'
g[13][35]='m'; build('curb',g,{'*':OUT})
g=room(32,24,'w')
for y in (6,10,14):
    for x0 in (4,12,20): box(g,x0,y,x0+4,y,'d')
for x in (3,8,24,28): g[2][x]='W'
g[2][15]=g[2][16]='m'; box(g,14,23,17,23,'w'); build('office',g,{'*':INT})
g=grid(32,18,'T'); box(g,1,4,30,5,'#'); box(g,1,6,30,12,'k'); box(g,1,13,30,13,'#')
for x in range(4,30,5): g[4][x]='W'
for y in (8,9,10): g[y][0]='k'; g[y][31]='k'
build('hall',g,{'*':INT})
g=room(32,24,'.'); box(g,8,8,23,16,'c'); box(g,13,11,18,12,'d'); box(g,3,4,8,5,'d'); box(g,24,4,28,5,'d')
for x in (11,20): g[2][x]='W'
box(g,14,23,17,23,'.'); build('house',g,{'*':INT})
g=room(32,32,'k'); box(g,13,4,18,30,'c'); box(g,11,5,20,6,'a')
for y in range(10,28,3): box(g,3,y,11,y,'p'); box(g,20,y,28,y,'p')
for x in (4,8,23,27): g[2][x]='W'
for y in (15,16,17): g[y][0]='k'
build('nave',g,{'*':TMP})
g=room(32,32 if False else 24,'x')
for y in (6,7,8): g[y][28]='m'
box(g,22,4,26,5,'d'); build('cellar',g,{'*':DUN})
build('white',grid(20,18,'~'),{'*':INT+TMP})
